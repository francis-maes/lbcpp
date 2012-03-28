/*-----------------------------------------.---------------------------------.
| Filename: ProteinOptimizationWorkUnit.h  | Protein Optimization WorkUnit   |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 13, 2011  1:20:34 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_

# include "DistributableWorkUnit.h"

# include "../Data/Features/PoseFeatureGenerator.h"
# include "../Data/Pose.h"
# include "../Data/Rosetta.h"
# include "../ProteinOptimizer/SimulatedAnnealing.h"
# include "../Data/Mover/PoseMover.h"
# include "../Data/MoverSampler/BlindPoseMoverSampler.h"
# include "../Data/MoverSampler/ConditionalPoseMoverSampler.h"
# include "../ProteinOptimizer/ProteinOptimizer.h"

namespace lbcpp
{

class SingleProteinOptimizationWorkUnit : public WorkUnit
{
public:
  SingleProteinOptimizationWorkUnit() {}
  SingleProteinOptimizationWorkUnit(size_t id,
                                    ProteinPtr inputProtein,
                                    String referencesDirectory,
                                    String moversDirectory,
                                    GeneralOptimizerParametersPtr parameters,
                                    size_t repeat)
    : id(id),
      inputProtein(inputProtein),
      referencesDirectory(referencesDirectory),
      moversDirectory(moversDirectory),
      parameters(parameters),
      repeat(repeat)
  {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Treating protein : ") + inputProtein->getName());
    Rosetta ros;
    ros.init(context, false, id, 100);

    PosePtr pose = new Pose(inputProtein);

    // features
    PoseFeatureGeneratorPtr features = new PoseFeatureGenerator();
    features->initialize(context, poseClass);
    DoubleVectorPtr initializeFeatures = features->compute(context, pose).getObjectAndCast<DoubleVector> ();

    // learn the distribution
    File referencesFile = context.getFile(referencesDirectory);
    File moversFile = context.getFile(moversDirectory);
    VectorPtr inputWorkers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    VectorPtr inputMovers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    SamplerPtr sampler;

    if (!referencesDirectory.isEmpty() && !moversDirectory.isEmpty() && referencesFile.exists() && moversFile.exists())
    {
      context.enterScope(T("Loading learning examples..."));
      juce::OwnedArray<File> references;
      referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));

      for (size_t i = 0; i < (size_t)references.size(); i++)
      {
        juce::OwnedArray<File> movers;
        String nameToSearch = (*references[i]).getFileNameWithoutExtension();

        PosePtr protein = new Pose(*references[i]);

        nameToSearch += T("_mover.xml");
        moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
        if (movers.size() > 0)
        {
          context.informationCallback(T("Structure : ") + nameToSearch);
          DoubleVectorPtr inFeatures = features->compute(context, protein).getObjectAndCast<DoubleVector> ();
          PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<PoseMover> ();
          inputWorkers->append(inFeatures);
          inputMovers->append(inMover);
          context.progressCallback(new ProgressionState((size_t)(i + 1), (size_t)juce::jmin((int)10000, (int)references.size()), T("Intermediate conformations")));
        }
      }
      context.leaveScope();

      ContainerPtr addWorkers = inputWorkers;
      ContainerPtr addMovers = inputMovers;

      sampler = new ConditionalPoseMoverSampler(pose->getLength());
      sampler->learn(context, addWorkers, addMovers, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());
    }
    else
    {
      if (referencesDirectory.isEmpty())
        context.informationCallback(T("Input empty : referencesDirectory"));
      if (moversDirectory.isEmpty())
        context.informationCallback(T("Input empty : moversDirectory"));
      if (!referencesFile.exists())
        context.informationCallback(T("Input file not found : referencesFile"));
      if (!moversFile.exists())
        context.informationCallback(T("Input file not found : moversFile"));

      context.informationCallback(T("Using BlindPoseMoverSampler"));
      sampler = new BlindPoseMoverSampler(pose->getLength());
    }

    std::vector<ScalarVariableMeanAndVariancePtr> meanEnergies;
    std::vector<ScalarVariableMeanAndVariancePtr> meanAccepted;
    std::vector<ScalarVariableMeanAndVariancePtr> meanDecreasing;
    size_t numElements = 0;

    // repeat optimization to average results
    repeat = (int)juce::jmax((int)1, (int)repeat);
    for (size_t i = 0; i < repeat; ++i)
    {
      // initialize data
      pose->initializeToHelix();
      OptimizationProblemStatePtr optState = new PoseOptimizationState(pose);

      OptimizationProblemStateModifierPtr modifier = new PoseOptimizationStateModifier(sampler, features);
      SimulatedAnnealingPtr sa = new SimulatedAnnealing(optState, modifier, GeneralOptimizerStoppingCriterionPtr(), parameters);

      // optimize
      Variable optimizationResult = sa->optimize(context);

      // results
      DenseDoubleVectorPtr costEvolution = optimizationResult.getObjectAndCast<VariableVector> ()->getElement(0).getObjectAndCast<DenseDoubleVector> ();
      DenseDoubleVectorPtr acceptedModificationsEvolution = optimizationResult.getObjectAndCast<VariableVector> ()->getElement(1).getObjectAndCast<DenseDoubleVector> ();
      DenseDoubleVectorPtr decreasingModificationsEvolution = optimizationResult.getObjectAndCast<VariableVector> ()->getElement(2).getObjectAndCast<DenseDoubleVector> ();
      numElements = costEvolution->getNumElements();

      // first result obtained
      if (i == 0)
      {
        meanEnergies = std::vector<ScalarVariableMeanAndVariancePtr>(numElements);
        meanAccepted = std::vector<ScalarVariableMeanAndVariancePtr>(numElements);
        meanDecreasing = std::vector<ScalarVariableMeanAndVariancePtr>(numElements);

        for (size_t j = 0; j < numElements; ++j)
        {
          meanEnergies[j] = new ScalarVariableMeanAndVariance();
          meanAccepted[j] = new ScalarVariableMeanAndVariance();
          meanDecreasing[j] = new ScalarVariableMeanAndVariance();
        }
      }

      for (size_t j = 0; j < numElements; j++)
      {
        meanEnergies[j]->push(costEvolution->getValue(j));
        meanAccepted[j]->push(acceptedModificationsEvolution->getValue(j));
        meanDecreasing[j]->push(decreasingModificationsEvolution->getValue(j));
      }

    }

    context.leaveScope();

    // get averages and return those averages
    DenseDoubleVectorPtr energiesToReturn = new DenseDoubleVector(numElements, -1);
    DenseDoubleVectorPtr acceptedToReturn = new DenseDoubleVector(numElements, -1);
    DenseDoubleVectorPtr decreasingToReturn = new DenseDoubleVector(numElements, -1);

    if (repeat > 1)
      context.enterScope(T("Mean values"));
    for (size_t i = 0; i < numElements; i++)
    {
      double meanEnergyTmp = meanEnergies[i]->getMean();
      double stdEnergyTmp = meanEnergies[i]->getStandardDeviation();
      energiesToReturn->setValue(i, meanEnergyTmp);

      double meanAcceptedTmp = meanAccepted[i]->getMean();
      double stdAcceptedTmp = meanAccepted[i]->getStandardDeviation();
      acceptedToReturn->setValue(i, meanAcceptedTmp);

      double meanDecreasingTmp = meanDecreasing[i]->getMean();
      double stdDecreasingTmp = meanDecreasing[i]->getStandardDeviation();
      decreasingToReturn->setValue(i, meanDecreasingTmp);

      if (repeat > 1)
      {
        context.enterScope(T("Result"));
        context.resultCallback(T("Iteration"), Variable(i));
        context.resultCallback(T("Mean energy"), Variable(meanEnergyTmp));
        context.resultCallback(T("Std Dev energy"), Variable(stdEnergyTmp));

        context.resultCallback(T("Mean accepted"), Variable(meanAcceptedTmp));
        context.resultCallback(T("Std Dev accepted"), Variable(stdAcceptedTmp));

        context.resultCallback(T("Mean decreasing"), Variable(meanDecreasingTmp));
        context.resultCallback(T("Std Dev decreasing"), Variable(stdDecreasingTmp));
        context.leaveScope(Variable(meanEnergyTmp));
      }
    }
    if (repeat > 1)
      context.leaveScope();

    VariableVectorPtr returnVector = variableVector(3);
    returnVector->setElement(0, energiesToReturn);
    returnVector->setElement(1, acceptedToReturn);
    returnVector->setElement(2, decreasingToReturn);

    return returnVector;
  }

protected:
  friend class SingleProteinOptimizationWorkUnitClass;

  size_t id;
  ProteinPtr inputProtein;
  String referencesDirectory;
  String moversDirectory;
  GeneralOptimizerParametersPtr parameters;
  size_t repeat;
};

class ProteinOptimizationWorkUnit : public DistributableWorkUnit
{
public:
  ProteinOptimizationWorkUnit() {}
  ProteinOptimizationWorkUnit(String inputDirectory,
                              String referencesDirectory,
                              String moversDirectory,
                              size_t numIterations,
                              double initialTemperature,
                              double finalTemperature,
                              size_t numDecreasingSteps,
                              size_t repeat)
    : inputDirectory(inputDirectory),
      referencesDirectory(referencesDirectory),
      moversDirectory(moversDirectory),
      numIterations(numIterations),
      initialTemperature(initialTemperature),
      finalTemperature(finalTemperature),
      numDecreasingSteps(numDecreasingSteps),
      repeat(repeat)
    {}

  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
  {
    context.resultCallback(T("numIterations"), Variable((int)numIterations));
    context.resultCallback(T("initialTemperature"), Variable(initialTemperature));
    context.resultCallback(T("finalTemperature"), Variable(finalTemperature));
    context.resultCallback(T("numDecreasingSteps"), Variable((int)numDecreasingSteps));
    context.resultCallback(T("repeat"), Variable((int)repeat));

    DenseDoubleVectorPtr costEvolution = result.getObjectAndCast<VariableVector> ()->getElement(0).getObjectAndCast<DenseDoubleVector> ();
    DenseDoubleVectorPtr acceptedModificationsEvolution = result.getObjectAndCast<VariableVector> ()->getElement(1).getObjectAndCast<DenseDoubleVector> ();
    DenseDoubleVectorPtr decreasingModificationsEvolution = result.getObjectAndCast<VariableVector> ()->getElement(2).getObjectAndCast<DenseDoubleVector> ();
    size_t numElements = costEvolution->getNumElements();

    context.enterScope(T("Results"));
    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Value"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Energy"), costEvolution->getValue(i));
      context.resultCallback(T("Accepted decreasing modifications"), acceptedModificationsEvolution->getValue(i));
      context.resultCallback(T("Energy decreasing modifications"), decreasingModificationsEvolution->getValue(i));
      context.leaveScope();
    }
    context.leaveScope();

    return result;
  }

  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
  {
    context.resultCallback(T("numIterations"), Variable((int)numIterations));
    context.resultCallback(T("initialTemperature"), Variable(initialTemperature));
    context.resultCallback(T("finalTemperature"), Variable(finalTemperature));
    context.resultCallback(T("numDecreasingSteps"), Variable((int)numDecreasingSteps));
    context.resultCallback(T("repeat"), Variable((int)repeat));

    DenseDoubleVectorPtr energies;
    DenseDoubleVectorPtr acceptedModificationsEvolution;
    DenseDoubleVectorPtr decreasingModificationsEvolution;

    // export results
    if (results.getNumElements() > 0)
    {
      size_t numElements = results.getElement(0).getObjectAndCast<VariableVector> ()->getElement(0).getObjectAndCast<DenseDoubleVector> ()->getNumElements();
      std::vector<ScalarVariableMeanAndVariancePtr> meanEnergies(numElements);
      std::vector<ScalarVariableMeanAndVariancePtr> meanAccepted(numElements);
      std::vector<ScalarVariableMeanAndVariancePtr> meanDecreasing(numElements);

      for (size_t i = 0; i < numElements; i++)
      {
        meanEnergies[i] = new ScalarVariableMeanAndVariance();
        meanAccepted[i] = new ScalarVariableMeanAndVariance();
        meanDecreasing[i] = new ScalarVariableMeanAndVariance();
      }

      energies = new DenseDoubleVector(numElements, -1);
      acceptedModificationsEvolution = new DenseDoubleVector(numElements, -1);
      decreasingModificationsEvolution = new DenseDoubleVector(numElements, -1);

      for (size_t i = 0; i < results.getNumElements(); i++)
      {
        VariableVectorPtr tmpResult = results.getElement(i).getObjectAndCast<VariableVector> ();

        for (size_t j = 0; j < numElements; j++)
        {
          meanEnergies[j]->push(tmpResult->getElement(0).getObjectAndCast<DenseDoubleVector> ()->getValue(j));
          meanAccepted[j]->push(tmpResult->getElement(1).getObjectAndCast<DenseDoubleVector> ()->getValue(j));
          meanDecreasing[j]->push(tmpResult->getElement(2).getObjectAndCast<DenseDoubleVector> ()->getValue(j));
        }
      }

      context.enterScope(T("Results"));
      for (size_t i = 0; i < numElements; i++)
      {
        double meanEnergyTmp = meanEnergies[i]->getMean();
        double stdEnergyTmp = meanEnergies[i]->getStandardDeviation();
        energies->setValue(i, meanEnergyTmp);

        double meanAcceptedTmp = meanAccepted[i]->getMean();
        double stdAcceptedTmp = meanAccepted[i]->getStandardDeviation();
        acceptedModificationsEvolution->setValue(i, meanAcceptedTmp);

        double meanDecreasingTmp = meanDecreasing[i]->getMean();
        double stdDecreasingTmp = meanDecreasing[i]->getStandardDeviation();
        decreasingModificationsEvolution->setValue(i, meanDecreasingTmp);

        context.enterScope(T("Result"));
        context.resultCallback(T("Iteration"), Variable(i));
        context.resultCallback(T("Mean energy"), Variable(meanEnergyTmp));
        context.resultCallback(T("Std Dev energy"), Variable(stdEnergyTmp));

        context.resultCallback(T("Mean accepted"), Variable(meanAcceptedTmp));
        context.resultCallback(T("Std Dev accepted"), Variable(stdAcceptedTmp));

        context.resultCallback(T("Mean decreasing"), Variable(meanDecreasingTmp));
        context.resultCallback(T("Std Dev decreasing"), Variable(stdDecreasingTmp));
        context.leaveScope(Variable(meanEnergyTmp));
      }
      context.leaveScope();
    }

    VariableVectorPtr returnVector = variableVector(3);
    returnVector->setElement(0, energies);
    returnVector->setElement(1, acceptedModificationsEvolution);
    returnVector->setElement(2, decreasingModificationsEvolution);

    return returnVector;
  }

  virtual void initializeWorkUnits(ExecutionContext& context)
  {
    // check files
    File inputFile = context.getFile(inputDirectory);

    if (!inputFile.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return;
    }

    // loading proteins and workunits
    juce::OwnedArray<File> inputProteins;
    inputFile.findChildFiles(inputProteins, File::findFiles, false, T("*.pdb"));

    context.enterScope(T("Creating work units..."));
    workUnits = new CompositeWorkUnit(T("Optimization"));

    for (size_t i = 0; i < inputProteins.size(); ++i)
    {
      ProteinPtr currentProtein = Protein::createFromPDB(context, (*inputProteins[i]));
      GeneralOptimizerParametersPtr parameters = new SimulatedAnnealingParameters(numIterations, initialTemperature, finalTemperature, numDecreasingSteps);

      workUnits->addWorkUnit(new SingleProteinOptimizationWorkUnit(i, currentProtein, referencesDirectory, moversDirectory, parameters, repeat));

      context.progressCallback(new ProgressionState((size_t)(i + 1), inputProteins.size(), T("Proteins")));
    }
    context.leaveScope();
  }

protected:
  friend class ProteinOptimizationWorkUnitClass;

  String inputDirectory;
  String referencesDirectory;
  String moversDirectory;

  size_t numIterations;
  double initialTemperature;
  double finalTemperature;
  size_t numDecreasingSteps;

  size_t repeat;
};

extern DistributableWorkUnitPtr proteinOptimizationWorkUnit(String inputDirectory,
                                                            String referencesDirectory,
                                                            String moversDirectory,
                                                            size_t numIterations,
                                                            double initialTemperature,
                                                            double finalTemperature,
                                                            size_t numDecreasingSteps,
                                                            size_t repeat);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_

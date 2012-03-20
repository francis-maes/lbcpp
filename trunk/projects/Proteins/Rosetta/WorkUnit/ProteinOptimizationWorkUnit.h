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
                                    GeneralOptimizerParametersPtr parameters)
    : id(id),
      inputProtein(inputProtein),
      referencesDirectory(referencesDirectory),
      moversDirectory(moversDirectory),
      parameters(parameters)
  {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Treating protein : ") + inputProtein->getName());
    Rosetta ros;
    ros.init(context, false, 0, id * 5);

    PosePtr pose = new Pose(inputProtein);
    pose->initializeToHelix();

    // features
    PoseFeatureGeneratorPtr features = new PoseFeatureGenerator();
    features->initialize(context, poseClass);
    DoubleVectorPtr initializeFeatures = features->compute(context, pose).getObjectAndCast<DoubleVector>();

    // learn the distribution
    File referencesFile = context.getFile(referencesDirectory);
    File moversFile = context.getFile(moversDirectory);
    VectorPtr inputWorkers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    VectorPtr inputMovers = vector(doubleVectorClass(initializeFeatures->getElementsEnumeration(), doubleType));
    SamplerPtr sampler;

    if (!referencesFile.exists() || !moversFile.exists())
    {
      context.enterScope(T("Loading learning examples..."));
      juce::OwnedArray<File> references;
      referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));

      for (size_t i = 0; i < references.size(); i++)
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
      context.informationCallback(T("Using BlindPoseMoverSampler"));
      sampler = new BlindPoseMoverSampler(pose->getLength());
    }

    OptimizationProblemStatePtr optState = new PoseOptimizationState(pose);

    OptimizationProblemStateModifierPtr modifier = new PoseOptimizationStateModifier(sampler, features);
    SimulatedAnnealingPtr sa = new SimulatedAnnealing(optState, modifier, GeneralOptimizerStoppingCriterionPtr(), parameters);

    Variable optimizationResult = sa->optimize(context);

    context.leaveScope();

    return optimizationResult;
  }

protected:
  friend class SingleProteinOptimizationWorkUnitClass;

  size_t id;
  ProteinPtr inputProtein;
  String referencesDirectory;
  String moversDirectory;
  GeneralOptimizerParametersPtr parameters;
};

class ProteinOptimizationWorkUnit : public DistributableWorkUnit
{
public:
  ProteinOptimizationWorkUnit() {}
  ProteinOptimizationWorkUnit(String inputDirectory,
                              String referencesDirectory,
                              String moversDirectory)
    : inputDirectory(inputDirectory),
      referencesDirectory(referencesDirectory),
      moversDirectory(moversDirectory)
    {}

  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
  {
    DenseDoubleVectorPtr costEvolution = result.getObjectAndCast<VariableVector> ()->getElement(0).getObjectAndCast<DenseDoubleVector> ();
    DenseDoubleVectorPtr acceptedModificationsEvolution = result.getObjectAndCast<VariableVector> ()->getElement(1).getObjectAndCast<DenseDoubleVector> ();
    DenseDoubleVectorPtr decreasingModificationsEvolution = result.getObjectAndCast<VariableVector> ()->getElement(2).getObjectAndCast<DenseDoubleVector> ();
    size_t numElements = costEvolution->getNumElements();

    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Value"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Energy"), costEvolution->getValue(i));
      context.resultCallback(T("Accepted decreasing modifications"), acceptedModificationsEvolution->getValue(i));
      context.resultCallback(T("Energy decreasing modifications"), decreasingModificationsEvolution->getValue(i));
      context.leaveScope();
    }

    return result;
  }

  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
  {
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

    return Variable(energies);
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

    for (int i = 0; i < inputProteins.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromPDB(context, (*inputProteins[i]));
      GeneralOptimizerParametersPtr parameters = new SimulatedAnnealingParameters(1000000, 4, 0.01, 50);

      workUnits->addWorkUnit(new SingleProteinOptimizationWorkUnit(i, currentProtein, referencesDirectory, moversDirectory, parameters));

      context.progressCallback(new ProgressionState((size_t)(i + 1), inputProteins.size(), T("Proteins")));
    }
    context.leaveScope();
  }

protected:
  friend class ProteinOptimizationWorkUnitClass;

  String inputDirectory;
  String referencesDirectory;
  String moversDirectory;
};

extern DistributableWorkUnitPtr proteinOptimizationWorkUnit(String inputDirectory,
                                                            String referencesDirectory,
                                                            String moversDirectory);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_

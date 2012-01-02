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
# include "../Data/Rosetta.h"
# include "../ProteinOptimizer/SimulatedAnnealingOptimizer.h"

namespace lbcpp
{

class SingleProteinOptimizationWorkUnit : public WorkUnit
{
public:
  SingleProteinOptimizationWorkUnit() {}
  SingleProteinOptimizationWorkUnit(
      Protein inputProtein,
      String outputDirectory,
      int numOutputFiles,
      String referencesDirectory,
      String moversDirectory,
      size_t maxLearningSamples,
      double initialTemperature,
      double finalTemperature,
      size_t numDecreasingSteps,
      size_t numIterations)
    : inputProtein(new Protein(inputProtein)),
      outputDirectory(outputDirectory),
      numOutputFiles(numOutputFiles),
      referencesDirectory(referencesDirectory),
      moversDirectory(moversDirectory),
      maxLearningSamples(maxLearningSamples),
      initialTemperature(initialTemperature),
      finalTemperature(finalTemperature),
      numDecreasingSteps(numDecreasingSteps),
      numIterations(numIterations)
  {}

  virtual Variable run(ExecutionContext& context)
  {
    Rosetta ros;
    ros.init(context);

    juce::File outputFile = context.getFile(outputDirectory);
    if (!outputFile.exists())
      outputFile.createDirectory();

    File referencesFile = context.getFile(referencesDirectory);
    File moversFile = context.getFile(moversDirectory);

# ifdef LBCPP_PROTEIN_ROSETTA

    VariableVectorPtr inputWorkers = new VariableVector(0);
    VariableVectorPtr inputMovers = new VariableVector(0);
    if (referencesFile.exists() && moversFile.exists())
    {
      context.enterScope(T("Loading learning examples..."));
      juce::OwnedArray<File> references;
      referencesFile.findChildFiles(references, File::findFiles, false, T("*.pdb"));

      std::vector<size_t> res;
      RandomGeneratorPtr rand = new RandomGenerator();
      rand->sampleOrder(references.size(), res);

      for (size_t i = 0; (i < references.size()) && (i < maxLearningSamples); i++)
      {
        size_t index = res[i];
        juce::OwnedArray<File> movers;
        String nameToSearch = (*references[index]).getFileNameWithoutExtension();

        ProteinPtr protein = Protein::createFromPDB(context, (*references[index]));
        core::pose::PoseOP pose;
        convertProteinToPose(context, protein, pose);
        if (pose() == NULL)
          continue;

        nameToSearch += T("_mover.xml");
        moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
        if (movers.size() > 0)
        {
          context.informationCallback(T("Name structure : ") + nameToSearch);
          RosettaProteinPtr inWorker = new RosettaProtein(pose, 1, 1, 1, 1);
          PoseMoverPtr inMover = Variable::createFromFile(context, (*movers[0])).getObjectAndCast<
              PoseMover> ();
          inputWorkers->append(inWorker);
          inputMovers->append(inMover);
          context.progressCallback(new ProgressionState((size_t)(i + 1), (size_t)juce::jmin(
              (int)maxLearningSamples, (int)references.size()), T("Intermediate conformations")));
        }
      }
      context.leaveScope();
    }
    else if (referencesFile.exists() || moversFile.exists())
    {
      if (!referencesFile.exists())
        context.errorCallback(T("References' directory not found."));
      if (!moversFile.exists())
        context.errorCallback(T("Movers' directory not found."));
    }

    core::pose::PoseOP currentPose;
    convertProteinToPose(context, inputProtein, currentPose);

    double frequenceVerbosity = 0.01;

    core::pose::PoseOP initialPose;
    initializeProteinStructure(currentPose, initialPose);
    context.enterScope(T("Optimizing protein : ") + inputProtein->getName());

    RosettaWorkerPtr worker = new RosettaWorker(initialPose);
    ContainerPtr addWorkers = inputWorkers;
    ContainerPtr addMovers = inputMovers;
    // learn
    worker->learn(context, addWorkers, addMovers);

    RandomGeneratorPtr random = new RandomGenerator();
    DenseDoubleVectorPtr energiesAtIteration;
    ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
        initialTemperature, finalTemperature, numDecreasingSteps, numIterations, 5,
        inputProtein->getName(), frequenceVerbosity, numOutputFiles, outputFile);

    optimizer->apply(context, worker, random, energiesAtIteration);

    double finalEnergy = 0;
    worker->energies(&finalEnergy);
    context.leaveScope(finalEnergy);

    return Variable(energiesAtIteration);

# else
    jassert(false);
    return Variable();
# endif
  }

protected:
  friend class SingleProteinOptimizationWorkUnitClass;

  ProteinPtr inputProtein;

  String outputDirectory;
  int numOutputFiles;

  String referencesDirectory;
  String moversDirectory;
  size_t maxLearningSamples;

  double initialTemperature;
  double finalTemperature;
  size_t numDecreasingSteps;
  size_t numIterations;
};

class ProteinOptimizationWorkUnit : public DistributableWorkUnit
{
public:
  ProteinOptimizationWorkUnit()
    : DistributableWorkUnit(String("ProteinOptimizationWorkunit")) {}

  virtual Variable resultsCallback(ExecutionContext& context, VariableVector& results)
  {
    DenseDoubleVectorPtr energies;
    // export results
    if (results.getNumElements() > 0)
    {
      size_t numElements =
          results.getElement(0).getObjectAndCast<DenseDoubleVector> ()->getNumElements();
      std::vector<ScalarVariableMeanAndVariancePtr> values(numElements);
      for (size_t i = 0; i < numElements; i++)
        values[i] = new ScalarVariableMeanAndVariance();

      energies = new DenseDoubleVector(numElements, -1);

      for (size_t i = 0; i < results.getNumElements(); i++)
        for (size_t j = 0; j < numElements; j++)
          values[j]->push(results.getElement(i).getObjectAndCast<DenseDoubleVector> ()->getValue(j));

      context.enterScope(T("Results"));

      for (size_t i = 0; i < numElements; i++)
      {
        double meanEnergy = values[i]->getMean();
        energies->setValue(i, meanEnergy);
        double stdEnergy = values[i]->getStandardDeviation();

        context.enterScope(T("Result"));
        context.resultCallback(T("Iteration"), Variable(i));
        context.resultCallback(T("Mean energy"), Variable(meanEnergy));
        context.resultCallback(T("Std Dev energy"), Variable(stdEnergy));
        context.leaveScope(Variable(meanEnergy));
      }
    }
    context.leaveScope();
    context.informationCallback(T("Done."));

    return Variable(energies);
  }

  virtual void initializeWorkUnits(ExecutionContext& context)
  {
    File inputFile = context.getFile(inputDirectory);
    if (!inputFile.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return;
    }

    juce::OwnedArray<File> inputProteins;
    inputFile.findChildFiles(inputProteins, File::findFiles, false, T("*.pdb"));

    context.enterScope(T("Creating work units..."));

    workUnits = new CompositeWorkUnit(T("Optimization"));

    for (size_t i = 0; i < inputProteins.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromPDB(context, (*inputProteins[i]));

      workUnits->addWorkUnit(new SingleProteinOptimizationWorkUnit(
          *currentProtein,
          outputDirectory,
          numOutputFiles,
          referencesDirectory,
          moversDirectory,
          maxLearningSamples,
          initialTemperature,
          finalTemperature,
          numDecreasingSteps,
          numIterations));

      context.progressCallback(new ProgressionState((size_t)(i + 1), inputProteins.size(),
          T("Proteins")));
    }
    context.leaveScope();
  }

protected:
  friend class ProteinOptimizationWorkUnitClass;

  String inputDirectory;
  String outputDirectory;
  int numOutputFiles;
  String referencesDirectory;
  String moversDirectory;
  size_t maxLearningSamples;
  double initialTemperature;
  double finalTemperature;
  size_t numDecreasingSteps;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_

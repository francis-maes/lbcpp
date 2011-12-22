/*-----------------------------------------.---------------------------------.
| Filename: ProteinOptimizationWorkUnit.h  | Protein Optimization WorkUnit   |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 13, 2011  1:20:34 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "../RosettaUtils.h"
# include "../ProteinOptimizer/SimulatedAnnealingOptimizer.h"

namespace lbcpp
{

class ProteinOptimizationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
# ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(context, false);

    File inputFile = context.getFile(inputDirectory);
    if (!inputFile.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return Variable();
    }

    juce::File outputFile = context.getFile(outputDirectory);
    if (!outputFile.exists())
      outputFile.createDirectory();

    File referencesFile = context.getFile(referencesDirectory);
    if (!referencesFile.exists() && (learningPolicy > 0))
    {
      context.errorCallback(T("References' directory not found."));
      return Variable();
    }

    File moversFile = context.getFile(moversDirectory);
    if (!moversFile.exists() && (learningPolicy > 0))
    {
      context.errorCallback(T("Movers' directory not found."));
      return Variable();
    }

    VariableVectorPtr inputWorkers = new VariableVector(0);
    VariableVectorPtr inputMovers = new VariableVector(0);
    if (learningPolicy)
    {
      context.enterScope(T("Loading learning examples..."));
      juce::OwnedArray<File> references;
      referencesFile.findChildFiles(references, File::findFiles, false, T("*.xml"));
      size_t maxCount = 10000;
      std::vector<size_t> res;
      RandomGeneratorPtr rand = new RandomGenerator();
      rand->sampleOrder(references.size(), res);

      for (size_t i = 0; (i < references.size()) && (i < maxCount); i++)
      {
        size_t index = res[i];
        juce::OwnedArray<File> movers;
        String nameToSearch = (*references[index]).getFileNameWithoutExtension();

        ProteinPtr protein = Protein::createFromFile(context, (*references[index]));
        core::pose::PoseOP pose;
        convertProteinToPose(context, protein, pose);
        if (pose() == NULL)
          continue;

        nameToSearch += T("_mover.xml");
        moversFile.findChildFiles(movers, File::findFiles, false, nameToSearch);
        for (size_t j = 0; (j < movers.size()) && (j < numMoversToLearn); j++)
        {
          context.informationCallback(T("Name structure : ") + nameToSearch);
          RosettaProteinPtr inWorker = new RosettaProtein(pose, residueFeatures, energyFeatures,
              histogramFeatures, distanceFeatures);
          PoseMoverPtr inMover =
              Variable::createFromFile(context, (*movers[j])).getObjectAndCast<PoseMover> ();
          inputWorkers->append(inWorker);
          inputMovers->append(inMover);
          context.progressCallback(new ProgressionState((size_t)(i + 1), (size_t)juce::jmin(
              (int)maxCount, (int)references.size()), T("Intermediate conformations")));
        }
      }
      context.leaveScope();
    }

    core::pose::PoseOP currentPose;

    juce::OwnedArray<File> results;
    inputFile.findChildFiles(results, File::findFiles, false, T("*.xml"));

    double frequenceVerbosity = 0.01;
    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances;

    context.enterScope(T("Optimization"));

    for (size_t i = 0; i < results.size(); i++)
    {
      context.progressCallback(new ProgressionState((size_t)(i + 1), results.size(), T("Proteins")));

      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));
      String currentName = currentProtein->getName();

      convertProteinToPose(context, currentProtein, currentPose);
      if (currentPose() == NULL)
        continue;

      core::pose::PoseOP initialPose;
      initializeProteinStructure(currentPose, initialPose);
      context.enterScope(T("Optimizing protein : ") + currentName);

      RosettaWorkerPtr worker = new RosettaWorker(initialPose, learningPolicy, residueFeatures,
          energyFeatures, histogramFeatures, distanceFeatures);
      ContainerPtr addWorkers = inputWorkers;
      ContainerPtr addMovers = inputMovers;
      // learn
      worker->learn(context, addWorkers, addMovers);

      RandomGeneratorPtr random = new RandomGenerator();
      DenseDoubleVectorPtr energiesAtIteration;
      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
          initialTemperature, finalTemperature, 50, numIterations, 5, currentName,
          frequenceVerbosity, numOutputFiles, outputFile);

      optimizer->apply(context, worker, random, energiesAtIteration);

      for (size_t j = 0; j < energiesAtIteration->getNumValues(); j++)
        if (j >= meansAndVariances.size())
        {
          meansAndVariances.push_back(new ScalarVariableMeanAndVariance());
          meansAndVariances[j]->push(energiesAtIteration->getValue(j));
        }
        else
          meansAndVariances[j]->push(energiesAtIteration->getValue(j));

      context.leaveScope(String("Done."));
    }

    context.leaveScope();

    DenseDoubleVectorPtr energies = new DenseDoubleVector(meansAndVariances.size(), -1);

    // export results
    context.enterScope(T("Results"));
    for (size_t k = 0; k < meansAndVariances.size(); k++)
    {
      context.enterScope(T("Values"));
      context.resultCallback(T("Iteration"), Variable(k));
      double meanEnergy = meansAndVariances[k]->getMean();
      energies->setValue(k, meanEnergy);
      context.resultCallback(T("Mean energy"), Variable(meanEnergy));
      context.resultCallback(T("Std Dev energy"), Variable(meansAndVariances[k]->getStandardDeviation()));
      context.leaveScope(Variable(meanEnergy));
    }
    context.leaveScope();

    context.informationCallback(T("Done."));

    return Variable(energies);

# else
    jassert(false);
    return Variable();
# endif

  }

protected:
  friend class ProteinOptimizationWorkUnitClass;

  String inputDirectory;
  String outputDirectory;
  String referencesDirectory;
  String moversDirectory;
  size_t learningPolicy;
  size_t residueFeatures;
  size_t energyFeatures;
  size_t histogramFeatures;
  size_t distanceFeatures;
  size_t numMoversToLearn;
  double initialTemperature;
  double finalTemperature;
  int numOutputFiles;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_PROTEINOPTIMIZATIONWORKUNIT_H_


/*-----------------------------------------.---------------------------------.
| Filename: ProteinEDA..WorkUnit.h         | ProteinEDAOptimizationWorkUnit  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 19, 2011  2:47:26 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_WORKUNIT_PROTEINEDAOPTIMIZATIONWORKUNIT_H_
# define LBCPP_PROTEIN_ROSETTA_WORKUNIT_PROTEINEDAOPTIMIZATIONWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "../Data/Rosetta.h"
# include "../Data/Pose.h"

namespace lbcpp
{

class ProteinEDAOptimizationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    RosettaPtr rosetta = new Rosetta();
    rosetta->init(context, false);

//    juce::OwnedArray<File> results;
//    inputFile.findChildFiles(results, File::findFiles, false, T("*.xml"));
//
//    double frequenceVerbosity = 0.01;
//    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances;
//
//    for (size_t i = 0; i < results.size(); i++)
//    {
//      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));
//      String currentName = currentProtein->getName();
//
//      convertProteinToPose(context, currentProtein, currentPose);
//      if (currentPose() == NULL)
//        continue;
//
//      core::pose::PoseOP initialPose;
//      initializeProteinStructure(currentPose, initialPose);
//      context.enterScope(T("Optimizing protein : ") + currentName);
//
//      RosettaWorkerPtr worker = new RosettaWorker(initialPose, learningPolicy, residueFeatures,
//          energyFeatures, histogramFeatures, distanceFeatures);
//      ContainerPtr addWorkers = inputWorkers;
//      ContainerPtr addMovers = inputMovers;
//      // learn
//      worker->learn(context, addWorkers, addMovers);
//
//      RandomGeneratorPtr random = new RandomGenerator();
//      DenseDoubleVectorPtr energiesAtIteration;
//      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
//          initialTemperature, finalTemperature, 50, numIterations, 5, currentName,
//          frequenceVerbosity, numOutputFiles, outputFile);
//
//      optimizer->apply(context, worker, random, energiesAtIteration);
//
//      for (size_t j = 0; j < energiesAtIteration->getNumValues(); j++)
//        if (j >= meansAndVariances.size())
//        {
//          meansAndVariances.push_back(new ScalarVariableMeanAndVariance());
//          meansAndVariances[j]->push(energiesAtIteration->getValue(j));
//        }
//        else
//          meansAndVariances[j]->push(energiesAtIteration->getValue(j));
//
//      context.leaveScope(String("Done."));
//    }
//
//    DenseDoubleVectorPtr energies = new DenseDoubleVector(meansAndVariances.size(), -1);
//
//    // export results
//    context.enterScope(T("Results"));
//    for (size_t k = 0; k < meansAndVariances.size(); k++)
//    {
//      context.enterScope(T("Values"));
//      context.resultCallback(T("Iteration"), Variable(k));
//      double meanEnergy = meansAndVariances[k]->getMean();
//      energies->setValue(k, meanEnergy);
//      context.resultCallback(T("Mean energy"), Variable(meanEnergy));
//      context.resultCallback(T("Std Dev energy"), Variable(meansAndVariances[k]->getStandardDeviation()));
//      context.leaveScope(Variable(meanEnergy));
//    }
//    context.leaveScope();
//
//    context.informationCallback(T("Done."));
//
//    return Variable(energies);
    return Variable();
  }

protected:
  friend class ProteinEDAOptimizationWorkUnitClass;

  size_t numSamplesPerIteration;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_WORKUNIT_PROTEINEDAOPTIMIZATIONWORKUNIT_H_

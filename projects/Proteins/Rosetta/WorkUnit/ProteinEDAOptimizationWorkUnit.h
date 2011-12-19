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
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Data/RandomGenerator.h>
# include "../Data/Rosetta.h"
# include "../Data/Pose.h"
# include <lbcpp/Data/RandomVariable.h>

# include <list>
# include <vector>

namespace lbcpp
{

typedef struct PoseAndScore
{
  PosePtr pose;
  double energy;
  DenseDoubleVectorPtr phis;
  DenseDoubleVectorPtr psis;
  PoseAndScore()
    : pose(NULL), energy(-1), phis(DenseDoubleVectorPtr()), psis(DenseDoubleVectorPtr()) {}
  PoseAndScore(PosePtr& p, double en, DenseDoubleVectorPtr& phi, DenseDoubleVectorPtr& psi)
    : pose(p), energy(en), phis(phi), psis(psi) {}
  PoseAndScore(const PoseAndScore& x)
    : pose(x.pose), energy(x.energy), phis(x.phis), psis(x.psis) {}
} PoseAndScore;

bool comparePoses(PoseAndScore first, PoseAndScore second)
  {return (first.energy < second.energy);}

class ProteinEDAOptimizationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    RosettaPtr rosetta = new Rosetta();
    rosetta->init(context, false);

    std::vector<ScalarVariableMean> minEnergies(numIterations);
    std::vector<ScalarVariableMean> averageEnergies(numIterations);

    RandomGeneratorPtr random = new RandomGenerator();

    context.enterScope(T("This protein"));

    PosePtr pose = new Pose(T("AAAAAAAAAA"));
    PosePtr minPose = new Pose(pose);
    double minEnergy = minPose->getEnergy();

    std::vector<ScalarVariableMean> phiMeans(pose->getLength());
    std::vector<ScalarVariableMean> psiMeans(pose->getLength());

    for (size_t i = 0; i < numIterations; i++)
    {
      std::list<PoseAndScore> scores;

      ScalarVariableMean meanEnergyAtThisIteration;

      for (size_t j = 0; j < numSamplesPerIteration; j++)
      {
        PosePtr tempPose = new Pose(pose);
        DenseDoubleVectorPtr phis = sampleAngles(random, phiMeans);
        DenseDoubleVectorPtr psis = sampleAngles(random, psiMeans);

        for (size_t k = 0; k < tempPose->getLength(); k++)
        {
          tempPose->setPhi(k, phis->getValue(k));
          tempPose->setPsi(k, psis->getValue(k));
        }

        double energy = tempPose->getEnergy();
        meanEnergyAtThisIteration.push(energy);

        if (energy < minEnergy)
        {
          *minPose = *tempPose;
          minEnergy = energy;
        }

        PoseAndScore sc(tempPose, energy, phis, psis);
        scores.push_back(sc);
      }

      scores.sort(comparePoses);
      context.progressCallback(new ProgressionState((size_t)(i + 1), numIterations,
          T("Interations")));

      context.enterScope(T("Iteration"));
      context.resultCallback(T("Iteration"), Variable((int)i));
      context.resultCallback(T("Minimal energy"), Variable(minEnergy));
      context.resultCallback(T("Average energy"), Variable(meanEnergyAtThisIteration.getMean()));
      context.leaveScope();

      minEnergies[i].push(minEnergy);
      averageEnergies[i].push(meanEnergyAtThisIteration.getMean());

      // learn
      for (size_t k = 0; k < phiMeans.size(); k++)
      {
        phiMeans[k].clear();
        psiMeans[k].clear();
      }

      for (size_t j = 0; j < numLearning; j++)
      {
        PoseAndScore tempSc(scores.front());
        for (size_t k = 0; k < pose->getLength(); k++)
        {
          phiMeans[k].push(tempSc.phis->getValue(k));
          psiMeans[k].push(tempSc.psis->getValue(k));
        }

        scores.pop_front();
      }

    }
    context.leaveScope();

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
    // export results
    context.enterScope(T("Results"));
    for (size_t k = 0; k < minEnergies.size(); k++)
    {
      context.enterScope(T("Values"));
      context.resultCallback(T("Iteration"), Variable((int)k));
      context.resultCallback(T("Min energy"), Variable(minEnergies[k].getMean()));
      context.resultCallback(T("Mean energy"), Variable(averageEnergies[k].getMean()));
      context.leaveScope();
    }
    context.leaveScope();

    context.informationCallback(T("Done."));

    return Variable();
  }

  DenseDoubleVectorPtr sampleAngles(RandomGeneratorPtr& random,
      std::vector<ScalarVariableMean>& means)
  {
    DenseDoubleVectorPtr values = new DenseDoubleVector(means.size(), 0);

    for (size_t i = 0; i < values->getNumElements(); i++)
      values->setValue(i, random->sampleDoubleFromGaussian(means[i].getMean(), 150));

    return values;
  }

protected:
  friend class ProteinEDAOptimizationWorkUnitClass;

  size_t numLearning;
  size_t numSamplesPerIteration;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_WORKUNIT_PROTEINEDAOPTIMIZATIONWORKUNIT_H_

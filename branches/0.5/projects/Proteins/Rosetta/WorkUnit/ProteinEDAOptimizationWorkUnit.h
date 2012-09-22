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
  double energy;
  DenseDoubleVectorPtr phis;
  DenseDoubleVectorPtr psis;
  PoseAndScore()
    : energy(-1), phis(DenseDoubleVectorPtr()), psis(DenseDoubleVectorPtr()) {}
  PoseAndScore(double en, DenseDoubleVectorPtr& phi, DenseDoubleVectorPtr& psi)
    : energy(en), phis(phi), psis(psi) {}
  PoseAndScore(const PoseAndScore& x)
    : energy(x.energy), phis(x.phis), psis(x.psis) {}
} PoseAndScore;

bool comparePoses(PoseAndScore first, PoseAndScore second)
  {return (first.energy < second.energy);}

class ProteinEDAOptimizationWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    RosettaPtr rosetta = new Rosetta();
    rosetta->init(context, false, 0, 0);

    std::vector<ScalarVariableMean> minEnergies(numIterations);
    std::vector<ScalarVariableMean> averageEnergies(numIterations);

    RandomGeneratorPtr random = new RandomGenerator();

    context.enterScope(T("This protein"));

    PosePtr pose = new Pose(T("AAAAA"));
    PosePtr minPose = pose->clone();
    PosePtr tempPose = pose->clone();
    double minEnergy = minPose->getEnergy();

    DenseDoubleVectorPtr phiMeans = new DenseDoubleVector(pose->getLength(), 0.0);
    DenseDoubleVectorPtr psiMeans = new DenseDoubleVector(pose->getLength(), 0.0);
    DenseDoubleVectorPtr phiVariances = new DenseDoubleVector(pose->getLength(), 50.0);
    DenseDoubleVectorPtr psiVariances = new DenseDoubleVector(pose->getLength(), 50.0);

    DenseDoubleVectorPtr phis;
    DenseDoubleVectorPtr psis;
    std::list<PoseAndScore> scores;
    ScalarVariableMean meanEnergyAtThisIteration;

    for (size_t i = 0; i < numIterations; i++)
    {
      scores.clear();
      meanEnergyAtThisIteration.clear();

      for (size_t j = 0; j < numSamplesPerIteration; j++)
      {
        phis = sampleAngles(random, phiMeans, phiVariances);
        psis = sampleAngles(random, psiMeans, psiVariances);

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

        PoseAndScore sc(energy, phis, psis);
        scores.push_back(sc);
      }

      // sorting
      scores.sort(comparePoses);

      // learn
      learnAnglesSimpleGaussian(scores, phiMeans, psiMeans, phiVariances, psiVariances);
//      learnAnglesSimpleGaussian(scores, phiMeans, psiMeans);

      // verbosity
      context.progressCallback(new ProgressionState((size_t)(i + 1), numIterations,
          T("Interations")));

      context.enterScope(T("Iteration"));
      context.resultCallback(T("Iteration"), Variable((int)i));
      context.resultCallback(T("Minimal energy"), Variable(minEnergy));
      context.resultCallback(T("Average energy"), Variable(meanEnergyAtThisIteration.getMean()));
      context.leaveScope();

      minEnergies[i].push(minEnergy);
      averageEnergies[i].push(meanEnergyAtThisIteration.getMean());
    }
    context.leaveScope();

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

  double checkAngleValidity(double angle)
    {return angle - 360 * std::floor((angle + 180) / 360.0);}

  DenseDoubleVectorPtr sampleAngles(RandomGeneratorPtr& random, DenseDoubleVectorPtr& means,
      DenseDoubleVectorPtr& vars)
  {
    DenseDoubleVectorPtr values = new DenseDoubleVector(means->getNumElements(), 0);

    for (size_t i = 0; i < values->getNumElements(); i++)
    {
      double sample = random->sampleDoubleFromGaussian(means->getValue(i), vars->getValue(i));
      values->setValue(i, checkAngleValidity(sample));
    }

    return values;
  }

  void learnAnglesSimpleGaussian(std::list<PoseAndScore>& scores, DenseDoubleVectorPtr& phiMeans,
      DenseDoubleVectorPtr& psiMeans, DenseDoubleVectorPtr phiVariances = DenseDoubleVectorPtr(),
      DenseDoubleVectorPtr psiVariances = DenseDoubleVectorPtr())
  {
    size_t numAvailableSamples = scores.size();
    std::vector<ScalarVariableMeanAndVariance> phiMeansCalculator(phiMeans->getNumElements());
    std::vector<ScalarVariableMeanAndVariance> psiMeansCalculator(psiMeans->getNumElements());

    for (size_t j = 0; (j < numLearning) && (j < numAvailableSamples); j++)
    {
      PoseAndScore *tempSc = &scores.front();

      for (size_t k = 0; k < phiMeans->getNumElements(); k++)
      {
        phiMeansCalculator[k].push(tempSc->phis->getValue(k));
        psiMeansCalculator[k].push(tempSc->psis->getValue(k));
      }

      scores.pop_front();
    }

    for (size_t k = 0; k < phiMeans->getNumElements(); k++)
    {
      phiMeans->setValue(k, phiMeansCalculator[k].getMean());
      psiMeans->setValue(k, psiMeansCalculator[k].getMean());
      if (phiVariances.get() != NULL)
        phiVariances->setValue(k, phiMeansCalculator[k].getVariance());
      if (psiVariances.get() != NULL)
        psiVariances->setValue(k, psiMeansCalculator[k].getVariance());
    }
  }

  void learnAnglesMeanShift(std::list<PoseAndScore>& scores, DenseDoubleVectorPtr& phiMeans,
      DenseDoubleVectorPtr& psiMeans, DenseDoubleVectorPtr phiVariances = DenseDoubleVectorPtr(),
      DenseDoubleVectorPtr psiVariances = DenseDoubleVectorPtr())
  {
//    size_t numAvailableSamples = scores.size();
//
//    for (size_t j = 0; (j < numLearning) && (j < numAvailableSamples); j++)
//    {
//      PoseAndScore tempSc(scores.front());
//
//      for (size_t k = 0; k < phiMeans.size(); k++)
//      {
//        phiMeans[k].push(tempSc.phis->getValue(k));
//        psiMeans[k].push(tempSc.psis->getValue(k));
//      }
//
//      scores.pop_front();
//    }
  }

protected:
  friend class ProteinEDAOptimizationWorkUnitClass;

  size_t numIterations;
  size_t numSamplesPerIteration;
  size_t numLearning;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_WORKUNIT_PROTEINEDAOPTIMIZATIONWORKUNIT_H_

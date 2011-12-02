/*-----------------------------------------.---------------------------------.
| Filename: ProteinEDAOptimizer.h          | ProteinEDAOptimizer             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 7 mai 2011  11:52:55           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_

# include "precompiled.h"

# include "RosettaUtils.h"
# include "Data/ProteinMover.h"
# include "ProteinSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "../Evaluator/QScoreEvaluator.h"

namespace lbcpp
{

struct MoverAndScore
{
  ProteinMoverPtr mover;
  double score;
  double deltaEnergy;
  double energy;
  double qScore;
  MoverAndScore() :
    mover(NULL), score(-1), deltaEnergy(-1), energy(-1), qScore(-1)
  {
  }
  MoverAndScore(ProteinMoverPtr& m, double s, double dE, double en, double qs) :
    mover(m), score(s), deltaEnergy(dE), energy(en), qScore(qs)
  {
  }
  MoverAndScore(const MoverAndScore& x) :
    mover(x.mover), score(x.score), deltaEnergy(x.deltaEnergy), energy(x.energy), qScore(x.qScore)
  {
  }
};

typedef struct MoverAndScore MoverAndScore;

bool compareMovers(MoverAndScore first, MoverAndScore second)
{
  return (first.score > second.score);
}

class ProteinEDAOptimizer;
typedef ReferenceCountedObjectPtr<ProteinEDAOptimizer> ProteinEDAOptimizerPtr;

class ProteinEDAOptimizer: public Object
{
public:
  ProteinEDAOptimizer() :
    Object()
  {
  }

  ProteinEDAOptimizer(double energyWeight) :
    Object(), energyWeight(energyWeight)
  {
  }

  MoverAndScore evaluate(ExecutionContext& context, const core::pose::PoseOP& target,
      const core::pose::PoseOP& reference, double energyBeforeMove, double scoreBeforeMove)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    int minDist =
        juce::jlimit(1, (int)target->n_residue(), juce::jmin(20, target->n_residue() / 2));
    int maxDist = -1;

    double realTargetEnergy = 0;
    double referenceEnergy = scoreBeforeMove;
    double targetEnergy = getConformationScore(target, fullAtomEnergy, &realTargetEnergy);
    double energyScore = 2 / (1 + std::exp(-0.0005 * (referenceEnergy - targetEnergy)));

    double structureScore = 0;
    QScoreObjectPtr scores = QScoreSingleEvaluator(target, reference, minDist, maxDist);

    if (scores.get() == NULL)
      context.errorCallback(
          T("Error in QScoreObject returned. Check that the two proteins are the same."));
    structureScore = scores->getMean();

    double globalScore = 0;
    if (energyWeight >= 0)
      globalScore = energyWeight * energyScore + (1 - energyWeight) * structureScore;
    else
      globalScore = energyScore * energyScore + juce::jmax(0.0, 1 - energyScore) * structureScore;

    MoverAndScore result;
    result.mover = ProteinMoverPtr();
    result.score = globalScore;
    result.deltaEnergy = energyBeforeMove - realTargetEnergy;
    result.energy = realTargetEnergy;
    result.qScore = structureScore;
    return result;
#else
    jassert(false);
    return MoverAndScore();
#endif // LBCPP_PROTEIN_ROSETTA
  }

  SamplerPtr findBestMovers(ExecutionContext& context, const RandomGeneratorPtr& random,
      const core::pose::PoseOP& target, const core::pose::PoseOP& reference, SamplerPtr sampler,
      std::vector<ProteinMoverPtr>& movers, size_t maxIterations, size_t numSamples = 1000,
      size_t numGoodSamples = 500, size_t numMoversToKeep = 20, bool includeBestMoversInLearning =
          true, DenseDoubleVectorPtr* energyMeans = NULL, DenseDoubleVectorPtr* qScoreMeans = NULL,
      DenseDoubleVectorPtr* scoreMeans = NULL)
  {
    SamplerPtr workingSampler = sampler->cloneAndCast<Sampler> ();
#ifdef LBCPP_PROTEIN_ROSETTA
    size_t numLearningSamplesFirstPass = numGoodSamples / 2;
    size_t numLearningSamplesSecondPass = numGoodSamples - numLearningSamplesFirstPass;

    core::pose::PoseOP workingPose = new core::pose::Pose(*target);
    double energyBeforeMove = 0;
    double scoreBeforeMove = getConformationScore(workingPose, fullAtomEnergy, &energyBeforeMove);
    MoverAndScore initialScore = evaluate(context, workingPose, reference, energyBeforeMove, scoreBeforeMove);

    std::list<MoverAndScore> moversToKeep;

    context.enterScope(T("Protein EDA optimizer."));

    for (size_t i = 0; i < maxIterations; i++)
    {
      context.progressCallback(new ProgressionState(i, maxIterations, T("Iterations")));
      std::list<MoverAndScore> tempList;
      ScalarVariableMeanAndVariance scoreRandomVariable;
      ScalarVariableMeanAndVariance deltaEnergyRandomVariable;
      ScalarVariableMeanAndVariance energyRandomVariable;
      ScalarVariableMeanAndVariance qScoreRandomVariable;

      for (size_t j = 0; j < numSamples; j++)
      {
        ProteinMoverPtr mover = workingSampler->sample(context, random, NULL).getObjectAndCast<
            ProteinMover> ();

        mover->move(workingPose);

        MoverAndScore candidate = evaluate(context, workingPose, reference, energyBeforeMove, scoreBeforeMove);
        candidate.mover = mover;

        scoreRandomVariable.push(candidate.score - initialScore.score);
        deltaEnergyRandomVariable.push(candidate.deltaEnergy);
        energyRandomVariable.push(candidate.energy);
        qScoreRandomVariable.push(candidate.qScore);

        tempList.push_back(candidate);
        *workingPose = *target;

        moversToKeep.push_back(MoverAndScore(candidate));
      }

      // sort movers
      tempList.sort(compareMovers);

      // evaluate performance of the iteration
      context.enterScope(T("Values"));
      context.resultCallback(T("Iteration"), Variable(i));
      context.resultCallback(T("Mean delta score"), Variable(scoreRandomVariable.getMean()));
      context.resultCallback(T("Std Dev delta score"), Variable(scoreRandomVariable.getStandardDeviation()));
      if (scoreMeans != NULL)
        (*scoreMeans)->setValue(i, scoreRandomVariable.getMean());
      double meanDeltaEnergy = deltaEnergyRandomVariable.getMean();
      context.resultCallback(T("Mean deltaEnergy"), Variable(meanDeltaEnergy));
      if (energyMeans != NULL)
        (*energyMeans)->setValue(i, meanDeltaEnergy);
      context.resultCallback(T("Std Dev deltaEnergy"), Variable(deltaEnergyRandomVariable.getStandardDeviation()));
      context.resultCallback(T("Mean energy"), Variable(energyRandomVariable.getMean()));
      context.resultCallback(T("Std Dev energy"), Variable(energyRandomVariable.getStandardDeviation()));
      context.resultCallback(T("Mean qScore"), Variable(qScoreRandomVariable.getMean()));
      if (qScoreMeans != NULL)
        (*qScoreMeans)->setValue(i, qScoreRandomVariable.getMean());
      context.resultCallback(T("Std Dev qScore"), Variable(qScoreRandomVariable.getStandardDeviation()));
      context.leaveScope(Variable(meanDeltaEnergy));

      // best samples ever seen
      moversToKeep.sort(compareMovers);
      while (moversToKeep.size() > juce::jmax((int)numGoodSamples, (int)numMoversToKeep))
        moversToKeep.pop_back();

      // keep best samplers
      std::vector<MoverAndScore> moversVector;

      if (includeBestMoversInLearning)
      {
        moversVector.clear();
        moversVector.resize(moversToKeep.size());
        std::list<MoverAndScore>::iterator it;
        size_t k = 0;
        for (it = moversToKeep.begin(); it != moversToKeep.end(); it++)
          moversVector[k++] = *it;
      }
      else
      {
        if (numGoodSamples > numSamples)
        {
          jassert(false);
          numGoodSamples = numSamples;
          numLearningSamplesFirstPass = numGoodSamples / 2;
          numLearningSamplesSecondPass = numGoodSamples - numLearningSamplesFirstPass;
        }
        moversVector = std::vector<MoverAndScore>(numGoodSamples);
        for (size_t j = 0; j < numLearningSamplesFirstPass; j++)
        {
          moversVector[j] = MoverAndScore(tempList.front());
          tempList.pop_front();
        }

        std::vector<MoverAndScore> rest(tempList.size());
        for (size_t j = 0; j < rest.size(); j++)
        {
          rest[j] = MoverAndScore(tempList.front());
          tempList.pop_front();
        }

        std::vector<size_t> ordering;
        random->sampleOrder((size_t)(rest.size()), ordering);

        for (size_t j = 0; j < numLearningSamplesSecondPass; j++)
          moversVector[numLearningSamplesFirstPass + j] = MoverAndScore(rest[ordering[j]]);
      }

      ObjectVectorPtr dataset = new ObjectVector(proteinMoverClass, 0);
      for (size_t j = 0; j < moversVector.size(); j++)
        dataset->append(moversVector[j].mover);
      if (dataset->getNumElements() >= numGoodSamples)
        workingSampler->learn(context, ContainerPtr(), dataset);
    }

    movers = std::vector<ProteinMoverPtr>(numMoversToKeep);
    for (size_t i = 0; i < numMoversToKeep; i++)
    {
      movers[i] = moversToKeep.front().mover;
      moversToKeep.pop_front();
    }

    context.progressCallback(new ProgressionState((double)maxIterations, (double)maxIterations,
        T("Iterations")));
    context.leaveScope();
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA

    return workingSampler;
  }

protected:
  friend class ProteinEDAOptimizerClass;

  double energyWeight;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_

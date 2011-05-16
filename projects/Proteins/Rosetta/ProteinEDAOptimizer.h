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
# include "ProteinMover.h"
# include "Sampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "../Evaluator/QScoreEvaluator.h"

namespace lbcpp
{
struct MoverAndScore
{
  ProteinMoverPtr mover;
  double score;
  MoverAndScore() :
    mover(NULL), score(-1)
  {
  }
  MoverAndScore(ProteinMoverPtr& m, double s) :
    mover(m), score(s)
  {
  }
  MoverAndScore(const MoverAndScore& x) :
    mover(x.mover), score(x.score)
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

  double evaluate(ExecutionContext& context, const core::pose::PoseOP& target, const core::pose::PoseOP& reference)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    int minDist =
        juce::jlimit(1, (int)target->n_residue(), juce::jmin(20, target->n_residue() / 2));
    int maxDist = -1;

    double referenceEnergy = getConformationScore(reference, fullAtomEnergy);
    double targetEnergy = getConformationScore(target, fullAtomEnergy);
    double energyScore;
    if (targetEnergy == 0)
      energyScore = std::numeric_limits<double>::max();
    else
      energyScore = juce::jmax(0.0, referenceEnergy / targetEnergy);

    double structureScore = 0;
    QScoreObjectPtr scores = QScoreSingleEvaluator(target, reference, minDist, maxDist);

    if (scores.get() == NULL)
      context.errorCallback(
          T("Error in QScoreObject returned. Check that the two proteins are the same."));
    structureScore = scores->getMean();

    if (energyWeight >= 0)
      return energyWeight * energyScore + (1 - energyWeight) * structureScore;
    else
      return energyScore * energyScore + juce::jmax(0.0, 1 - energyScore) * structureScore;
#else
    jassert(false);
    return 0.0;
#endif // LBCPP_PROTEIN_ROSETTA
  }

  SamplerPtr findBestMovers(ExecutionContext& context,
      const RandomGeneratorPtr& random, const core::pose::PoseOP& target,
      const core::pose::PoseOP& reference, SamplerPtr sampler, std::vector<
          ProteinMoverPtr>& movers, size_t maxIterations, size_t numSamples = 1000,
      double ratioGoodSamples = 0.5, size_t numMoversToKeep = 20)
  {
    SamplerPtr workingSampler = sampler->cloneAndCast<Sampler>();
#ifdef LBCPP_PROTEIN_ROSETTA
    core::pose::PoseOP workingPose = new core::pose::Pose(*target);

    std::list<MoverAndScore> moversToKeep;

    context.enterScope(T("Protein EDA optimizer."));

    for (size_t i = 0; i < maxIterations; i++)
    {
      context.progressCallback(new ProgressionState(i, maxIterations, T("Iterations")));
      std::list<MoverAndScore> tempList;
      for (size_t j = 0; j < numSamples; j++)
      {
        ProteinMoverPtr mover = workingSampler->sample(context, random, NULL).getObjectAndCast<ProteinMover>();

        mover->move(workingPose);
        double score = evaluate(context, workingPose, reference);

        MoverAndScore candidate(mover, score);
        tempList.push_back(candidate);
        *workingPose = *target;

        moversToKeep.push_back(MoverAndScore(candidate));
      }

      tempList.sort(compareMovers);

      moversToKeep.sort(compareMovers);
      while (moversToKeep.size() > numMoversToKeep)
        moversToKeep.pop_back();

      size_t numLearningSamples = (size_t)(numSamples * ratioGoodSamples);
      size_t numLearningSamplesFirstPass = numLearningSamples / 2;
      size_t numLearningSamplesSecondPass = numLearningSamples - numLearningSamplesFirstPass;
      std::vector<MoverAndScore> moversVector(numLearningSamples);
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

      std::vector<Variable> dataset(numLearningSamples);
      for (size_t j = 0; j < numLearningSamples; j++)
        dataset[j] = moversVector[j].mover;
      workingSampler->learn(context, dataset);
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

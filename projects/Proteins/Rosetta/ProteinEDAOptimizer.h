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

  double evaluate(ExecutionContext& context, core::pose::PoseOP target,
      core::pose::PoseOP reference)
  {
    int minDist =
        juce::jlimit(1, (int)target->n_residue(), juce::jmin(20, target->n_residue() / 2));
    int maxDist = -1;

    double referenceEnergy = getConformationScore(reference);
    double targetEnergy = getConformationScore(target);
    double energyScore;
    if (targetEnergy == 0)
      energyScore = std::numeric_limits<double>::max();
    else
      energyScore = juce::jmax(0.0, referenceEnergy / targetEnergy);

    double structureScore = 0;
    QScoreObjectPtr scores = QScoreSingleEvaluator(convertPoseToProtein(context, target),
        convertPoseToProtein(context, reference), minDist, maxDist);

    if (scores.get() == NULL)
      context.errorCallback(
          T("Error in QScoreObject returned. Check that the two proteins are the same."));
    structureScore = scores->getMean();

    if (energyWeight >= 0)
      return energyWeight * energyScore + (1 - energyWeight) * structureScore;
    else
      return energyScore * energyScore + juce::jmax(0.0, 1 - energyScore) * structureScore;
  }

  ProteinMoverSamplerPtr findBestMovers(ExecutionContext& context, RandomGeneratorPtr& random,
      core::pose::PoseOP target, core::pose::PoseOP reference, ProteinMoverSamplerPtr sampler,
      int maxIterations, int numSamples = 1000, double ratioGoodSamples = 0.5)
  {
    ProteinMoverSamplerPtr workingSampler = new ProteinMoverSampler(*sampler);
    core::pose::PoseOP workingPose = new core::pose::Pose(*target);

    context.enterScope(T("Protein EDA optimizer."));

    for (int i = 0; i < maxIterations; i++)
    {
      context.progressCallback(new ProgressionState((double)i, (double)maxIterations,
          T("Iterations")));
      std::list<MoverAndScore> tempList;
      for (int j = 0; j < numSamples; j++)
      {
        ProteinMoverPtr mover = workingSampler->sample(context, random, NULL).getObjectAndCast<
            ProteinMover> ();

        mover->move(workingPose);
        double score = evaluate(context, workingPose, reference);

        MoverAndScore candidate(mover, score);
        tempList.push_back(candidate);
        *workingPose = *target;
      }

      tempList.sort(compareMovers);

      size_t numLearningSamples = (size_t)(numSamples * ratioGoodSamples);
      std::vector<MoverAndScore> moversVector(numLearningSamples);
      for (int j = 0; j < numLearningSamples; j++)
      {
        moversVector[j] = MoverAndScore(tempList.front());
        tempList.pop_front();
      }

      std::vector<size_t> ordering;
      random->sampleOrder((size_t)(numSamples * ratioGoodSamples), ordering);

      std::vector<std::pair<Variable, Variable> > dataset(numLearningSamples / 2);
      for (int j = 0; j < numLearningSamples / 2; j++)
        dataset[j] = std::pair<Variable, Variable>(Variable(moversVector[ordering[j]].mover),
            Variable());

      workingSampler->learn(context, random, dataset);
    }
    context.progressCallback(new ProgressionState((double)maxIterations, (double)maxIterations,
        T("Iterations")));
    context.leaveScope();
    return workingSampler;
  }

protected:
  friend class ProteinEDAOptimizerClass;
  double energyWeight;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_

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
  MoverAndScore(ProteinMoverPtr m, double s) :
    mover(m->clone()), score(s)
  {
  }
  MoverAndScore(const MoverAndScore& x) :
    mover(x.mover->clone()), score(x.score)
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

  double evaluate(ExecutionContext& context, core::pose::PoseOP target,
      core::pose::PoseOP reference)
  {
    double energyWeight = 0.2;
    int minDist =
        juce::jlimit(1, (int)target->n_residue(), juce::jmin(20, target->n_residue() / 2));
    int maxDist = -1;

    double referenceEnergy = getConformationScore(reference);
    double targetEnergy = getConformationScore(target);
    double energyScore = juce::jlimit(0.0, 1.0, targetEnergy / referenceEnergy);

    double structureScore = 0;
    QScoreObjectPtr scores = QScoreSingleEvaluator(convertPoseToProtein(context, target),
        convertPoseToProtein(context, reference), minDist, maxDist);
    structureScore = scores->getMean();

    return energyWeight * energyScore + (1 - energyWeight) * structureScore;
  }

  ProteinMoverSamplerPtr findBestMovers(ExecutionContext& context, RandomGeneratorPtr& random,
      core::pose::PoseOP target, core::pose::PoseOP reference, ProteinMoverSamplerPtr sampler,
      int maxIterations, int numSamples = 1000, double ratioGoodSamples = 0.5)
  {
    ProteinMoverSamplerPtr workingSampler = new ProteinMoverSampler(*sampler);
    core::pose::PoseOP workingPose = new core::pose::Pose(*target);

    for (int i = 0; i < maxIterations; i++)
    {
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

      std::vector<std::pair<Variable, Variable> > dataset;
      for (int j = 0; j < numSamples * ratioGoodSamples; j++)
      {
        dataset.push_back(std::pair<Variable, Variable>(Variable(tempList.front().mover),
            Variable()));
        tempList.pop_front();
      }

      workingSampler->learn(context, random, dataset);

    }

    return workingSampler;
  }

protected:
  friend class ProteinEDAOptimizerClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_

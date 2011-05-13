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
      std::vector<ProteinMoverPtr>& movers, size_t maxIterations, size_t numSamples = 1000,
      double ratioGoodSamples = 0.5, size_t numMoversToKeep = 20)
  {
    ProteinMoverSamplerPtr workingSampler = new ProteinMoverSampler(*sampler);
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

        // TEST
        printMover(mover);
        std::cout << "score : " << score << std::endl;
        //FIN TEST

        MoverAndScore candidate(mover, score);
        tempList.push_back(candidate);
        *workingPose = *target;

        moversToKeep.push_back(MoverAndScore(candidate));
      }

      tempList.sort(compareMovers);

      // TEST
      std::list<MoverAndScore>::iterator it;
      std::cout << "============= tempList ===========" << std::endl;
      for ( it=tempList.begin() ; it != tempList.end(); it++ )
        printMover((*it).mover);
      // FIN TEST

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

      // TEST
      std::cout << "numLearningSamples : " << numLearningSamples << std::endl;
      std::cout << "numLearningSamplesFirstPass : " <<  numLearningSamplesFirstPass << std::endl;
      std::cout << "numLearningSampelsSecondPass : " << numLearningSamplesSecondPass << std::endl;
      std::cout << "======================= chosen movers : " << moversVector.size() << "==================== "<< std::endl;
      for (size_t j = 0; j < moversVector.size(); j++)
      {
        printMover(moversVector[j].mover);
      }
      std::cout << "=================== affichage de rest : ========================" << rest.size() << std::endl;
      for (size_t j = 0; j < rest.size(); j++)
      {
        printMover(rest[j].mover);
      }
      // FIN TEST

      std::vector<Variable> dataset(numLearningSamples);
      for (size_t j = 0; j < numLearningSamples; j++)
        dataset[j] = moversVector[j].mover;
      workingSampler->learn(context, dataset);

      // TEST
      workingSampler->saveToFile(context, context.getFile(T("sampler_temp_") + String(i)
          +T(".xml")));
      // FIN TEST
    }

    // TEST
    std::cout << " ==================== best ever movers =====================" << std::endl;
    // FIN TEST

    movers = std::vector<ProteinMoverPtr>(numMoversToKeep);
    for (size_t i = 0; i < numMoversToKeep; i++)
    {
      // TEST
      printMover(moversToKeep.front().mover);
      std::cout << "score : " << moversToKeep.front().score << std::endl;
      // FIN TEST

      movers[i] = moversToKeep.front().mover;
      moversToKeep.pop_front();
    }

    context.progressCallback(new ProgressionState((double)maxIterations, (double)maxIterations,
        T("Iterations")));
    context.leaveScope();
    return workingSampler;
  }

  void printMover(ProteinMoverPtr& t)
  {
    if (t.isInstanceOf<PhiPsiMover> ())
    {
      PhiPsiMoverPtr t0 = (PhiPsiMoverPtr)t;
      std::cout << "phipsi" << " r : " << t0->getResidueIndex() << ", phi : " << t0->getDeltaPhi()
          << ", psi : " << t0->getDeltaPsi() << std::endl;
    }
    else if (t.isInstanceOf<ShearMover> ())
    {
      ShearMoverPtr t0 = (ShearMoverPtr)t;
      std::cout << "shear " << " r : " << t0->getResidueIndex() << ", phi : " << t0->getDeltaPhi()
          << ", psi : " << t0->getDeltaPsi() << std::endl;
    }
    else if (t.isInstanceOf<RigidBodyTransMover> ())
    {
      RigidBodyTransMoverPtr t0 = (RigidBodyTransMoverPtr)t;
      std::cout << "trans" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
          << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << std::endl;
    }
    else if (t.isInstanceOf<RigidBodySpinMover> ())
    {
      RigidBodySpinMoverPtr t0 = (RigidBodySpinMoverPtr)t;
      std::cout << "spin" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
          << t0->getIndexResidueTwo() << ", amplitude : " << t0->getAmplitude() << std::endl;
    }
    else if (t.isInstanceOf<RigidBodyGeneralMover> ())
    {
      RigidBodyGeneralMoverPtr t0 = (RigidBodyGeneralMoverPtr)t;
      std::cout << "general" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
          << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << ", amplitude : "
          << t0->getAmplitude() << std::endl;
    }
    else
    {
      std::cout << "autre mover......" << std::endl;
    }
  }

protected:
  friend class ProteinEDAOptimizerClass;
  double energyWeight;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_EDA_OPTIMIZER_H_

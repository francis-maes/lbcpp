/*-----------------------------------------.---------------------------------.
| Filename: Policy.cpp                     | Policies                        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/lbcpp.h>
#include "Policy/RandomPolicy.h"
#include "Policy/GreedyPolicy.h"
#include "Policy/StochasticPolicy.h"
#include "Policy/EpsilonGreedyPolicy.h"
#include "Policy/GibbsGreedyPolicy.h"
#include "Policy/MixturePolicy.h"
#include "Policy/ComputeStatisticsPolicy.h"
#include "Policy/VerbosePolicy.h"
#include "Policy/ClassificationExamplesCreatorPolicy.h"
#include "Policy/RankingExamplesCreatorPolicy.h"
#include "Policy/QLearningPolicy.h"
#include "Policy/MonteCarloControlPolicy.h"
#include "Policy/GPOMDPPolicy.h"

using namespace lbcpp;

/*
** Policy
*/
bool Policy::run(CRAlgorithmPtr crAlgorithm, PolicyStatisticsPtr statistics)
{
  PolicyPtr policy = statistics ? computeStatisticsPolicy(this, statistics) : PolicyPtr(this);
  return crAlgorithm->cloneAndCast<CRAlgorithm>()->run(policy);
}

bool Policy::run(ObjectStreamPtr crAlgorithms, PolicyStatisticsPtr statistics, ProgressCallbackPtr progress)
{
  if (progress)
    progress->progressStart("Policy::run");
  PolicyPtr policy = statistics ? computeStatisticsPolicy(this, statistics) : PolicyPtr(this);
  bool res = true;
  size_t i = 0;
  while (true)
  {
    CRAlgorithmPtr crAlgorithm = crAlgorithms->nextAndCast<CRAlgorithm>();
    if (!crAlgorithm)
      break;
    if (progress && !progress->progressStep("Policy::run", (double)i))
      break;
    ++i;
    if (!policy->run(crAlgorithm))
    {
      res = false;
      break;
    }
  }
  if (progress)
    progress->progressEnd();
  return res;
}

bool Policy::run(ObjectContainerPtr crAlgorithms, PolicyStatisticsPtr statistics, ProgressCallbackPtr progress)
{
  if (progress)
    progress->progressStart("Policy::run");
  PolicyPtr policy = statistics ? computeStatisticsPolicy(this, statistics) : PolicyPtr(this);
  size_t n = crAlgorithms->size();
  for (size_t i = 0; i < n; ++i)
  {
    CRAlgorithmPtr crAlgorithm = crAlgorithms->getAndCast<CRAlgorithm>(i);
    if (progress && !progress->progressStep("Policy::run", (double)i, (double)n))
      return false;
    if (crAlgorithm && !policy->run(crAlgorithm->cloneAndCast<CRAlgorithm>()))
      return false;
  }
  if (progress)
    progress->progressEnd();
  return true;
}

ObjectPtr Policy::getResultWithName(const String& name) const
{
  for (size_t i = 0; i < getNumResults(); ++i)
  {
    ObjectPtr res = getResult(i);
    if (res->getName() == name)
      return res;
  }
  error("Policy::getResultWithName", "Could not find result with name '" + name + "'");
  return ObjectPtr();
}

PolicyPtr Policy::verbose(size_t verbosity, std::ostream& ostr) const
  {return new VerbosePolicy(const_cast<Policy* >(this), verbosity, ostr);}

PolicyPtr lbcpp::randomPolicy()
  {return new RandomPolicy();}

PolicyPtr lbcpp::greedyPolicy(ActionValueFunctionPtr actionValues)
  {return new GreedyPolicy(actionValues);}

PolicyPtr lbcpp::gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature)
  {return new GibbsGreedyPolicy(actionValue, temperature);}

PolicyPtr lbcpp::stochasticPolicy(ActionValueFunctionPtr actionProbabilities)
  {return new StochasticPolicy(actionProbabilities);}

PolicyPtr lbcpp::mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient)
  {return new MixturePolicy(policy1, policy2, mixtureCoefficient);}

PolicyPtr lbcpp::epsilonGreedyPolicy(PolicyPtr basePolicy, IterationFunctionPtr epsilon)
  {return new EpsilonGreedyPolicy(basePolicy, epsilon);}

PolicyPtr lbcpp::computeStatisticsPolicy(PolicyPtr policy, PolicyStatisticsPtr statistics)
  {return new ComputeStatisticsPolicy(policy, statistics);}

PolicyPtr lbcpp::classificationExamplesCreatorPolicy(PolicyPtr explorationPolicy, ClassifierPtr classifier, ActionValueFunctionPtr supervisor)
  {return new ClassificationExamplesCreatorPolicy(explorationPolicy, classifier, supervisor);}

PolicyPtr lbcpp::rankingExamplesCreatorPolicy(PolicyPtr explorationPolicy, RankerPtr ranker, ActionValueFunctionPtr supervisor)
  {return new RankingExamplesCreatorPolicy(explorationPolicy, ranker, supervisor);}

PolicyPtr lbcpp::qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
  {return new QLearningPolicy(explorationPolicy, regressor, discount, false);}

PolicyPtr lbcpp::sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
  {return new QLearningPolicy(explorationPolicy, regressor, discount, true);}

PolicyPtr lbcpp::monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
  {return new MonteCarloControlPolicy(explorationPolicy, regressor, discount);}

PolicyPtr lbcpp::gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration)
  {return new GPOMDPPolicy(classifier, beta, exploration);}

PolicyPtr lbcpp::gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, PolicyPtr explorationPolicy)
  {return new GPOMDPPolicy(classifier, beta, explorationPolicy);}

/*
** EpisodicPolicy
*/
EpisodicPolicy::EpisodicPolicy()
  : inclusionLevel(0), stepNumber(0)
{
}

void EpisodicPolicy::policyEnter(CRAlgorithmPtr crAlgorithm)
{
  if (inclusionLevel == 0)
  {
    currentReward = 0.0;
    stepNumber = 0;
  }
  ++inclusionLevel;
}

VariablePtr EpisodicPolicy::policyChoose(ChoosePtr choose)
{
  VariablePtr res;
  if (stepNumber == 0)
    res = policyStart(choose);
  else
    res = policyStep(currentReward, choose);
  currentReward = 0.0;
  ++stepNumber;
  return res;
}
  
void EpisodicPolicy::policyReward(double reward)
{
  currentReward += reward;
}

void EpisodicPolicy::policyLeave()
{
  jassert(inclusionLevel > 0);
  --inclusionLevel;
  if (inclusionLevel == 0)
  {
    policyEnd(currentReward);
    currentReward = 0.0;
  }
}

/*
** PolicyStatistics
*/
PolicyStatistics::PolicyStatistics()
  : rewardPerChoose(new ScalarVariableStatistics("rewardPerChoose")),
    choicesPerChoose(new ScalarVariableStatistics("choicesPerChoose")),
    rewardPerEpisode(new ScalarVariableStatistics("rewardPerEpisode")),
    choosesPerEpisode(new ScalarVariableStatistics("choosesPerEpisode")),
    choicesPerEpisode(new ScalarVariableStatistics("choicesPerEpisode"))
{
}

String PolicyStatistics::toString() const
{
  return "reward/choose:   " + rewardPerChoose->toString() + "\n" +
         "choices/choose:  " + choicesPerChoose->toString() + "\n" +
         "reward/episode:  " + rewardPerEpisode->toString() + "\n" +
         "chooses/episode: " + choosesPerEpisode->toString() + "\n" +
         "choices/epsiode: " + choicesPerEpisode->toString() + "\n";
}

/*
** Serializable classes declaration
*/
void declarePolicies()
{
  LBCPP_DECLARE_CLASS(RandomPolicy);
  LBCPP_DECLARE_CLASS(GreedyPolicy);
  LBCPP_DECLARE_CLASS(GibbsGreedyPolicy);
  LBCPP_DECLARE_CLASS(StochasticPolicy);
  LBCPP_DECLARE_CLASS(MixturePolicy);
  LBCPP_DECLARE_CLASS(EpsilonGreedyPolicy);
  LBCPP_DECLARE_CLASS(ComputeStatisticsPolicy);
  LBCPP_DECLARE_CLASS(VerbosePolicy);
  LBCPP_DECLARE_CLASS(ClassificationExamplesCreatorPolicy);
  LBCPP_DECLARE_CLASS(RankingExamplesCreatorPolicy);
  LBCPP_DECLARE_CLASS(GPOMDPPolicy);
  LBCPP_DECLARE_CLASS(MonteCarloControlPolicy);
  LBCPP_DECLARE_CLASS(QLearningPolicy);
}

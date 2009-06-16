/*-----------------------------------------.---------------------------------.
| Filename: Policy.cpp                     | Policies                        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/lbcpp.h>
#include <lbcpp/impl/impl.h> // tmp

#include "Policy/RandomPolicy.h"
#include "Policy/GreedyPolicy.h"
#include "Policy/StochasticPolicy.h"
#include "Policy/EpsilonGreedyPolicy.h"
#include "Policy/GibbsGreedyPolicy.h"
#include "Policy/MixturePolicy.h"

using namespace lbcpp;

ObjectPtr Policy::getResultWithName(const std::string& name) const
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

bool Policy::run(CRAlgorithmPtr crAlgorithm)
{
  return crAlgorithm->cloneAndCast<CRAlgorithm>()->run(this);
}

bool Policy::run(ObjectStreamPtr crAlgorithms, ProgressCallbackPtr progress)
{
  if (progress)
    progress->progressStart("Policy::run");
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
    if (!run(crAlgorithm->cloneAndCast<CRAlgorithm>()))
    {
      res = false;
      break;
    }
  }
  if (progress)
    progress->progressEnd();
  return res;
}

bool Policy::run(ObjectContainerPtr crAlgorithms, ProgressCallbackPtr progress)
{
  if (progress)
    progress->progressStart("Policy::run");
  size_t n = crAlgorithms->size();
  for (size_t i = 0; i < n; ++i)
  {
    CRAlgorithmPtr crAlgorithm = crAlgorithms->getAndCast<CRAlgorithm>(i);
    if (progress && !progress->progressStep("Policy::run", (double)i, (double)n))
      return false;
    if (crAlgorithm && !run(crAlgorithm->cloneAndCast<CRAlgorithm>()))
      return false;
  }
  if (progress)
    progress->progressEnd();
  return true;
}

// 

inline impl::DynamicToStaticPolicy dynamicToStatic(const Policy* policy)
  {return impl::dynamicToStatic(PolicyPtr(const_cast<Policy* >(policy)));}

inline impl::DynamicToStaticPolicy dynamicToStatic(const PolicyPtr policy)
  {return impl::dynamicToStatic(policy);}

// 

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

PolicyPtr lbcpp::qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::QLearningPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount, false));
}

PolicyPtr lbcpp::sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::QLearningPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount, true));
}

PolicyPtr lbcpp::monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::MonteCarloControlPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount));
}

PolicyPtr lbcpp::classificationExampleCreatorPolicy(PolicyPtr explorationPolicy,
          ClassifierPtr classifier, ActionValueFunctionPtr supervisor)
{
  return impl::staticToDynamic(impl::ClassificationExampleCreatorPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(explorationPolicy), classifier, supervisor));
}


PolicyPtr lbcpp::rankingExampleCreatorPolicy(PolicyPtr explorationPolicy,
          RankerPtr ranker, ActionValueFunctionPtr supervisor)
{
  return impl::staticToDynamic(impl::RankingExampleCreatorPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(explorationPolicy), ranker, supervisor));
}

PolicyPtr lbcpp::gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration)
  {return impl::staticToDynamic(impl::GPOMDPPolicy(classifier, beta, exploration));}

PolicyPtr lbcpp::gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, PolicyPtr explorationPolicy)
  {return impl::staticToDynamic(impl::GPOMDPPolicy(classifier, beta, explorationPolicy));}

PolicyPtr Policy::verbose(size_t verbosity, std::ostream& ostr) const
{
  return impl::staticToDynamic(impl::VerbosePolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this), verbosity, ostr));
}

PolicyPtr Policy::addComputeStatistics() const
  {return impl::staticToDynamic(impl::ComputeStatisticsPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this)));}

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
}

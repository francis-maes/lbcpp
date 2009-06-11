/*-----------------------------------------.---------------------------------.
| Filename: Policy.cpp                     | Policies                        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/lbcpp.h>
#include <lbcpp/impl/impl.h>
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
  return crAlgorithm->run(this);
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
  {return impl::staticToDynamic(impl::RandomPolicy());}

PolicyPtr lbcpp::greedyPolicy(ActionValueFunctionPtr actionValues)
  {return impl::staticToDynamic(impl::GreedyPolicy(actionValues));}

PolicyPtr lbcpp::gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature)
  {return impl::staticToDynamic(impl::GibbsGreedyPolicy(actionValue, temperature));}

PolicyPtr lbcpp::stochasticPolicy(ActionValueFunctionPtr actionProbabilities)
  {return impl::staticToDynamic(impl::NonDeterministicPolicy(actionProbabilities));}

// todo: ranger
class MixturePolicy : public Policy
{
public:
  MixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient)
    : policy1(policy1), policy2(policy2), mixtureCoefficient(mixtureCoefficient) {}
    
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    policy1->policyEnter(crAlgorithm);
    policy2->policyEnter(crAlgorithm);
  }
  
  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr choice1 = policy1->policyChoose(choose);
    VariablePtr choice2 = policy2->policyChoose(choose);
    return Random::getInstance().sampleBool(mixtureCoefficient) ? choice2 : choice1;
  }
  
  virtual void policyReward(double reward)
  {
    policy1->policyReward(reward);
    policy2->policyReward(reward);
  }
  
  virtual void policyLeave()
  {
    policy1->policyLeave();
    policy2->policyLeave();
  }
  
private:
  PolicyPtr policy1;
  PolicyPtr policy2;
  double mixtureCoefficient;
};

PolicyPtr lbcpp::mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient)
{
  return new MixturePolicy(policy1, policy2, mixtureCoefficient);
}

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

PolicyPtr Policy::epsilonGreedy(IterationFunctionPtr epsilon) const
  {return impl::staticToDynamic(impl::EpsilonGreedyPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this), epsilon));}
  
PolicyPtr Policy::addComputeStatistics() const
  {return impl::staticToDynamic(impl::ComputeStatisticsPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this)));}

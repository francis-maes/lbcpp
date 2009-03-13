/*-----------------------------------------.---------------------------------.
| Filename: QLearningPolicy.hpp            | QLearning/Sarsa Policy          |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 02:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_QLEARNING_H_
# define CRALGO_IMPL_POLICY_QLEARNING_H_

# include "PolicyStatic.hpp"
# include "../../RandomVariable.h"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct QLearningPolicy
  : public DecoratorPolicy<QLearningPolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<QLearningPolicy<DecoratedType>, DecoratedType> BaseClass;
  
  QLearningPolicy(const DecoratedType& decorated, RegressorPtr regressor, double discount, bool useSarsaRule = false)
    : BaseClass(decorated), regressor(regressor), discount(discount), useSarsaRule(useSarsaRule),
      predictionError(new ScalarRandomVariableStatistics("predictionError"))
    {}
  
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    // FIXME: check flat
    BaseClass::policyEnter(crAlgorithm);
    lastActionDescription = SparseVectorPtr();
    currentReward = 0.0;
  }
  
  ////////////////////
  VariablePtr policyStart(ChoosePtr choose)
  {
    regressor->trainStochasticBegin();
    VariablePtr res = BaseClass::policyChoose(choose);
    lastActionDescription = choose->computeActionFeatures(res)->toSparseVector();
    return res;
  }
  
  VariablePtr policyStep(ChoosePtr choose, double reward)
  {
    assert(lastActionDescription);
    VariablePtr res = BaseClass::policyChoose(choose);
    
    VariablePtr nextAction;
    if (useSarsaRule)
      nextAction = res;
    else
    {
      // todo...
    }
    
    // todo: selection of next action 
    //   qlearning: predicted, sampleBests(RegressorActionValue)
    //        -> optimize if we explore with the predicted strategy
    //   sarsa: explored, ok.
    
    SparseVectorPtr nextActionDescription = choose->computeActionFeatures(res)->toSparseVector();
    regressor->trainStochasticExample(RegressionExample(lastActionDescription, 
        reward + discount * regressor->predict(nextActionDescription)));
    lastActionDescription = nextActionDescription;
    return res;
  }
  
  void policyEnd(double reward)
  {
    if (lastActionDescription)
      regressor->trainStochasticExample(RegressionExample(lastActionDescription, reward));
    regressor->trainStochasticEnd();
  }
  ////////////////////

  VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr res;
    if (lastActionDescription)
      res = policyStep(choose, currentReward);
    else
      res = policyStart(choose);
    currentReward = 0.0;
    return res;
  }

  void policyReward(double reward)
  {
    currentReward += reward;
    BaseClass::policyReward(reward);
  }
  
  void policyLeave()
  {
    if (lastActionDescription)
    {
      policyEnd(currentReward);
      currentReward = 0.0;
      lastActionDescription = SparseVectorPtr();
    }
    BaseClass::policyLeave();    
  }
  
  size_t getNumResults() const
    {return 1 + BaseClass::getNumResults();}
    
  ObjectPtr getResult(size_t i) const
  {
    if (i == 0)
      return predictionError;
    else
      return BaseClass::getResult(i - 1);
  }

private:
  RegressorPtr regressor;
  double discount;
  bool useSarsaRule;
  ScalarRandomVariableStatisticsPtr predictionError;

  double currentReward;
  SparseVectorPtr lastActionDescription;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_QLEARNING_H_

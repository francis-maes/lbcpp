/*-----------------------------------------.---------------------------------.
| Filename: QLearningPolicy.hpp            | QLearning/Sarsa Policy          |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 02:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_QLEARNING_H_
# define LBCPP_CORE_IMPL_POLICY_QLEARNING_H_

# include "PolicyStatic.hpp"
# include "../../RandomVariable.h"

namespace lbcpp {
namespace impl {

template<class DecoratedType>
struct QLearningPolicy
  : public EpisodicDecoratorPolicy<QLearningPolicy<DecoratedType> , DecoratedType>
{
  typedef EpisodicDecoratorPolicy<QLearningPolicy<DecoratedType>, DecoratedType> BaseClass;
  
  QLearningPolicy(const DecoratedType& decorated, RegressorPtr regressor, double discount, bool useSarsaRule = false)
    : BaseClass(decorated), regressor(regressor), discount(discount), useSarsaRule(useSarsaRule),
      predictionError(new ScalarRandomVariableStatistics("predictionError"))
    {}
  
  VariablePtr policyStart(ChoosePtr choose)
  {
    lastActionDescription = SparseVectorPtr();
    regressor->trainStochasticBegin(choose->getActionFeaturesFunction()->getDictionary());
    VariablePtr res = BaseClass::policyStart(choose);
    lastActionDescription = choose->computeActionFeatures(res)->toSparseVector();
    return res;
  }
  
  VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    assert(lastActionDescription);
    VariablePtr res = BaseClass::policyStep(reward, choose);
    
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
    double ycorrect = reward + discount * regressor->predict(nextActionDescription);
    double ypredicted = regressor->predict(lastActionDescription);
    predictionError->push(fabs(ycorrect - ypredicted));
    regressor->trainStochasticExample(new RegressionExample(lastActionDescription, ycorrect));
    
    lastActionDescription = nextActionDescription;
    return res;
  }
  
  void policyEnd(double reward)
  {
    BaseClass::policyEnd(reward);
    if (lastActionDescription)
      regressor->trainStochasticExample(new RegressionExample(lastActionDescription, reward));
    regressor->trainStochasticEnd();
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
  SparseVectorPtr lastActionDescription;
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_QLEARNING_H_

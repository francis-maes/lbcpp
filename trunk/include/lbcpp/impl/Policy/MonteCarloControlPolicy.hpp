/*-----------------------------------------.---------------------------------.
| Filename: MonteCarloControlPolicy.hpp    | Monte Carlo Control Policy      |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 02:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_MONTE_CARLO_CONTROL_H_
# define LBCPP_CORE_IMPL_POLICY_MONTE_CARLO_CONTROL_H_

# include "PolicyStatic.hpp"
# include "../../RandomVariable.h"

namespace lbcpp {
namespace impl {

template<class DecoratedType>
struct MonteCarloControlPolicy
  : public EpisodicDecoratorPolicy<MonteCarloControlPolicy<DecoratedType> , DecoratedType>
{
  typedef EpisodicDecoratorPolicy<MonteCarloControlPolicy<DecoratedType>, DecoratedType> BaseClass;
  
  MonteCarloControlPolicy(const DecoratedType& decorated, RegressorPtr regressor, double discount)
    : BaseClass(decorated), regressor(regressor), discount(discount),
      predictionError(new ScalarRandomVariableStatistics("predictionError"))
    {}
  
  VariablePtr policyStart(ChoosePtr choose)
  {
    actionFeatures.clear();
    rewards.clear();
    VariablePtr res = BaseClass::policyStart(choose);
    actionFeatures.push_back(choose->computeActionFeatures(res)->toSparseVector());
    return res;
  }
  
  VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    rewards.push_back(reward);
    VariablePtr res = BaseClass::policyStep(reward, choose);
    actionFeatures.push_back(choose->computeActionFeatures(res)->toSparseVector());
    return res;
  }
  
  void policyEnd(double reward)
  {
    rewards.push_back(reward);
    BaseClass::policyEnd(reward);
    assert(rewards.size() == actionFeatures.size());
    
    regressor->trainStochasticBegin(actionFeatures[0]->getDictionary());
    double R = 0.0;
    for (int i = rewards.size() - 1; i >= 0; --i)
    {
      R = R * discount + rewards[i];
      SparseVectorPtr features = actionFeatures[i];
      double prediction = regressor->predict(features);
      predictionError->push(fabs(prediction - R));
      regressor->trainStochasticExample(new RegressionExample(features, R));
    }
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
  ScalarRandomVariableStatisticsPtr predictionError;

  std::vector<SparseVectorPtr> actionFeatures;
  std::vector<double> rewards;
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_MONTE_CARLO_CONTROL_H_

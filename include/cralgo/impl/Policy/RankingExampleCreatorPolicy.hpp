/*-----------------------------------------.---------------------------------.
| Filename: RankingExampleCreatorPolicy.hpp| A policy that creates           |
| Author  : Francis Maes                   |   ranking examples              |
| Started : 13/03/2009 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_
# define CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_

# include "PolicyStatic.hpp"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct RankingExampleCreatorPolicy
  : public DecoratorPolicy<RankingExampleCreatorPolicy<DecoratedType> , DecoratedType>
{
  typedef RankingExampleCreatorPolicy<DecoratedType> ExactType;
  typedef DecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  RankingExampleCreatorPolicy(const DecoratedType& explorationPolicy, RankerPtr ranker, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : BaseClass(explorationPolicy), ranker(ranker), supervisor(supervisor), inclusionLevel(0) {}
    
  RankerPtr ranker;
  ActionValueFunctionPtr supervisor;
  size_t inclusionLevel;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (inclusionLevel == 0)
      ranker->trainStochasticBegin();
    BaseClass::policyEnter(crAlgorithm);
    ++inclusionLevel;
  }
    
  void policyLeave()
  {
    BaseClass::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0)
      ranker->trainStochasticEnd();
  }

  VariablePtr policyChoose(ChoosePtr choose)
  {
    FeatureGeneratorPtr alternatives = choose->computeActionFeatures(false);

    std::vector<double> costs;
    choose->computeActionValues(costs, supervisor);
    if (!costs.size())
      return VariablePtr();
    
    // transform action values into (regret) costs
    double bestValue = -DBL_MAX;
    for (size_t i = 0; i < costs.size(); ++i)
      if (costs[i] > bestValue)
        bestValue = costs[i];
    for (size_t i = 0; i < costs.size(); ++i)
      costs[i] = bestValue - costs[i];
    
    ranker->trainStochasticExample(RankingExample(alternatives, costs));
    return BaseClass::policyChoose(choose);
  }
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_

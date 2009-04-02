/*-----------------------------------------.---------------------------------.
| Filename: RankerValueFunction.hpp        | Ranker-based value functions    |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 19:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_VALUE_FUNCTION_RANKER_H_
# define LCPP_CORE_IMPL_VALUE_FUNCTION_RANKER_H_

# include "ChooseFunctionStatic.hpp"
# include "../../LearningMachine.h"

namespace lcpp {
namespace impl {

struct RankerStateValueFunction : public StateValueFunction<RankerStateValueFunction>
{
  typedef StateValueFunction<RankerStateValueFunction> BaseClass;
  
  RankerStateValueFunction(RankerPtr ranker)
    : ranker(ranker) {}
  
  RankerPtr ranker;
  FeatureGeneratorPtr stateFeatures;
  
  void setChoose(ChoosePtr choose)
    {stateFeatures = choose->computeStateFeatures();}

  double compute() const
    {return ranker->predictScore(stateFeatures);}
};

struct RankerActionValueFunction 
  : public ActionValueFunction<RankerActionValueFunction>
{
  typedef ActionValueFunction<RankerActionValueFunction> BaseClass;
  
  RankerActionValueFunction(RankerPtr ranker)
    : ranker(ranker) {}
  
  RankerPtr ranker;
  ActionFeaturesFunctionPtr actionFeatures;
  
  void setChoose(ChoosePtr choose)
    {actionFeatures = choose->getActionFeaturesFunction();}

  double computeDynamicType(VariablePtr variable) const
    {return ranker->predictScore(actionFeatures->compute(variable));}
};

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_VALUE_FUNCTION_RANKER_H_

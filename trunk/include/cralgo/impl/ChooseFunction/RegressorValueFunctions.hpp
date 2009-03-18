/*-----------------------------------------.---------------------------------.
| Filename: RegressorValueFunction.hpp     | Regressor-based value functions |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VALUE_FUNCTION_REGRESSOR_H_
# define CRALGO_IMPL_VALUE_FUNCTION_REGRESSOR_H_

# include "ChooseFunctionStatic.hpp"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

struct RegressorStateValueFunction : public StateValueFunction<RegressorStateValueFunction>
{
  typedef StateValueFunction<RegressorStateValueFunction> BaseClass;
  
  RegressorStateValueFunction(RegressorPtr regressor)
    : regressor(regressor) {}
  
  RegressorPtr regressor;
  FeatureGeneratorPtr stateFeatures;
  
  void setChoose(ChoosePtr choose)
    {stateFeatures = choose->computeStateFeatures();}

  double compute() const
    {return regressor->predict(stateFeatures);}
};

struct RegressorActionValueFunction 
  : public ActionValueFunction<RegressorActionValueFunction>
{
  typedef ActionValueFunction<RegressorActionValueFunction> BaseClass;
  
  RegressorActionValueFunction(RegressorPtr regressor)
    : regressor(regressor) {}
  
  RegressorPtr regressor;
  ActionFeaturesFunctionPtr actionFeatures;
  
  void setChoose(ChoosePtr choose)
    {actionFeatures = choose->getActionFeaturesFunction();}

  double computeDynamicType(VariablePtr variable) const
    {return regressor->predict(actionFeatures->compute(variable));}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_VALUE_FUNCTION_REGRESSOR_H_

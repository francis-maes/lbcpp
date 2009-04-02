/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunctionDynamicToSt...hpp| Choose functions                |
| Author  : Francis Maes                   |     dynamic -> static bridge    |
| Started : 13/03/2009 00:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_
# define LCPP_CORE_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_

# include "ChooseFunctionStatic.hpp"

namespace lcpp {
namespace impl {

struct DynamicToStaticActionValueFunction : public ActionValueFunction<DynamicToStaticActionValueFunction>
{
  DynamicToStaticActionValueFunction(ActionValueFunctionPtr function) 
    : function(function) {}
  
  ActionValueFunctionPtr function;

  void setChoose(ChoosePtr choose)
    {return function->setChoose(choose);}

  double computeDynamicType(VariablePtr choice) const
    {return function->compute(choice);}
};

inline DynamicToStaticActionValueFunction dynamicToStatic(ActionValueFunctionPtr function)
  {return DynamicToStaticActionValueFunction(function);}

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_

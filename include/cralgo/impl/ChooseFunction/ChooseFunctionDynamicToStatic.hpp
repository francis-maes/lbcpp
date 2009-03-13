/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunctionDynamicToSt...hpp| Choose functions                |
| Author  : Francis Maes                   |     dynamic -> static bridge    |
| Started : 13/03/2009 00:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_
# define CRALGO_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_

# include "ChooseFunctionStatic.hpp"

namespace cralgo {
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
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_

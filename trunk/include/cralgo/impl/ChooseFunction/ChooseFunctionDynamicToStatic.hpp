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

template<class ChoiceType>
struct DynamicToStaticActionValueFunction
  : public ActionValueFunction<DynamicToStaticActionValueFunction<ChoiceType>, ChoiceType>
{
  DynamicToStaticActionValueFunction(ActionValueFunctionPtr function) 
    : function(function) {}
  
  ActionValueFunctionPtr function;

  void setChoose(ChoosePtr choose)
    {return function->setChoose(choose);}

  double compute(const ChoiceType& choice) const
    {return function->compute(Variable::create(choice));}
};

template<class ChoiceType>
inline DynamicToStaticActionValueFunction<ChoiceType> dynamicToStatic(ActionValueFunctionPtr function)
  {return DynamicToStaticActionValueFunction<ChoiceType>(function);}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_CHOOSE_FUNCTION_DYNAMIC_TO_STATIC_H_

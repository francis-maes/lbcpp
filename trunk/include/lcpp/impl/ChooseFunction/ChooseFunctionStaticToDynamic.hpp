/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunctionStaticToDy....hpp| Choose Functions                |
| Author  : Francis Maes                   |     Static to Dynamic Bridge    |
| Started : 12/03/2009 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_
# define LCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_

# include "ChooseFunctionStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace lcpp {
namespace impl {

/*
** Choose Function
*/
STATIC_TO_DYNAMIC_ABSTRACT_CLASS(ChooseFunction_, Object)
  virtual void setChoose(ChoosePtr choose)
  {
    assert(choose);
    BaseClass::impl.setChoose(choose);
  }
};

STATIC_TO_DYNAMIC_CLASS(ChooseFunction, ChooseFunction_)
STATIC_TO_DYNAMIC_ENDCLASS(ChooseFunction);

/*
** Values
*/
STATIC_TO_DYNAMIC_CLASS(StateValueFunction, ChooseFunction_)
  virtual double compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_ENDCLASS(StateValueFunction);

STATIC_TO_DYNAMIC_CLASS(ActionValueFunction, ChooseFunction_)
  virtual double compute(VariablePtr choice) const
    {return BaseClass::impl.computeDynamicType(choice);}
STATIC_TO_DYNAMIC_ENDCLASS(ActionValueFunction);

/*
** Features
*/
STATIC_TO_DYNAMIC_CLASS(StateFeaturesFunction, ChooseFunction_)
  virtual FeatureGeneratorPtr compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_ENDCLASS(StateFeaturesFunction);

/*STATIC_TO_DYNAMIC_BEGIN_0_1(ActionFeaturesFunctionDirect, ChooseFunction_, lcpp::ActionFeaturesFunction) 
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice);}
};
template<class ExactType>
inline ActionFeaturesFunctionPtr staticToDynamic(const ActionFeaturesFunction<ExactType, VariablePtr>& impl)
  {return ActionFeaturesFunctionPtr(new StaticToDynamicActionFeaturesFunctionDirect<ExactType>(static_cast<const ExactType& >(impl)));}
*/

STATIC_TO_DYNAMIC_CLASS(ActionFeaturesFunction, ChooseFunction_)
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const
    {assert(choice); return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_ENDCLASS_1(ActionFeaturesFunction);


/*
** String Descriptions
*/
STATIC_TO_DYNAMIC_CLASS(StateDescriptionFunction, ChooseFunction_)
  virtual std::string compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_ENDCLASS(StateDescriptionFunction);

STATIC_TO_DYNAMIC_CLASS(ActionDescriptionFunction, ChooseFunction_)
  virtual std::string compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_ENDCLASS_1(ActionDescriptionFunction);

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_

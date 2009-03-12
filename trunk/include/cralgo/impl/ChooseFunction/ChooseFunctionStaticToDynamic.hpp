/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunctionStaticToDy....hpp| Choose Functions                |
| Author  : Francis Maes                   |     Static to Dynamic Bridge    |
| Started : 12/03/2009 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_
# define CRALGO_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_

# include "ChooseFunctionStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace cralgo {
namespace impl {

/*
** Choose Function
*/
STATIC_TO_DYNAMIC_BEGIN_1_1(ChooseFunction_, DynamicClass, Object, DynamicClass)
  virtual void setChoose(ChoosePtr choose)
  {
    assert(choose);
    BaseClass::impl.setChoose(choose);
  }
};

STATIC_TO_DYNAMIC_BEGIN_0_1(ChooseFunction, ChooseFunction_, cralgo::ChooseFunction)
STATIC_TO_DYNAMIC_END_0(ChooseFunction);

/*
** Values
*/
STATIC_TO_DYNAMIC_BEGIN_0_1(StateValueFunction, ChooseFunction_, cralgo::StateValueFunction)
  virtual double compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_END_0(StateValueFunction);

STATIC_TO_DYNAMIC_BEGIN_0_1(ActionValueFunction, ChooseFunction_, cralgo::ActionValueFunction)
  virtual double compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_END_1(ActionValueFunction);

/*
** Features
*/
STATIC_TO_DYNAMIC_BEGIN_0_1(StateFeaturesFunction, ChooseFunction_, cralgo::StateFeaturesFunction)
  virtual FeatureGeneratorPtr compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_END_0(StateFeaturesFunction);

STATIC_TO_DYNAMIC_BEGIN_0_1(ActionFeaturesFunction, ChooseFunction_, cralgo::ActionFeaturesFunction)
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_END_1(ActionFeaturesFunction);

/*
** String Descriptions
*/
STATIC_TO_DYNAMIC_BEGIN_0_1(StateDescriptionFunction, ChooseFunction_, cralgo::StateDescriptionFunction)
  virtual std::string compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_END_0(StateDescriptionFunction);

STATIC_TO_DYNAMIC_BEGIN_0_1(ActionDescriptionFunction, ChooseFunction_, cralgo::ActionDescriptionFunction)
  virtual std::string compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_END_1(ActionDescriptionFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_CHOOSE_FUNCTION_STATIC_TO_DYNAMIC_H_

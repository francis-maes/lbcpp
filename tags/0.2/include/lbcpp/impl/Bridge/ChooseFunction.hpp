/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunction.hpp             | Static Choose Functions         |
| Author  : Francis Maes                   |            Interface            |
| Started : 12/03/2009 16:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_H_
# define LBCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_H_

# include "../../CRAlgorithm/ChooseFunction.h"
# include "../../CRAlgorithm/Variable.h"
# include "../../CRAlgorithm/Choose.h"

namespace lbcpp {
namespace impl {

template<class ExactType>
struct ChooseFunction : public Object<ExactType>
{
  void setChoose(ChoosePtr choose)
    {jassert(false);}
};

/*
** Values
*/
template<class ExactType>
struct StateValueFunction : public ChooseFunction<ExactType>
{
  double compute() const
    {jassert(false); return 0.0;}
};

template<class ExactType>
struct ActionValueFunction : public ChooseFunction<ExactType>
{
  template<class T>
  double compute(const T& choice) const
    {return ((const ExactType* )this)->computeDynamicType(Variable::create(choice));}

  double computeDynamicType(lbcpp::VariablePtr variable) const
    {jassert(false); return 0.0;}
};

template<class ExactType, class ChoiceType_>
struct TypedActionValueFunction : public ActionValueFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;

  double compute(const ChoiceType& choice) const
    {jassert(false); return 0.0;}

  double computeDynamicType(lbcpp::VariablePtr variable) const
    {return ((const ExactType* )this)->compute(variable->getConstReference<ChoiceType>());}
};

/*
** Features
*/
template<class ExactType>
struct StateFeaturesFunction : public ChooseFunction<ExactType>
{
  FeatureGeneratorPtr compute() const
    {jassert(false); return FeatureGeneratorPtr();}
};

template<class ExactType, class ChoiceType_>
struct ActionFeaturesFunction : public ChooseFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;

  FeatureGeneratorPtr compute(const ChoiceType& choice) const
    {jassert(false); return FeatureGeneratorPtr();}
};

/*
** String Descriptions
*/
template<class ExactType>
struct StateDescriptionFunction : public ChooseFunction<ExactType>
{
  String compute() const
    {jassert(false); return "";}
};

template<class ExactType, class ChoiceType_>
struct ActionDescriptionFunction : public ChooseFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;

  String compute(const ChoiceType& choice) const
    {jassert(false); return "";}
};


/*
** Choose Function
*/
STATIC_TO_DYNAMIC_ABSTRACT_CLASS(ChooseFunction_, Object)
  virtual void setChoose(ChoosePtr choose)
  {
    jassert(choose);
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


STATIC_TO_DYNAMIC_CLASS(ActionFeaturesFunction, ChooseFunction_)
  virtual FeatureDictionaryPtr getDictionary() const
    {typedef typename ImplementationType::FeatureGenerator FG; return FG::getDictionary();}

  virtual FeatureGeneratorPtr compute(VariablePtr choice) const
    {jassert(choice); return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_ENDCLASS_1(ActionFeaturesFunction);


/*
** String Descriptions
*/
STATIC_TO_DYNAMIC_CLASS(StateDescriptionFunction, ChooseFunction_)
  virtual String compute() const
    {return BaseClass::impl.compute();}
STATIC_TO_DYNAMIC_ENDCLASS(StateDescriptionFunction);

STATIC_TO_DYNAMIC_CLASS(ActionDescriptionFunction, ChooseFunction_)
  virtual String compute(VariablePtr choice) const
    {return BaseClass::impl.compute(choice->getConstReference<typename ImplementationType::ChoiceType>());}
STATIC_TO_DYNAMIC_ENDCLASS_1(ActionDescriptionFunction);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_CHOOSE_FUNCTION_STATIC_H_

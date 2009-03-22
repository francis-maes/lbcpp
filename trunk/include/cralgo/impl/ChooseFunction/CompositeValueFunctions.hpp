/*-----------------------------------------.---------------------------------.
| Filename: CompositeValueFunctions.hpp    | Composite value functions       |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VALUE_FUNCTION_COMPOSITE_H_
# define CRALGO_IMPL_VALUE_FUNCTION_COMPOSITE_H_

# include "ChooseFunctionStatic.hpp"
# include "../../EditableFeatureGenerator.h"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

/*
** Composite value functions
*/
struct CompositeStateValueFunction : public StateValueFunction<CompositeStateValueFunction>
{
  void add(StateValueFunctionPtr stateValue)
  {
    if (this->stateValue)
      std::cerr << "Warning: only the first state value will be used in choose." << std::endl;
    else
      this->stateValue = stateValue;
  }

  void setChoose(ChoosePtr choose)
  {
    if (stateValue) 
      stateValue->setChoose(choose);
  }

  double compute() const
    {return stateValue ? stateValue->compute() : 0.0;}

  std::string getName() const
    {return stateValue ? stateValue->getName() : "emptyStateValue";}
      
private:
  StateValueFunctionPtr stateValue;  
};

struct CompositeActionValueFunction : public ActionValueFunction<CompositeActionValueFunction>
{
  void add(ActionValueFunctionPtr actionValue)
  {
    if (this->actionValue)
      std::cerr << "Warning: only the first action value will be used in choose." << std::endl;
    else
      this->actionValue = actionValue;
  }

  void setChoose(ChoosePtr choose)
  {
    if (actionValue) 
      actionValue->setChoose(choose);
  }

  double computeDynamicType(VariablePtr choice) const
    {return actionValue ? actionValue->compute(choice) : 0.0;}

  std::string getName() const
    {return actionValue ? actionValue->getName() : "emptyActionValue";}
      
private:
  ActionValueFunctionPtr actionValue;    
};

template<class ExactType, class BaseClass, class DynamicType>
struct CompositeChooseFunction : public BaseClass
{
  typedef ReferenceCountedObjectPtr<DynamicType> DynamicTypePtr;
  std::vector<DynamicTypePtr> functions;

  void add(DynamicTypePtr function)
    {functions.push_back(function);}
    
  void setChoose(ChoosePtr choose)
  {
    for (size_t i = 0; i < functions.size(); ++i)
      functions[i]->setChoose(choose);
  }
  
  std::string getName() const
  {
    std::string res;
    for (size_t i = 0; i < functions.size(); ++i)
    {
      res += functions[i]->getName();
      if (i < functions.size() - 1)
        res += "\n";
    }
    return res.size() ? res : "empty";
  }
};

/*
** Features
*/
TEMPLATE_INHERIT_BEGIN_3(CompositeStateFeaturesFunction, CompositeChooseFunction, 
  CompositeStateFeaturesFunction, StateFeaturesFunction<CompositeStateFeaturesFunction>, cralgo::StateFeaturesFunction)

  FeatureGeneratorPtr compute() const
  {
    size_t n = BaseClass::functions.size();
    std::vector<std::pair<FeatureGeneratorPtr, double> >* res = new std::vector<std::pair<FeatureGeneratorPtr, double> >(n);
    for (size_t i = 0; i < n; ++i)
      (*res)[i] = std::make_pair(BaseClass::functions[i]->compute(), 1.0);
    return FeatureGenerator::linearCombination(res);
  }
};

template<class ChoiceType>
struct CompositeActionFeaturesFunction : public CompositeChooseFunction<
  CompositeActionFeaturesFunction<ChoiceType>,
  ActionFeaturesFunction< CompositeActionFeaturesFunction<ChoiceType>, ChoiceType >,
  cralgo::ActionFeaturesFunction >
{
  typedef CompositeChooseFunction<
    CompositeActionFeaturesFunction<ChoiceType>,
    ActionFeaturesFunction< CompositeActionFeaturesFunction<ChoiceType>, ChoiceType >,
    cralgo::ActionFeaturesFunction > BaseClass;

  FeatureGeneratorPtr compute(const ChoiceType& choice) const
  {
    size_t n = BaseClass::functions.size();
    std::vector<std::pair<FeatureGeneratorPtr, double> >* res = new std::vector<std::pair<FeatureGeneratorPtr, double> >(n);
    for (size_t i = 0; i < n; ++i)
      (*res)[i] = std::make_pair(BaseClass::functions[i]->compute(choice), 1.0);
    return FeatureGenerator::linearCombination(res);
  }
};

/*
** String Descriptions
*/
TEMPLATE_INHERIT_BEGIN_3(CompositeStateDescriptionFunction, CompositeChooseFunction, 
  CompositeStateDescriptionFunction, StateDescriptionFunction<CompositeStateDescriptionFunction>, cralgo::StateDescriptionFunction)

  std::string compute() const
  {
    std::string res;
    for (size_t i = 0; i < BaseClass::functions.size(); ++i)
    {
      res += BaseClass::functions[i]->compute();
      if (i < BaseClass::functions.size() - 1)
        res += "\n";
    }
    return res;
  }
};

template<class ChoiceType>
struct CompositeActionDescriptionFunction : public CompositeChooseFunction<
  CompositeActionDescriptionFunction<ChoiceType>,
  ActionDescriptionFunction< CompositeActionDescriptionFunction<ChoiceType>, ChoiceType >,
  cralgo::ActionDescriptionFunction >
{
  typedef CompositeChooseFunction<
      CompositeActionDescriptionFunction<ChoiceType>,
      ActionDescriptionFunction< CompositeActionDescriptionFunction<ChoiceType>, ChoiceType >,
      cralgo::ActionDescriptionFunction > BaseClass;

  std::string compute(const ChoiceType& choice) const
  {
    std::string res;
    for (size_t i = 0; i < BaseClass::functions.size(); ++i)
    {
      res += BaseClass::functions[i]->compute(choice);
      if (i < BaseClass::functions.size() - 1)
        res += "\n";
    }
    return res;
  }
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_VALUE_FUNCTION_COMPOSITE_H_

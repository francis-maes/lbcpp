/*-----------------------------------------.---------------------------------.
| Filename: Choose.hpp                     | Static to Dynamic Choose        |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 04/02/2009 19:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_CHOOSE_HPP_
# define CRALGO_STATIC_CHOOSE_HPP_

# include "../../Choose.h"
# include "../ChooseFunction/ChooseFunctionDynamicToStatic.hpp"
# include "Variable.hpp"
# include "FeatureGenerator.hpp"

namespace cralgo
{

template<class ChooseType>
class StaticToDynamicChoose : public Choose
{
public:
  typedef typename ChooseType::ContainerType ContainerType;
  typedef typename ChooseType::ChoiceType ChoiceType;
  typedef Traits<ContainerType> ContainerTraits;
  
  StaticToDynamicChoose(const ContainerType& container, CRAlgorithmPtr crAlgorithm = CRAlgorithmPtr())
    : Choose(crAlgorithm), choose(ChooseType::getInstance()), container(container) {}
  
  const ChooseType& choose;
  const ContainerType& container;
  
  /*
  ** Choices
  */
  virtual std::string getChoiceType() const
    {return ChooseType::getChoiceType();}

  virtual size_t getNumChoices() const
    {return ContainerTraits::size(container);}

  virtual VariableIteratorPtr newIterator() const
    {return VariableIteratorPtr(new StaticToDynamicVariableIterator<ContainerType>(container));}
  
  virtual VariablePtr sampleRandomChoice() const
  {
    return ContainerTraits::size(container)
      ? Variable::create(ContainerTraits::sampleRandom(container))
      : VariablePtr();
  }
  
  virtual VariablePtr sampleBestChoice(ActionValueFunctionPtr valueFunction) const
  {
    assert(valueFunction);
    if (!ContainerTraits::size(container))
      return VariablePtr();
    valueFunction->setChoose(getReferenceCountedPointer());
    typename ContainerTraits::ConstIterator best = 
      ContainerTraits::sampleBests(container, impl::dynamicToStatic<ChoiceType>(valueFunction)); 
    return Variable::create(ContainerTraits::value(best));
  }
  
  /*
  ** Composites functions
  */
  virtual StateDescriptionFunctionPtr getStateDescriptionFunction() const
  {
    StateDescriptionFunctionPtr res = choose.getStateDescriptionFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }
  
  virtual ActionDescriptionFunctionPtr getActionDescriptionFunction() const
  {
    ActionDescriptionFunctionPtr res = choose.getActionDescriptionFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }
  
  virtual StateValueFunctionPtr getStateValueFunction() const
  {
    StateValueFunctionPtr res = choose.getStateValueFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }
  
  virtual ActionValueFunctionPtr getActionValueFunction() const
  {
    ActionValueFunctionPtr res = choose.getActionValueFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }

  virtual StateFeaturesFunctionPtr getStateFeaturesFunction() const
  {
    StateFeaturesFunctionPtr res = choose.getStateFeaturesFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }

  virtual ActionFeaturesFunctionPtr getActionFeaturesFunction() const
  {
    ActionFeaturesFunctionPtr res = choose.getActionFeaturesFunction();
    res->setChoose(getReferenceCountedPointer());
    return res;
  }
  
  /*
  ** Functions
  */
  virtual size_t getNumStateDescriptions() const
    {return ChooseType::numStateDescriptionFunctions;}
    
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const
    {return choose.getStateDescription(index);}
    
  virtual size_t getNumActionDescriptions() const
    {return ChooseType::numActionDescriptionFunctions;}
    
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const
    {return choose.getActionDescription(index);}
    
  virtual size_t getNumStateValues() const
    {return ChooseType::numStateValueFunctions;}
    
  virtual StateValueFunctionPtr getStateValue(size_t index) const
    {return choose.getStateValue(index);}

  virtual size_t getNumActionValues() const
    {return ChooseType::numActionValueFunctions;}
    
  virtual ActionValueFunctionPtr getActionValue(size_t index) const
    {return choose.getActionValue(index);}
    
  virtual size_t getNumStateFeatures() const
    {return ChooseType::numStateFeaturesFunctions;}
    
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const
    {return choose.getStateFeatures(index);}
    
  virtual size_t getNumActionFeatures() const
    {return ChooseType::numActionFeaturesFunctions;}
    
  virtual ActionFeaturesFunctionPtr getActionFeatures(size_t index) const
    {return choose.getActionFeatures(index);}
};


}; /* namespace cralgo */

#endif // !CRALGO_STATIC_CHOOSE_HPP_

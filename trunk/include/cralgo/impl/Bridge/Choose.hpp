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
# include <sstream>
# include "FeatureGenerator.hpp"
# include "StateFunction.hpp"

namespace cralgo
{

template<class ContainerType>
class WrapperActionIterator : public ActionIterator
{
public:
  typedef Traits<ContainerType> ContainerTraits;
  typedef typename ContainerTraits::ConstIterator iterator;
    
  WrapperActionIterator(const ContainerType& container)
    : container(container), it(ContainerTraits::begin(container)) {}
  
  virtual void next()
  {
    if (it != ContainerTraits::end(container))
      ++it;
  }
  
  virtual bool exists() const
    {return it != ContainerTraits::end(container);}
  
  virtual const void* get() const
    {return it == ContainerTraits::end(container) ? NULL : &ContainerTraits::value(it);}

private:
  const ContainerType& container;
  iterator it;
};

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

  virtual ActionIteratorPtr newIterator() const
    {return ActionIteratorPtr(new WrapperActionIterator<ContainerType>(container));}
  
  virtual void* cloneChoice(const void* choice) const
    {return new ChoiceType(*(const ChoiceType* )choice);}
  
  virtual void deleteChoice(const void* choice) const
    {delete (ChoiceType* )choice;}

  virtual const void* sampleRandomChoice() const
  {
    return ContainerTraits::size(container)
      ? cloneChoice(&ContainerTraits::sampleRandom(container))
      : NULL;
  }
  
  virtual const void* sampleBestChoice(ActionValueFunctionPtr valueFunction) const
  {
    if (!ContainerTraits::size(container))
      return NULL;
    return cloneChoice(&ContainerTraits::sampleBests(container, crAlgorithm, valueFunction));
  }
  
  /*
  ** Composites functions
  */
  virtual StateDescriptionFunctionPtr getStateDescriptionFunction() const
    {return choose.getStateDescriptionFunction();}
  virtual ActionDescriptionFunctionPtr getActionDescriptionFunction() const
    {return choose.getActionDescriptionFunction();}  
  virtual StateValueFunctionPtr getStateValueFunction() const
    {return choose.getStateValueFunction();}  
  virtual ActionValueFunctionPtr getActionValueFunction() const
    {return choose.getActionValueFunction();}  
  virtual StateFeaturesFunctionPtr getStateFeaturesFunction() const
    {return choose.getStateFeaturesFunction();}  
  virtual ActionFeaturesFunctionPtr getActionFeaturesFunction() const
    {return choose.getActionFeaturesFunction();}  
  
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

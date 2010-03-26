/*-----------------------------------------.---------------------------------.
| Filename: Choose.hpp                     | Static to Dynamic Choose        |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 04/02/2009 19:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_CHOOSE_HPP_
# define LBCPP_STATIC_CHOOSE_HPP_

# include "../../Choose.h"
# include "../../CRAlgorithm.h"
# include "Variable.hpp"
# include "FeatureGenerator.hpp"
# include "ChooseFunction.hpp"
# include "CompositeChooseFunctions.hpp"

namespace lbcpp
{

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

template<class ChooseType>
class StaticToDynamicChoose : public Choose
{
public:
  typedef typename ChooseType::ContainerType ContainerType;
  typedef typename ChooseType::ChoiceType ChoiceType;
  typedef Traits<ContainerType> ContainerTraits;
  
  StaticToDynamicChoose(const ContainerType& container, CRAlgorithmPtr crAlgorithm)
    : Choose(crAlgorithm), choose(ChooseType::getInstance()), container(container) {}
  
  const ChooseType& choose;
  const ContainerType& container;
  
  enum {chooseNumber = ChooseType::chooseNumber};

  /*
  ** Object
  */
  virtual String getName() const
    {return crAlgorithm->getName() + "::choose" + lbcpp::toString((size_t)chooseNumber);}

  virtual ObjectPtr clone() const
    {return new StaticToDynamicChoose(container, crAlgorithm->cloneAndCast<CRAlgorithm>());}

  /*
  ** Choices
  */
  virtual String getChoiceType() const
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
    jassert(valueFunction);
    if (!ContainerTraits::size(container))
      return VariablePtr();
    valueFunction->setChoose(getReferenceCountedPointer());
    typename ContainerTraits::ConstIterator best = 
      ContainerTraits::sampleBests(container, impl::dynamicToStatic(valueFunction)); 
    return Variable::create(ContainerTraits::value(best));
  }

  virtual VariablePtr sampleChoiceWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0) const
  {
    jassert(probabilities.size() == ContainerTraits::size(container));
    if (!probabilitiesSum)
      for (size_t i = 0; i < probabilities.size(); ++i)
        probabilitiesSum += probabilities[i];
    double number = RandomGenerator::getInstance().sampleDouble(probabilitiesSum);
    typename ContainerTraits::ConstIterator it = ContainerTraits::begin(container);
    for (size_t i = 0; i < probabilities.size(); ++i, ++it)
    {
      jassert(it != ContainerTraits::end(container));
      double prob = probabilities[i];
      if (number <= prob)
        return Variable::create(ContainerTraits::value(it));
      number -= prob;
    }
    jassert(false);
    return VariablePtr();
  }

  virtual void computeActionValues(std::vector<double>& res, ActionValueFunctionPtr valueFunction) const
  {
    if (!valueFunction)
    {
      valueFunction = choose.getActionValueFunction();
      if (!valueFunction)
      {
        res.clear();
        Object::error("Choose::computeActionValues", "No action values in this choose");
        return;
      }
    }
    valueFunction->setChoose(getReferenceCountedPointer());
    StaticToDynamicVariable< ChoiceType > v;
    StaticallyAllocatedReferenceCountedObjectPtr<Variable> variable(v);
    
    res.clear();
    res.reserve(ContainerTraits::size(container));
    typename ContainerTraits::ConstIterator it = ContainerTraits::begin(container);
    for (; it != ContainerTraits::end(container); ++it)
    {
      variable->getUntypedPointer() = const_cast<void* >((const void* )&ContainerTraits::value(it));
      res.push_back(valueFunction->compute(variable));
    }
    variable->getUntypedPointer() = NULL;
  }

  FeatureDictionaryPtr getActionsFeatureDictionary() const
  {
    if (!choose.actionsFeatureDictionary)
      const_cast<ChooseType& >(choose).actionsFeatureDictionary = new FeatureDictionary(getName());
    return choose.actionsFeatureDictionary;
  }

  virtual FeatureGeneratorPtr computeActionsFeatures(bool transformIntoSparseVectors) const
  {
    ActionFeaturesFunctionPtr f = choose.getActionFeaturesFunction();
    jassert(f); // todo: error message
    f->setChoose(getReferenceCountedPointer());
    size_t numChoices = ContainerTraits::size(container);

    // get dictionary and complete sub-dictionaries
    FeatureDictionaryPtr featureDictionary = getActionsFeatureDictionary(); 
    for (size_t i = featureDictionary->getNumScopes(); i < numChoices; ++i)
      featureDictionary->addScope("choice " + lbcpp::toString(i), f->getDictionary());
    
    StaticToDynamicVariable< ChoiceType > v;
    StaticallyAllocatedReferenceCountedObjectPtr<Variable> variable(v);
    
    if (transformIntoSparseVectors)
    {
      SparseVectorPtr res = new SparseVector(featureDictionary, 0, numChoices);
      size_t i = 0;
      typename ContainerTraits::ConstIterator it = ContainerTraits::begin(container);
      for (; it != ContainerTraits::end(container); ++it, ++i)
      {
        variable->getUntypedPointer() = const_cast<void* >((const void* )&ContainerTraits::value(it));
        res->setSubVector(i, f->compute(variable)->toSparseVector());
      }
      variable->getUntypedPointer() = NULL;
      return res;
    }
    else
    {
      CompositeFeatureGeneratorPtr res = new CompositeFeatureGenerator(featureDictionary, numChoices);
      size_t i = 0;
      typename ContainerTraits::ConstIterator it = ContainerTraits::begin(container);
      for (; it != ContainerTraits::end(container); ++it, ++i)
      {
        variable->getUntypedPointer() = const_cast<void* >((const void* )&ContainerTraits::value(it));
        res->setSubGenerator(i, f->compute(variable));
      }
      variable->getUntypedPointer() = NULL;
      return res;
    }
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
    // TODO: check result
    res->setChoose(getReferenceCountedPointer());
    return res;
  }

  virtual ActionFeaturesFunctionPtr getActionFeaturesFunction() const
  {
    ActionFeaturesFunctionPtr res = choose.getActionFeaturesFunction();
    // TODO: check result
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


}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_CHOOSE_HPP_

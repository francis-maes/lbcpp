/*-----------------------------------------.---------------------------------.
| Filename: SelectAndMakeProductsPerception.h| A decorator to make lots of   |
| Author  : Francis Maes                   | conjunction features            |
| Started : 05/10/2010 21:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_
# define LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class SelectAndMakeProductsPerception : public VariableVectorPerception
{
public:
  SelectAndMakeProductsPerception(PerceptionPtr decorated, FunctionPtr multiplyFunction, ContainerPtr selectedConjunctions)
    : decorated(decorated), multiplyFunction(multiplyFunction), selectedConjunctions(selectedConjunctions)
    {createSubPerceptions();}

  SelectAndMakeProductsPerception() {}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}
  
  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName() + T(" selected");}

  typedef std::vector< std::pair<PerceptionPtr, Variable> > PerceptionVariableVector;

  struct Callback : public PerceptionCallback
  {
    Callback(PerceptionVariableVector& variables)
      : variables(variables) {}

    PerceptionVariableVector& variables;

    virtual void sense(size_t variableNumber, const Variable& value)
      {variables[variableNumber].second = value;}

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
      {variables[variableNumber].first = subPerception; variables[variableNumber].second = input;}
  };

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    // surface compute decorated perception
    PerceptionVariableVector variables(decorated->getNumOutputVariables());
    Callback callback(variables);
    callback.setStaticAllocationFlag();
    decorated->computePerception(input, &callback);
  
    // sense 
    size_t n = getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      PerceptionPtr subPerception = getOutputVariableSubPerception(i);
      ContainerPtr conjunction = selectedConjunctions->getElement(i).getObjectAndCast<Container>();
      size_t arity = conjunction->getNumElements();
      if (arity == 1)
      {
        std::pair<PerceptionPtr, Variable> v = variables[conjunction->getElement(0).getInteger()];
        if (v.second)
        {
          jassert(subPerception == v.first);
          if (v.first)
            targetCallback->sense(i, v.first, v.second);
          else
            targetCallback->sense(i, v.second);
        }
      }
      else if (arity == 2)
      {
        std::pair<PerceptionPtr, Variable> v1 = variables[conjunction->getElement(0).getInteger()];
        std::pair<PerceptionPtr, Variable> v2 = variables[conjunction->getElement(1).getInteger()];
        if (v1.second && v2.second)
        {
          jassert(v1.first && v2.first);
          targetCallback->sense(i, subPerception, Variable::pair(v1.second, v2.second));
        }
      }
      else
        jassert(false); // not supported yet
    }
  }

protected:
  friend class SelectAndMakeProductsPerceptionClass;

  PerceptionPtr decorated;
  FunctionPtr multiplyFunction;
  ContainerPtr selectedConjunctions; // outputNumber -> numberInConjunction -> variableNumber

  void createSubPerceptions()
  {
    size_t n = selectedConjunctions->getNumElements();
    outputVariables.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr conjunction = selectedConjunctions->getElement(i).getObjectAndCast<Container>();
      jassert(conjunction);
      createSubPerception(conjunction);
    }
  }

  void createSubPerception(ContainerPtr conjunction)
  {
    size_t arity = conjunction->getNumElements();
    jassert(arity);
    if (arity == 1)
    {
      int variableNumber = conjunction->getElement(0).getInteger();
      jassert(variableNumber >= 0 && variableNumber < (int)decorated->getNumOutputVariables());
      String name = decorated->getOutputVariableName(variableNumber);
      
      PerceptionPtr subPerception = decorated->getOutputVariableSubPerception(variableNumber);
      addOutputVariable(decorated->getOutputVariableType(variableNumber), name, subPerception);
    }

    else if (arity == 2)
    {
      int index1 = conjunction->getElement(0).getInteger();
      int index2 = conjunction->getElement(1).getInteger();
      jassert(index1 >= 0 && index1 <= (int)decorated->getNumOutputVariables());
      jassert(index2 >= 0 && index2 <= (int)decorated->getNumOutputVariables());
      String name = decorated->getOutputVariableName(index1) + T("&&") + decorated->getOutputVariableName(index2);
      PerceptionPtr subPerception1 = decorated->getOutputVariableSubPerception(index1);
      PerceptionPtr subPerception2 = decorated->getOutputVariableSubPerception(index2);
      jassert(subPerception1 && subPerception2);
      PerceptionPtr subPerception = productPerception(multiplyFunction, subPerception1, subPerception2, true, false);
      addOutputVariable(subPerception->getOutputType(), name, subPerception);
    }
    else
    {
       // not supported yet
      jassert(false);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_

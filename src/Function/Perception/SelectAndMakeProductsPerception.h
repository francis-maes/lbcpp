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

class SelectAndMakeProductsPerception : public CompositePerception
{
public:
  SelectAndMakeProductsPerception(TypePtr inputType, FunctionPtr multiplyFunction, ContainerPtr selectedConjunctions)
    : CompositePerception(inputType, inputType->getClassName() + T(" selected")),
      multiplyFunction(multiplyFunction), selectedConjunctions(selectedConjunctions)
    {createSubPerceptions();}

  SelectAndMakeProductsPerception() {}

protected:
  friend class SelectAndMakeProductsPerceptionClass;

  FunctionPtr multiplyFunction;
  ContainerPtr selectedConjunctions; // outputNumber -> numberInConjunction -> variableNumber

  void createSubPerceptions()
  {
    std::vector<PerceptionPtr> inputPerceptions(inputType->getObjectNumVariables());
    for (size_t i = 0; i < inputPerceptions.size(); ++i)
      inputPerceptions[i] = Perception::compose(selectVariableFunction(i), identityPerception(inputType->getObjectVariableType(i)));

    size_t n = selectedConjunctions->getNumElements();
    subPerceptions->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr conjunction = selectedConjunctions->getElement(i).getObjectAndCast<Container>();
      String name;
      PerceptionPtr subPerception = computeSubPerception(conjunction, name, inputPerceptions);
      addPerception(name, subPerception);
    }
  }

  PerceptionPtr computeSubPerception(ContainerPtr conjunction, String& name, const std::vector<PerceptionPtr>& inputPerceptions) const
  {
    size_t arity = conjunction->getNumElements();
    jassert(arity);
    if (arity == 1)
    {
      int variableNumber = conjunction->getElement(0).getInteger();
      jassert(variableNumber >= 0 && variableNumber < (int)inputPerceptions.size());
      name = inputType->getObjectVariableName(variableNumber);
      return inputPerceptions[variableNumber];
    }
    else if (arity == 2)
    {
      int index1 = conjunction->getElement(0).getInteger();
      int index2 = conjunction->getElement(1).getInteger();
      jassert(index1 >= 0 && index1 <= (int)inputPerceptions.size());
      jassert(index2 >= 0 && index2 <= (int)inputPerceptions.size());
      name = inputType->getObjectVariableName(index1) + T("&&") + inputType->getObjectVariableName(index2);
      return productPerception(multiplyFunction, inputPerceptions[index1], inputPerceptions[index2], true, true);
    }
    else
    {
       // not supported yet
      jassert(false);
      return PerceptionPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_SELECT_AND_MAKE_PRODUCTS_H_

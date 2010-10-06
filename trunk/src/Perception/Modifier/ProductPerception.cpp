/*-----------------------------------------.---------------------------------.
| Filename: ProductPerception.cpp          | Product of two Perceptions      |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2010 19:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProductPerception.h"
using namespace lbcpp;

/*
** ProductPerception
*/
struct PerceptionVariable
{
  size_t number;
  PerceptionPtr subPerception;
  Variable input;
};

struct ComputePerceptionProductCallback : public PerceptionCallback
{
  ComputePerceptionProductCallback(const ProductPerception* owner, std::list<PerceptionVariable>& variables, PerceptionCallbackPtr targetCallback)
    : owner(owner), variables(variables), targetCallback(targetCallback) {}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    for (std::list<PerceptionVariable>::const_iterator it = variables.begin(); it != variables.end(); ++it)
      forwardSense(variableNumber, value, it->number, it->input);
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
  {
    for (std::list<PerceptionVariable>::const_iterator it = variables.begin(); it != variables.end(); ++it)
      forwardSense(variableNumber, input, it->number, it->input);
  }

  const ProductPerception* owner;
  std::list<PerceptionVariable>& variables;
  PerceptionCallbackPtr targetCallback;

private:
  void forwardSense(size_t number1, const Variable& variable1,
                    size_t number2, const Variable& variable2)
  {
    size_t n = owner->getPerception2()->getNumOutputVariables();
    jassert(number2 < n);
    size_t index = number1 * n + number2;
    jassert(index < owner->getNumOutputVariables());

    Variable p = Variable::pair(variable1, variable2);
    PerceptionPtr subPerception = owner->getOutputVariableSubPerception(index);
    if (subPerception)
      targetCallback->sense(index, subPerception, p);
    else
      targetCallback->sense(index, owner->getMultiplyFunction()->compute(p));
  }
};

struct FillVariableListCallback : public PerceptionCallback
{
  FillVariableListCallback(std::list<PerceptionVariable>& variables)
    : variables(variables) {}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    PerceptionVariable v;
    v.number = variableNumber;
    v.input = value;
    variables.push_back(v);
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
  {
    PerceptionVariable v;
    v.number = variableNumber;
    v.subPerception = subPerception;
    v.input = input;
    variables.push_back(v);
  }

  std::list<PerceptionVariable>& variables;
};

ProductPerception::ProductPerception(FunctionPtr multiplyFunction,
                                     PerceptionPtr perception1, PerceptionPtr perception2,
                                     bool symmetricFunction, bool singleInputForBothPerceptions)
  : multiplyFunction(multiplyFunction), symmetricFunction(symmetricFunction), 
    perception1(perception1), perception2(perception2), singleInputForBothPerceptions(singleInputForBothPerceptions)
{
  jassert(perception1 && perception2);
  computeOutputVariables();
}

TypePtr ProductPerception::getInputType() const
{
  if (singleInputForBothPerceptions)
    return Type::findCommonBaseType(perception1->getInputType(), perception2->getInputType());
  else
    return pairClass(perception1->getInputType(), perception2->getInputType());
}

void ProductPerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  // retrieve inputs
  Variable input1, input2;
  if (singleInputForBothPerceptions)
    input1 = input2 = input;
  else
    input1 = input[0], input2 = input[1];

  // compute Perception2
  std::list<PerceptionVariable> variables;
  FillVariableListCallback fillVariableListCallback(variables);
  fillVariableListCallback.setStaticAllocationFlag();
  perception2->computePerception(input2, &fillVariableListCallback);

  // iterate over Perception1
  ComputePerceptionProductCallback computePerceptionProductCallback(this, variables, callback);
  computePerceptionProductCallback.setStaticAllocationFlag();
  perception1->computePerception(input1, &computePerceptionProductCallback);
}

void ProductPerception::addOutputVariable(const String& name, TypePtr type1, PerceptionPtr sub1, TypePtr type2, PerceptionPtr sub2)
{
  if (!sub1 && !sub2)
    Perception::addOutputVariable(multiplyFunction->getOutputType(pairClass(type1, type2)), name, PerceptionPtr());
  else
  {
    ProductPerceptionPtr product;
    if (sub1 && sub2)
      product = productPerception(multiplyFunction, sub1, sub2, symmetricFunction);
    else if (sub1)
      product = productPerception(multiplyFunction, sub1, type2);
    else if (sub2)
      product = productPerception(multiplyFunction, type1, sub2);
    else
      jassert(false);
    Perception::addOutputVariable(product->getOutputType(), name, product);
  }
}

void ProductPerception::computeOutputVariables()
{
  size_t n1 = perception1->getNumOutputVariables();
  size_t n2 = perception2->getNumOutputVariables();
  reserveOutputVariables(n1 * n2);
  for (size_t i = 0; i < n1; ++i)
    for (size_t j = 0; j < n2; ++j)
    {
      PerceptionPtr sub1 = perception1->getOutputVariableSubPerception(i);
      TypePtr type1 = perception1->getOutputVariableType(i);
      String name1 = perception1->getOutputVariableName(i);

      PerceptionPtr sub2 = perception2->getOutputVariableSubPerception(j);
      TypePtr type2 = perception2->getOutputVariableType(j);
      String name2 = perception2->getOutputVariableName(j);

      String name = name1 + T(" x ") + name2;
      addOutputVariable(name, type1, sub1, type2, sub2);
    }
}

/*
** ProductWithVariablePerception
*/
struct ComputePerceptionWithVariableProductCallback : public PerceptionCallback
{
  ComputePerceptionWithVariableProductCallback(const ProductWithVariablePerception* owner, const Variable& constant, PerceptionCallbackPtr targetCallback)
    : owner(owner), constant(constant), targetCallback(targetCallback) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {targetCallback->sense(variableNumber, productWith(value));}

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
  {
    PerceptionPtr newSubPerception = owner->getOutputVariableSubPerception(variableNumber);
    jassert(newSubPerception);
    return targetCallback->sense(variableNumber, newSubPerception, makePairWith(input));
  }

protected:
  const ProductWithVariablePerception* owner;
  Variable constant;
  PerceptionCallbackPtr targetCallback;

  Variable makePairWith(const Variable& value) const
  {
    if (owner->areVariablesSwapped())
      return Variable::pair(constant, value);
    else
      return Variable::pair(value, constant);
  }

  Variable productWith(const Variable& value) const
    {return owner->getMultiplyFunction()->compute(makePairWith(value));}
};

ProductWithVariablePerception::ProductWithVariablePerception(FunctionPtr multiplyFunction, PerceptionPtr perception, TypePtr variableType, bool swapVariables)
  : multiplyFunction(multiplyFunction), perception(perception), variableType(variableType), swapVariables(swapVariables)
{
  computeOutputVariables();
}

TypePtr ProductWithVariablePerception::getInputType() const
{
  if (swapVariables)
    return pairClass(variableType, perception->getInputType());
  else
    return pairClass(perception->getInputType(), variableType);
}

String ProductWithVariablePerception::toString() const
{
  String a = perception->toString();
  String b = variableType->getName();
  return (swapVariables ? b : a) + T(" x ") + (swapVariables ? a : b);
}

void ProductWithVariablePerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  Variable input1 = input[swapVariables ? 1 : 0];
  Variable input2 = input[swapVariables ? 0 : 1];

  jassert(input1.getType()->inheritsFrom(perception->getInputType()));
  jassert(input2.getType()->inheritsFrom(variableType));

  ComputePerceptionWithVariableProductCallback computeProductCallback(this, input2, callback);
  computeProductCallback.setStaticAllocationFlag();
  perception->computePerception(input1, &computeProductCallback);
}

void ProductWithVariablePerception::computeOutputVariables()
{
  size_t n = perception->getNumOutputVariables();
  reserveOutputVariables(n);
  for (size_t i = 0; i < n; ++i)
  {
    PerceptionPtr subPerception = perception->getOutputVariableSubPerception(i);
    if (subPerception)
    {
      subPerception = new ProductWithVariablePerception(multiplyFunction, subPerception, variableType, swapVariables);
      addOutputVariable(subPerception->getOutputType(), perception->getOutputVariableName(i), subPerception);
    }
    else
    {
      TypePtr type1 = perception->getOutputVariableType(i);
      TypePtr type2 = variableType;
      if (this->swapVariables)
        juce::swapVariables(type1, type2);
      TypePtr outputType = multiplyFunction->getOutputType(pairClass(type1, type2));
      addOutputVariable(outputType, perception->getOutputVariableName(i), PerceptionPtr());
    }
  }
}

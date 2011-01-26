/*-----------------------------------------.---------------------------------.
| Filename: ProductPerception.cpp          | Product of two Perceptions      |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2010 19:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProductPerception.h"
#include <lbcpp/Core/Pair.h>
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
  ComputePerceptionProductCallback(ExecutionContext& context, const ProductPerception* owner, std::list<PerceptionVariable>& variables, PerceptionCallbackPtr targetCallback)
    : PerceptionCallback(context), owner(owner), variables(variables), targetCallback(targetCallback) {}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    for (std::list<PerceptionVariable>::const_iterator it = variables.begin(); it != variables.end(); ++it)
      forwardSense(variableNumber, value, it->number, it->input);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
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
      targetCallback->sense(index, owner->getMultiplyFunction()->computeFunction(context, p));
  }
};

struct FillVariableListCallback : public PerceptionCallback
{
  FillVariableListCallback(ExecutionContext& context, std::list<PerceptionVariable>& variables)
    : PerceptionCallback(context), variables(variables) {}

  virtual void sense(size_t variableNumber, double value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {sense(variableNumber, Variable(value));}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    PerceptionVariable v;
    v.number = variableNumber;
    v.input = value;
    variables.push_back(v);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
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
  computeOutputType();
}

TypePtr ProductPerception::getInputType() const
{
  if (singleInputForBothPerceptions)
    return Type::findCommonBaseType(perception1->getInputType(), perception2->getInputType());
  else
    return pairClass(perception1->getInputType(), perception2->getInputType());
}

void ProductPerception::computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
{
  PairPtr inputPair;
  if (!singleInputForBothPerceptions)
    inputPair = input.getObjectAndCast<Pair>();

  // compute Perception2
  std::list<PerceptionVariable> variables;
  FillVariableListCallback fillVariableListCallback(context, variables);
  perception2->computePerception(context, inputPair ? inputPair->getSecond() : input, &fillVariableListCallback);

  // iterate over Perception1
  ComputePerceptionProductCallback computePerceptionProductCallback(context, this, variables, callback);
  perception1->computePerception(context, inputPair ? inputPair->getFirst() : input, &computePerceptionProductCallback);
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

void ProductPerception::computeOutputType()
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
  Perception::computeOutputType();
}

/*
** ProductWithVariablePerception
*/
struct ComputePerceptionWithVariableProductCallback : public PerceptionCallback
{
  ComputePerceptionWithVariableProductCallback(ExecutionContext& context, const ProductWithVariablePerception* owner, const Variable& constant, PerceptionCallbackPtr targetCallback)
    : PerceptionCallback(context), owner(owner), constant(constant), targetCallback(targetCallback) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {targetCallback->sense(variableNumber, productWith(value));}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
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
    {return owner->getMultiplyFunction()->computeFunction(context, makePairWith(value));}
};

ProductWithVariablePerception::ProductWithVariablePerception(FunctionPtr multiplyFunction, PerceptionPtr perception, TypePtr variableType, bool swapVariables)
  : multiplyFunction(multiplyFunction), perception(perception), variableType(variableType), swapVariables(swapVariables)
{
  computeOutputType();
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

void ProductWithVariablePerception::computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
{
  Variable input1 = input[swapVariables ? 1 : 0];
  Variable input2 = input[swapVariables ? 0 : 1];

  jassert(input1.getType()->inheritsFrom(perception->getInputType()));
  jassert(input2.getType()->inheritsFrom(variableType));

  ComputePerceptionWithVariableProductCallback computeProductCallback(context, this, input2, callback);
  perception->computePerception(context, input1, &computeProductCallback);
}

void ProductWithVariablePerception::computeOutputType()
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
  Perception::computeOutputType();
}

/*-----------------------------------------.---------------------------------.
| Filename: Frame.cpp                      | Frame                           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 20:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/Frame.h>
#include <lbcpp/Function/Function.h>
using namespace lbcpp;

namespace lbcpp {extern ClassPtr frameClassClass;};

/*
** FrameClass
*/
FrameClass::FrameClass(const String& name, TypePtr baseClass)
  : DefaultClass(name, baseClass) {}

FrameClass::FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
  : DefaultClass(templateType, templateArguments, baseClass) {}

ClassPtr FrameClass::getClass() const
  {return frameClassClass;}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& function, size_t input, const String& outputName, const String& outputShortName)
  {return addMemberOperator(context, function, std::vector<size_t>(1, input), outputName, outputShortName);}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& function, size_t input1, size_t input2, const String& outputName, const String& outputShortName)
{
  std::vector<size_t> inputs(2);
  inputs[0] = input1;
  inputs[1] = input2;
  return addMemberOperator(context, function, inputs, outputName, outputShortName);
}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, const std::vector<size_t>& inputs, const String& outputName, const String& outputShortName)
{
  FrameOperatorSignaturePtr signature = new FrameOperatorSignature(operation, inputs, outputName, outputShortName);
  size_t res = addMemberVariable(context, signature);
  initializeFunctionTypes(context, signature);
  return res;
}

bool FrameClass::initialize(ExecutionContext& context)
{
 /* for (size_t i = 0; i < variables.size(); ++i)
  {
    FrameOperatorSignaturePtr signature = variables[i].dynamicCast<FrameOperatorSignature>();
    if (signature && !initializeFunctionTypes(context, signature))
      return false;
  }*/
  return DefaultClass::initialize(context);
}

bool FrameClass::initializeFunctionTypes(ExecutionContext& context, const FrameOperatorSignaturePtr& signature)
{
  const FunctionPtr& function = signature->getFunction();
  jassert(function);
  const std::vector<size_t>& inputs = signature->getInputIndices();

  std::vector<VariableSignaturePtr> inputVariables(inputs.size());
  for (size_t i = 0; i < inputVariables.size(); ++i)
  {
    size_t inputIndex = inputs[i];
    if (inputIndex == (size_t)-1)
    {
      // this
      inputVariables[i] = new VariableSignature(refCountedPointerFromThis(this), T("this"));
    }
    else
    {
      if (inputIndex >= variables.size())
      {
        context.errorCallback(T("Invalid index: ") + String((int)inputIndex));
        return false;
      }
      inputVariables[i] = variables[inputIndex];
    }
  }
  if (!function->initialize(context, inputVariables))
    return false;

  FrameOperatorSignaturePtr autoSignature = function->getOutputVariable();
  signature->setType(autoSignature->getType());
  if (signature->getName().isEmpty())
    signature->setName(autoSignature->getName());
  if (signature->getShortName().isEmpty())
    signature->setShortName(autoSignature->getShortName());
  if (signature->getDescription().isEmpty())
    signature->setDescription(autoSignature->getDescription());
  return true;
}

/*
** Frame
*/
Frame::Frame(ClassPtr frameClass)
  : DenseGenericObject(frameClass) {}

Frame::Frame(ClassPtr frameClass, const Variable& firstVariable)
  : DenseGenericObject(frameClass)
{
  jassert(getNumVariables() > 0);
  setVariable(0, firstVariable);
}

Frame::Frame(ClassPtr frameClass, const Variable& firstVariable, const Variable& secondVariable)
  : DenseGenericObject(frameClass)
{
  jassert(getNumVariables() > 1);
  setVariable(0, firstVariable);
  setVariable(1, secondVariable);
}

Frame::Frame(ClassPtr frameClass, const Variable& firstVariable, const Variable& secondVariable, const Variable& thirdVariable)
  : DenseGenericObject(frameClass)
{
  jassert(getNumVariables() > 2);
  setVariable(0, firstVariable);
  setVariable(1, secondVariable);
  setVariable(2, thirdVariable);
}

void Frame::ensureAllVariablesAreComputed()
{
  size_t n = thisClass->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
    getOrComputeVariable(i);
}

bool Frame::isVariableComputed(size_t index) const
{
  return index < variableValues.size() &&
    !thisClass->getMemberVariableType(index)->isMissingValue(variableValues[index]);
}

Variable Frame::getOrComputeVariable(size_t index)
{
  if (index == (size_t)-1)
    return this;

  VariableSignaturePtr signature = thisClass->getMemberVariable(index);
  VariableValue& variableValue = getVariableValueReference(index);
  const TypePtr& type = signature->getType();
  if (!type->isMissingValue(variableValue))
    return Variable::copyFrom(type, variableValue);

  FrameOperatorSignaturePtr operatorSignature = signature.dynamicCast<FrameOperatorSignature>();
  if (!operatorSignature)
    return Variable();
  
  const std::vector<size_t>& inputIndices = operatorSignature->getInputIndices();
  std::vector<Variable> inputs(inputIndices.size());
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    inputs[i] = getOrComputeVariable(inputIndices[i]);
    if (!inputs[i].exists())
      return Variable();
  }
  Variable value = operatorSignature->getFunction()->compute(defaultExecutionContext(), &inputs[0]);
  setVariable(index, value);
  return value;
}

Variable Frame::getVariable(size_t index) const
  {return const_cast<Frame* >(this)->getOrComputeVariable(index);}

/*
** FrameBasedFunction
*/
TypePtr FrameBasedFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  jassert(frameClass);
  for (size_t i = 0; i < frameClass->getNumMemberVariables(); ++i)
  {
    FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(i).dynamicCast<FrameOperatorSignature>();
    if (signature)
      subFunctions.push_back(signature->getFunction());
  }

  VariableSignaturePtr lastMemberVariable = frameClass->getMemberVariable(frameClass->getNumMemberVariables() - 1);
  outputName = lastMemberVariable->getName();
  outputShortName = lastMemberVariable->getShortName();
  return lastMemberVariable->getType();
}

Variable FrameBasedFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  FramePtr frame(new Frame(frameClass));
  for (size_t i = 0; i < getNumInputs(); ++i)
    frame->setVariable(i, inputs[i]);
  return frame->getVariable(frameClass->getNumMemberVariables() - 1);
}

/*-----------------------------------------.---------------------------------.
| Filename: CompositeFunction.cpp          | Composite Function              |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2011 19:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/CompositeFunction.h>
using namespace lbcpp;

/*
** CompositeFunction
*/
TypePtr CompositeFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  CompositeFunctionBuilder builder(context, refCountedPointerFromThis(this), inputVariables);
  buildFunction(builder);

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

Variable CompositeFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  FramePtr frame(new Frame(frameClass));
  for (size_t i = 0; i < getNumInputs(); ++i)
    frame->setVariable(i, inputs[i]);
  return frame->getVariable(frameClass->getNumMemberVariables() - 1);
}

/*
** CompositeFunctionBuilder
*/
CompositeFunctionBuilder::CompositeFunctionBuilder(ExecutionContext& context, CompositeFunctionPtr function, const std::vector<VariableSignaturePtr>& inputVariables)
  : context(context), frameClass(new FrameClass(T("Function"))), inputVariables(inputVariables)
{
  function->setFrameClass(frameClass);
}

size_t CompositeFunctionBuilder::addInput(TypePtr type, const String& name)
{
  size_t inputNumber = frameClass->getNumMemberVariables();
  const VariableSignaturePtr& inputVariable = inputVariables[inputNumber];
  if (!context.checkInheritance(inputVariable->getType(), type))
    return invalidIndex();

  String n = name.isEmpty() ? inputVariable->getName() : name;
  return addInSelection(frameClass->addMemberVariable(context, inputVariable->getType(), n));
}

size_t CompositeFunctionBuilder::addConstant(const Variable& value, const String& name)
  {jassert(false); return 0;}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& function, size_t input, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, input, outputName, outputShortName));}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, input1, input2, outputName, outputShortName));}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& function, std::vector<size_t>& inputs, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, inputs, outputName, outputShortName));}

void CompositeFunctionBuilder::startSelection()
  {currentSelection.clear();}

const std::vector<size_t>& CompositeFunctionBuilder::finishSelection()
  {return currentSelection;}

size_t CompositeFunctionBuilder::finishSelectionWithFunction(const FunctionPtr& function)
{
  size_t res = addFunction(function, currentSelection);
  currentSelection.clear();
  return res;
}

size_t CompositeFunctionBuilder::addInSelection(size_t index)
{
  currentSelection.push_back(index);
  return index;
}

TypePtr CompositeFunctionBuilder::getOutputType() const
{
  size_t n = frameClass->getNumMemberVariables();
  jassert(n);
  return frameClass->getMemberVariableType(n - 1);
}

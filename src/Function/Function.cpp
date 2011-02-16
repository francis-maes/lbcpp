/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 18:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Function/Function.h>
#include <lbcpp/Core/Frame.h>
#include <lbcpp/Function/Evaluator.h>
using namespace lbcpp;

bool Function::initialize(ExecutionContext& context, TypePtr inputType)
{
  std::vector<VariableSignaturePtr> inputVariables(1);
  inputVariables[0] = new VariableSignature(inputType, T("input"));
  return initialize(context, inputVariables);
}

bool Function::initialize(ExecutionContext& context, VariableSignaturePtr inputVariable)
  {return initialize(context, std::vector<VariableSignaturePtr>(1, inputVariable));}

bool Function::initialize(ExecutionContext& context, const std::vector<TypePtr>& inputTypes)
{
  std::vector<VariableSignaturePtr> inputVariables(inputTypes.size());
  for (size_t i = 0; i < inputVariables.size(); ++i)
    inputVariables[i] = new VariableSignature(inputTypes[i], T("input") + String((int)i + 1));
  return initialize(context, inputVariables);
}

bool Function::initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables)
{
  // check inputs
  numInputs = inputVariables.size();
  size_t minInputs = getMinimumNumRequiredInputs();
  if (minInputs && numInputs < minInputs)
  {
    context.errorCallback(T("Missing input: expected ") + String((int)minInputs) + T(" inputs, found only ") + numInputs + T(" inputs"));
    return false;
  }

  size_t maxInputs = getMaximumNumRequiredInputs();
  if (numInputs > maxInputs)
  {
    context.errorCallback(T("Too much inputs: expected ") + String((int)maxInputs) + T(" inputs, found ") + numInputs + T(" inputs"));
    return false;
  }

  for (size_t i = 0; i < numInputs; ++i)
    if (!context.checkInheritance(inputVariables[i]->getType(), getRequiredInputType(i, numInputs)))
      return false;

  // make inputs class
  inputsClass = new DynamicClass(getClassName() + T("Inputs"));
  inputsClass->reserveMemberVariables(inputVariables.size() + 1);
  for (size_t i = 0; i < inputVariables.size(); ++i)
    inputsClass->addMemberVariable(context, inputVariables[i]);
  
  // compute output type
  String outputPostFix = getOutputPostFix();
  String outputName = (numInputs ? inputVariables[0]->getName() : String::empty) + outputPostFix;
  String outputShortName = (numInputs ? inputVariables[0]->getShortName() : String::empty) + outputPostFix;
  TypePtr outputType = initializeFunction(context, inputVariables, outputName, outputShortName);
  if (!outputType)
    return false;
  outputVariable = new VariableSignature(outputType, outputName, outputShortName);
  return true;
}

void Function::setBatchLearner(const FunctionPtr& batchLearner)
{
/*  jassert(frameClass);
  std::vector<TypePtr> types(3);
  types[0] = getClass();
  types[1] = containerClass(frameClass);
  types[2] = containerClass(frameClass);
  learner->initialize(defaultExecutionContext(), types);*/
  this->batchLearner = batchLearner;
}

Variable Function::compute(ExecutionContext& context, const Variable* inputs) const
{
  if (pushIntoStack)
    context.enterScope(getDescription(context, inputs));

  for (size_t i = 0; i < preCallbacks.size(); ++i)
    preCallbacks[i]->functionCalled(context, refCountedPointerFromThis(this), inputs);

  Variable res = computeFunction(context, inputs);

  for (size_t i = 0; i < postCallbacks.size(); ++i)
    postCallbacks[i]->functionReturned(context, refCountedPointerFromThis(this), inputs, res);

  if (pushIntoStack)
    context.leaveScope(res);
  return res;
}

Variable Function::compute(ExecutionContext& context, const Variable& input) const
  {jassert(getNumInputs() == 1); return compute(context, &input);}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2) const
{
  jassert(getNumInputs() == 2);
  Variable v[2];
  v[0] = input1;
  v[1] = input2;
  return compute(context, v);
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3) const
{
  jassert(getNumInputs() == 3);
  Variable v[3];
  v[0] = input1;
  v[1] = input2;
  v[2] = input3;
  return compute(context, v);
}

Variable Function::compute(ExecutionContext& context, const std::vector<Variable>& inputs) const
  {return compute(context, &inputs[0]);}

Variable Function::computeWithInputsObject(ExecutionContext& context, const ObjectPtr& inputsObject) const
{
  std::vector<Variable> inputs(getNumInputs());
  for (size_t j = 0; j < inputs.size(); ++j)
    inputs[j] = inputsObject->getVariable(j);
  return compute(context, inputs);
}

bool Function::train(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& validationData)
{
  if (!batchLearner)
    return false;
  batchLearner->compute(context, this, trainingData, validationData);
  return true;
}

static void evaluateFunctionOnExample(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& example, const EvaluatorPtr& evaluator)
{
  Variable prediction = function->computeWithInputsObject(context, example);
  Variable correct = example->getVariable(example->getNumVariables() - 1);
  evaluator->addPrediction(context, prediction, correct);
}

bool Function::evaluate(ExecutionContext& context, const ContainerPtr& examples, const EvaluatorPtr& evaluator) const
{
  // todo: parallel evaluation
  size_t n = examples->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr example = examples->getElement(i).getObject();
    evaluateFunctionOnExample(context, refCountedPointerFromThis(this), example, evaluator);
  }
  return true;
}

bool Function::evaluate(ExecutionContext& context, const std::vector<ObjectPtr>& examples, const EvaluatorPtr& evaluator) const
{
  // todo: parallel evaluation
  size_t n = examples.size();
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr example = examples[i];
    evaluateFunctionOnExample(context, refCountedPointerFromThis(this), example, evaluator);
  }
  return true;
}

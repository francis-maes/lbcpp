/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 18:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/Function.h>
#include <lbcpp/Core/Frame.h>
#include <lbcpp/Function/OldEvaluator.h>
#include <lbcpp/Learning/BatchLearner.h>
using namespace lbcpp;

bool Function::initialize(ExecutionContext& context, TypePtr inputType)
{
  std::vector<VariableSignaturePtr> inputVariables(1);
  inputVariables[0] = new VariableSignature(inputType, T("input"));
  return initialize(context, inputVariables);
}

bool Function::initialize(ExecutionContext& context, TypePtr inputType1, TypePtr inputType2)
{
  std::vector<VariableSignaturePtr> inputVariables(2);
  inputVariables[0] = new VariableSignature(inputType1, T("firstInput"));
  inputVariables[1] = new VariableSignature(inputType2, T("secondInput"));
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
  if (isInitialized())
  {
    // todo: check that inputVariables type has not changed
    return true;
  }

  // check inputs
  numInputs = inputVariables.size();
  size_t minInputs = getMinimumNumRequiredInputs();
  if (minInputs && numInputs < minInputs)
  {
    context.errorCallback(T("Missing input: expected ") + String((int)minInputs) + T(" inputs, found only ") + String((int)numInputs) + T(" inputs"));
    return false;
  }

  size_t maxInputs = getMaximumNumRequiredInputs();
  if (numInputs > maxInputs)
  {
    context.errorCallback(T("Too much inputs: expected ") + String((int)maxInputs) + T(" inputs, found ") + String((int)numInputs) + T(" inputs"));
    return false;
  }

  for (size_t i = 0; i < numInputs; ++i)
  {
    TypePtr requiredInputType = getRequiredInputType(i, numInputs);
    if (!context.checkInheritance(inputVariables[i]->getType(), requiredInputType))
      return false;
  }

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
  for (size_t i = 0; i < preCallbacks.size(); ++i)
    preCallbacks[i]->functionCalled(context, refCountedPointerFromThis(this), inputs);

  Variable res = computeFunction(context, inputs);

  for (size_t i = 0; i < postCallbacks.size(); ++i)
    postCallbacks[i]->functionReturned(context, refCountedPointerFromThis(this), inputs, res);
  return res;
}

Variable Function::compute(ExecutionContext& context, const Variable& input) const
  {jassert(getNumInputs() == 1); return compute(context, &input);}

static inline void fastVariableCopy(juce::int64* dst, const Variable& source)
{
  const juce::int64* src = (const juce::int64* )&source;
  dst[0] = src[0];
  dst[1] = src[1];
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2) const
{
  jassert(getNumInputs() == 2);
  jassert(sizeof (Variable) == sizeof (juce::int64) * 2);
  juce::int64 tmp[4];
  fastVariableCopy(tmp, input1);
  fastVariableCopy(tmp + 2, input2);
  return compute(context, (const Variable* )tmp);
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3) const
{
  jassert(getNumInputs() == 3);
  juce::int64 tmp[6];
  fastVariableCopy(tmp, input1);
  fastVariableCopy(tmp + 2, input2);
  fastVariableCopy(tmp + 4, input3);
  return compute(context, (const Variable* )tmp);
}

Variable Function::compute(ExecutionContext& context, const std::vector<Variable>& inputs) const
  {return compute(context, &inputs[0]);}

Variable Function::computeWithInputsObject(ExecutionContext& context, const ObjectPtr& inputsObject) const
{
  if (getNumInputs() == 1 && inputsObject->getClass()->inheritsFrom(getRequiredInputType(0, 1)))
  {
    Variable in(inputsObject);
    return compute(context, &in);
  }

  PairPtr inputPair = inputsObject.dynamicCast<Pair>();
  if (inputPair)
  {
    // faster version for pairs
    const Variable* inputs = &inputPair->getFirst();
    jassert(inputs + 1 == &inputPair->getSecond());
    return compute(context, inputs);
  }
  else
  {
    std::vector<Variable> inputs(getNumInputs());
    for (size_t j = 0; j < inputs.size(); ++j)
      inputs[j] = inputsObject->getVariable(j);
    return compute(context, &inputs[0]);
  }
}

bool Function::checkIsInitialized(ExecutionContext& context) const
{
  if (!outputVariable)
  {
    context.errorCallback(T("Function ") + toShortString(), T("Not initialized"));
    return false;
  }
  return true;
}

bool Function::train(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& validationData, const String& scopeName, bool returnLearnedFunction)
{
  bool doScope = scopeName.isNotEmpty();
  if (doScope)
    context.enterScope(scopeName);
  bool res = true;

  if (!isInitialized())
  {
    // auto-initialize given examples type
    TypePtr examplesType = trainingData->getElementsType();
    std::vector<VariableSignaturePtr> inputSignatures(examplesType->getNumMemberVariables());
    for (size_t i = 0; i < inputSignatures.size(); ++i)
      inputSignatures[i] = examplesType->getMemberVariable(i);
    if (!initialize(context, inputSignatures))
      res = false;
  }

  if (res)
  {
    if (!batchLearner)
    {
      context.errorCallback(T("Function ") + toShortString(), T("No batch learners"));
      res = false;
    }
    else
      res = batchLearner->compute(context, this, trainingData, validationData).getBoolean();
  }

  if (doScope)
  {
    if (returnLearnedFunction)
      context.resultCallback(T("learned"), refCountedPointerFromThis(this));
    context.leaveScope(Variable());
  }
  return res;
}

bool Function::train(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData, const String& scopeName, bool returnLearnedFunction)
  {return train(context, new ObjectVector(trainingData), new ObjectVector(validationData), scopeName, returnLearnedFunction);}

ScoreObjectPtr Function::evaluate(ExecutionContext& context, const ContainerPtr& examples, const EvaluatorPtr& evaluator, const String& scopeName) const
{
  bool doScope = scopeName.isNotEmpty();

  if (!checkIsInitialized(context))
    return ScoreObjectPtr();

  if (!evaluator->isInitialized() && !evaluator->initialize(context, functionClass, examples->getClass()))
    return ScoreObjectPtr();

  if (doScope)
    context.enterScope(scopeName);

  ScoreObjectPtr res = evaluator->compute(context, refCountedPointerFromThis(this), examples).getObjectAndCast<ScoreObject>();

  if (doScope)
  {
    context.resultCallback(T("score"), res);
    context.leaveScope(-res->getScoreToMinimize());
  }
  return res;
}

ScoreObjectPtr Function::evaluate(ExecutionContext& context, const std::vector<ObjectPtr>& examples, const EvaluatorPtr& evaluator, const String& scopeName) const
  {return evaluate(context, new ObjectVector(examples), evaluator, scopeName);}

String Function::toString() const
{
  return toShortString();
}

String Function::toShortString() const
{
  ClassPtr thisClass = getClass();
  String shortName = thisClass->getShortName();
  if (shortName.isNotEmpty())
    return shortName;
  else
    return thisClass->getName();
}

/*
** ProxyFunction
*/
TypePtr ProxyFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  implementation = createImplementation(inputVariables);
  if (!implementation)
  {
    context.errorCallback(T("Could not create implementation in proxy operator"));
    return TypePtr();
  }
  if (!implementation->initialize(context, inputVariables))
    return TypePtr();

  const VariableSignaturePtr& v = implementation->getOutputVariable();
  outputName = v->getName();
  outputShortName = v->getShortName();
  setBatchLearner(proxyFunctionBatchLearner());
  return v->getType();
}

Variable ProxyFunction::computeFunction(ExecutionContext& context, const Variable& input) const
  {jassert(implementation); return implementation->compute(context, input);}

Variable ProxyFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
  {jassert(implementation); return implementation->compute(context, inputs);}

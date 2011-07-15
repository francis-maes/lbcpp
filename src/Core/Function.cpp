/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 18:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Function.h>
#include <lbcpp/Core/Frame.h>
#include <lbcpp/Learning/BatchLearner.h>
#include <lbcpp/Function/Evaluator.h>
using namespace lbcpp;

bool Function::initialize(ExecutionContext& context, TypePtr inputType)
{
  std::vector<VariableSignaturePtr> inputVariables(1);
  inputVariables[0] = new VariableSignature(inputType, T("input"));
  return initialize(context, inputVariables);
}

bool Function::initialize(ExecutionContext& context, ClassPtr inputClass)
{
  std::vector<VariableSignaturePtr> inputVariables(1);
  inputVariables[0] = new VariableSignature(inputClass, T("input"));
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
    // check that input variable type have not changed
    if (inputVariables.size() != this->inputVariables.size())
    {
      context.errorCallback(toShortString(), T("Number of inputs has changed. Previous count was ") +
                            String((int)this->inputVariables.size()) + T(", new count is ") + String((int)inputVariables.size()));
      return false;
    }
    for (size_t i = 0; i < inputVariables.size(); ++i)
      if (inputVariables[i]->getType() != this->inputVariables[i]->getType())
      {
        context.errorCallback(toShortString(), T("Input #") + String((int)i + 1) + T(" type has changed. Previous type was ") +
            this->inputVariables[i]->getType()->getName() + T(", new type is ") + inputVariables[i]->getType()->getName());
        return false;
      }
    return true;
  }
  this->inputVariables = inputVariables;

  // check inputs
  size_t numInputs = inputVariables.size();
  size_t minInputs = getMinimumNumRequiredInputs();
  if (minInputs && numInputs < minInputs)
  {
    context.errorCallback(toShortString(), T("Missing input: expected ") + String((int)minInputs) + T(" inputs, found only ") + String((int)numInputs) + T(" inputs"));
    return false;
  }

  size_t maxInputs = getMaximumNumRequiredInputs();
  if (numInputs > maxInputs)
  {
    context.errorCallback(toShortString(), T("Too much inputs: expected ") + String((int)maxInputs) + T(" inputs, found ") + String((int)numInputs) + T(" inputs"));
    return false;
  }

  for (size_t i = 0; i < numInputs; ++i)
  {
    if (!inputVariables[i])
    {
      context.errorCallback(toShortString(), T("Missing input #") + String((int)i + 1));
      return false;
    }

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
  {this->batchLearner = batchLearner;}

Variable Function::compute(ExecutionContext& context, const Variable* inputs, size_t numInputs) const
{
  if (!isInitialized())
  {
    // /!!\ This is not thread-safe
    std::vector<VariableSignaturePtr> inputVariables(numInputs);
    for (size_t i = 0; i < inputVariables.size(); ++i)
    {
      TypePtr type = inputs[i].getType();
      inputVariables[i] = new VariableSignature(type, T("input") + String((int)i + 1));
    }
    //context.informationCallback(T("Auto-initialize function ") + toShortString());
    if (!const_cast<Function* >(this)->initialize(context, inputVariables))
      return Variable();
  }
  if (numInputs != getNumInputs())
  {
    context.errorCallback(T("Invalid number of inputs"));
    return Variable();
  }

  for (size_t i = 0; i < preCallbacks.size(); ++i)
    preCallbacks[i]->functionCalled(context, refCountedPointerFromThis(this), inputs);

  Variable res = computeFunction(context, inputs);
  checkInheritance(res.getType(), getOutputType());

  for (size_t i = 0; i < postCallbacks.size(); ++i)
    postCallbacks[i]->functionReturned(context, refCountedPointerFromThis(this), inputs, res);
  return res;
}

Variable Function::compute(ExecutionContext& context, const Variable& input) const
  {return compute(context, &input, 1);}

static inline void fastVariableCopy(juce::int64* dst, const Variable& source)
{
  const juce::int64* src = (const juce::int64* )&source;
  dst[0] = src[0];
  dst[1] = src[1];
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2) const
{
  jassert(sizeof (Variable) == sizeof (juce::int64) * 2);
  juce::int64 tmp[4];
  fastVariableCopy(tmp, input1);
  fastVariableCopy(tmp + 2, input2);
  return compute(context, (const Variable* )tmp, 2);
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3) const
{
  juce::int64 tmp[6];
  fastVariableCopy(tmp, input1);
  fastVariableCopy(tmp + 2, input2);
  fastVariableCopy(tmp + 4, input3);
  return compute(context, (const Variable* )tmp, 3);
}

Variable Function::compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3, const Variable& input4) const
{
  juce::int64 tmp[8];
  fastVariableCopy(tmp, input1);
  fastVariableCopy(tmp + 2, input2);
  fastVariableCopy(tmp + 4, input3);
  fastVariableCopy(tmp + 6, input4);
  return compute(context, (const Variable* )tmp, 4);
}

Variable Function::compute(ExecutionContext& context, const std::vector<Variable>& inputs) const
  {return compute(context, &inputs[0], inputs.size());}

Variable Function::computeWithInputsObject(ExecutionContext& context, const ObjectPtr& inputsObject) const
{
  jassert(inputsObject);

  if (getNumInputs() == 1 && inputsObject->getClass()->inheritsFrom(getRequiredInputType(0, 1)))
  {
    Variable in(inputsObject);
    return compute(context, in);
  }

  PairPtr inputPair = inputsObject.dynamicCast<Pair>();
  if (inputPair)
  {
    // faster version for pairs
    const Variable* inputs = &inputPair->getFirst();
    jassert(inputs + 1 == &inputPair->getSecond());
    return compute(context, inputs, 2);
  }
  else
  {
    std::vector<Variable> inputs(getNumInputs());
    for (size_t j = 0; j < inputs.size(); ++j)
      inputs[j] = inputsObject->getVariable(j);
    return compute(context, inputs);
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

bool Function::initializeWithInputsObjectClass(ExecutionContext& context, ClassPtr inputsObjectClass)
{
  if (getMinimumNumRequiredInputs() == 1 && getMaximumNumRequiredInputs() == 1 && inputsObjectClass->inheritsFrom(getRequiredInputType(0, 1)))
    return initialize(context, (TypePtr)inputsObjectClass);

  std::vector<VariableSignaturePtr> inputSignatures(inputsObjectClass->getNumMemberVariables());
  for (size_t i = 0; i < inputSignatures.size(); ++i)
    inputSignatures[i] = inputsObjectClass->getMemberVariable(i);
  return initialize(context, inputSignatures);
}

ScoreObjectPtr Function::train(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& validationData, const String& scopeName, bool returnLearnedFunction)
{
  bool doScope = scopeName.isNotEmpty();
  if (doScope)
    context.enterScope(scopeName);
  ScoreObjectPtr res;

  // auto-initialize given examples type
  if (isInitialized() || initializeWithInputsObjectClass(context, trainingData->getElementsType()))
  {
    if (!batchLearner)
    {
      //context.errorCallback(T("Function ") + toShortString(), T("No batch learners"));
      res = new DummyScoreObject();
    }
    else if (batchLearner->compute(context, this, trainingData, Variable(validationData, trainingData->getClass())).getBoolean())
    {
      // evaluate
      if (hasEvaluator())
      {
        ScoreObjectPtr trainScore = evaluate(context, trainingData, EvaluatorPtr(), T("Train evaluation"));
        if (trainScore)
        {
          context.resultCallback(T("Train score"), trainScore->getScoreToMinimize());
          res = trainScore;
        }

        if (validationData)
        {
          ScoreObjectPtr validationScore = evaluate(context, validationData, EvaluatorPtr(), T("Validation evaluation"));
          if (validationScore)
          {
            context.resultCallback(T("Validation score"), validationScore->getScoreToMinimize());
            res = validationScore;
          }
        }
      }
      else
        res = new DummyScoreObject();
    }
  }


  if (doScope)
  {
    if (returnLearnedFunction)
      context.resultCallback(T("learned"), refCountedPointerFromThis(this));
    context.leaveScope(res ? Variable(res->getScoreToMinimize()) : Variable(false));
  }
  return res;
}

ScoreObjectPtr Function::train(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData, const String& scopeName, bool returnLearnedFunction)
  {return train(context, new ObjectVector(trainingData), new ObjectVector(validationData), scopeName, returnLearnedFunction);}

ScoreObjectPtr Function::evaluate(ExecutionContext& context, const ContainerPtr& examples, const EvaluatorPtr& evaluator, const String& scopeName) const
{
  bool doScope = scopeName.isNotEmpty();

  EvaluatorPtr eval = evaluator ? evaluator : this->evaluator;
  if (!eval)
  {
    context.errorCallback(T("Could not evaluate function ") + toShortString() + T(": no evaluator"));
    return ScoreObjectPtr();
  }

  if (!eval->isInitialized() && !eval->initialize(context, functionClass, examples->getClass()))
    return ScoreObjectPtr();

  if (doScope)
    context.enterScope(scopeName);

  bool learningFlag = learning;
  const_cast<Function* >(this)->learning = false;
  ScoreObjectPtr res = eval->compute(context, refCountedPointerFromThis(this), examples).getObjectAndCast<ScoreObject>();
  const_cast<Function* >(this)->learning = learningFlag;

  if (doScope)
  {
    context.resultCallback(T("score"), res);
    context.leaveScope(res->getScoreToMinimize());
  }
  return res;
}

ScoreObjectPtr Function::evaluate(ExecutionContext& context, const std::vector<ObjectPtr>& examples, const EvaluatorPtr& evaluator, const String& scopeName) const
  {return evaluate(context, new ObjectVector(examples), evaluator, scopeName);}

String Function::toShortString() const
{
  ClassPtr thisClass = getClass();
  String shortName = thisClass->getShortName();
  if (shortName.isNotEmpty())
    return shortName;
  else
    return thisClass->getName();
}

ObjectPtr Function::clone(ExecutionContext& context) const
  {return Object::clone(context);}

void Function::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Object::clone(context, t);
  //const FunctionPtr& target = t.staticCast<Function>();
  //if (isInitialized())
  //  target->initialize(context, inputVariables);
}


void Function::removePostCallback(const FunctionCallbackPtr& callback)
{
  for (size_t i = 0; i < postCallbacks.size(); ++i)
    if (postCallbacks[i] == callback)
    {
      postCallbacks.erase(postCallbacks.begin() + i);
      return;
    }
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
  implementation->setEvaluator(evaluator);
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
  {jassert(implementation); return implementation->compute(context, inputs, getNumInputs());}

/*
** UnaryHigherOrderFunction
*/
UnaryHigherOrderFunction::UnaryHigherOrderFunction(FunctionPtr baseFunction)
  : baseFunction(baseFunction)
{
  setBatchLearner(unaryHigherOrderFunctionBatchLearner());
}

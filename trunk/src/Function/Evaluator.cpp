/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 10:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Function/Evaluator.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Core/DynamicObject.h>
using namespace lbcpp;

namespace lbcpp 
{

struct EvaluateExampleWorkUnit : public WorkUnit
{
  EvaluateExampleWorkUnit(const EvaluatorPtr& evaluator, const FunctionPtr& function, const ObjectPtr& example, const ScoreObjectPtr& scores, CriticalSection& lock)
    : evaluator(evaluator), function(function), example(example), scores(scores), lock(lock) {}

  virtual Variable run(ExecutionContext& context)
  {
    evaluator->evaluateExample(context, scores, function, example, &lock);
    return Variable();
  }
  
protected:
  const EvaluatorPtr& evaluator;
  const FunctionPtr& function;
  ObjectPtr example;
  const ScoreObjectPtr& scores;
  CriticalSection& lock;
};

}; /* namespace lbcpp */

bool Evaluator::evaluateExample(ExecutionContext& context, const ScoreObjectPtr& scores, const FunctionPtr& function, const ObjectPtr& example, CriticalSection* lock) const
{
  if (!example)
    return true; // skip missing examples
  Variable output = function->computeWithInputsObject(context, example);
  if (lock)
    lock->enter();
  bool res = updateScoreObject(context, scores, example, output);
  if (lock)
    lock->exit();
  return res;
}

void Evaluator::computeEvaluatorMultiThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const
{
  size_t n = examples->getNumElements();
  CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Evaluating"), n));
  workUnit->setPushChildrenIntoStackFlag(false);
  workUnit->setProgressionUnit(T("Examples"));
  EvaluatorPtr pthis = refCountedPointerFromThis(this);
  CriticalSection lock;
  for (size_t i = 0; i < n; ++i)
    workUnit->setWorkUnit(i, new EvaluateExampleWorkUnit(pthis, function, examples->getElement(i).getObject(), scores, lock));
  context.run(workUnit, false);
}

void Evaluator::computeEvaluatorSingleThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const
{
  size_t n = examples->getNumElements();

  ProgressionStatePtr progression = new ProgressionState(0, n, T("Examples"));
  ObjectVectorPtr objectExamples = examples.dynamicCast<ObjectVector>();

  juce::uint32 lastProgressionTime = 0;
  if (objectExamples)
  {
    // fast version for object vectors
    const std::vector<ObjectPtr>& objects = objectExamples->getObjects();
    for (size_t i = 0; i < n; ++i)
    {
      const ObjectPtr& example = objects[i];
      if (!evaluateExample(context, scores, function, example))
        return;

      juce::uint32 time = Time::getApproximateMillisecondCounter();
      if ((i == n - 1) || (time > lastProgressionTime + 1000))
      {
        progression->setValue(i + 1);
        context.progressCallback(progression);
        lastProgressionTime = time;
      }
    }
  }
  else
  { 
    // generic version
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr example = examples->getElement(i).getObject();
      if (!evaluateExample(context, scores, function, example))
        return;

      juce::uint32 time = Time::getApproximateMillisecondCounter();
      if ((i == n - 1) || (time > lastProgressionTime + 1000))
      {
        progression->setValue(i + 1);
        context.progressCallback(progression);
        lastProgressionTime = time;
      }
    }
  }
}

Variable Evaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const FunctionPtr& function = inputs[0].getObjectAndCast<Function>();
  const ContainerPtr& examples = inputs[1].getObjectAndCast<Container>();

  ScoreObjectPtr res = createEmptyScoreObject(context, function);
  if (!res)
    return Variable::missingValue(getOutputType());

  if (useMultiThreading && context.isMultiThread())
    computeEvaluatorMultiThread(context, function, examples, res);
  else
    computeEvaluatorSingleThread(context, function, examples, res);
  finalizeScoreObject(res, function);
  return res;
}

/*
** SupervisedEvaluator
*/
bool SupervisedEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& inputsObject, const Variable& output) const
{
  if (!output.exists())
  {
    context.errorCallback(T("SupervisedEvaluator::updateScoreObject"), T("Missing output"));
    return false;
  }
  Variable supervision = inputsObject->getVariable(inputsObject->getNumVariables() - 1);
  if (supervision.exists())
    addPrediction(context, output, supervision, scores);
  return true;
}

/*
** CompositeEvaluator
*/
ScoreObjectPtr CompositeEvaluator::createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
{
  CompositeScoreObjectPtr res = new CompositeScoreObject();
  for (size_t i = 0; i < evaluators.size(); ++i)
    res->addScoreObject(evaluators[i]->createEmptyScoreObject(context, function));
  return res;
}

bool CompositeEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
{
  bool res = true;
  for (size_t i = 0; i < evaluators.size(); ++i)
    res &= evaluators[i]->updateScoreObject(context, scores.staticCast<CompositeScoreObject>()->getScoreObject(i), example, output);
  return res;
}

void CompositeEvaluator::finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
{
  for (size_t i = 0; i < evaluators.size(); ++i)
    evaluators[i]->finalizeScoreObject(scores.staticCast<CompositeScoreObject>()->getScoreObject(i), function);
}

/*
** ProxyEvaluator
*/
TypePtr ProxyEvaluator::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  implementation = createImplementation(inputVariables);
  if (!implementation)
  {
    context.errorCallback(T("Could not create implementation of Evaluator"));
    return TypePtr();
  }
  return Evaluator::initializeFunction(context, inputVariables, outputName, outputShortName);
}

Variable ProxyEvaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
  {jassert(implementation); return implementation->computeFunction(context, inputs);}

ScoreObjectPtr ProxyEvaluator::createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {jassert(implementation); return implementation->createEmptyScoreObject(context, function);}

bool ProxyEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {jassert(implementation); return implementation->updateScoreObject(context, scores, example, output);}

void ProxyEvaluator::finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
  {jassert(implementation); implementation->finalizeScoreObject(scores, function);}

/*
** CallbackBasedEvaluator
*/
class CallbackBasedEvaluatorCallback : public FunctionCallback
{
public:
  CallbackBasedEvaluatorCallback(const EvaluatorPtr& evaluator, const ScoreObjectPtr& scores)
    : evaluator(evaluator), scores(scores) {}

  EvaluatorPtr evaluator;
  ScoreObjectPtr scores;

  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
  {
    ObjectPtr inputsObject = Object::create(function->getInputsClass());
    for (size_t i = 0; i < inputsObject->getNumVariables(); ++i)
      inputsObject->setVariable(i, inputs[i]);
    evaluator->updateScoreObject(context, scores, inputsObject, output);
  }
};

CallbackBasedEvaluator::CallbackBasedEvaluator(EvaluatorPtr evaluator)
  : evaluator(evaluator), callback(NULL)
{
}

ScoreObjectPtr CallbackBasedEvaluator::createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
{
  ScoreObjectPtr res = evaluator->createEmptyScoreObject(context, function);
  FunctionPtr functionToListen = getFunctionToListen(function);
  functionToListen->addPostCallback(const_cast<CallbackBasedEvaluator* >(this)->callback = new CallbackBasedEvaluatorCallback(evaluator, res));
  return res;
}

bool CallbackBasedEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {return true;}

void CallbackBasedEvaluator::finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
{
  evaluator->finalizeScoreObject(scores, function);
  getFunctionToListen(function)->removePostCallback(callback);
  deleteAndZero(const_cast<CallbackBasedEvaluator* >(this)->callback);
}

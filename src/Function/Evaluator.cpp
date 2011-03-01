/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 10:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/Evaluator.h>
#include <lbcpp/Execution/WorkUnit.h>
using namespace lbcpp;

namespace lbcpp 
{

struct EvaluateExampleWorkUnit : public WorkUnit
{
  EvaluateExampleWorkUnit(const EvaluatorPtr& evaluator, const FunctionPtr& function, const ObjectPtr& example, const ScoreObjectPtr& scores, CriticalSection& lock)
    : evaluator(evaluator), function(function), example(example), scores(scores), lock(lock) {}

  virtual Variable run(ExecutionContext& context)
  {
    Variable output = function->computeWithInputsObject(context, example);
    ScopedLock _(lock);
    evaluator->updateScoreObject(context, scores, example, output);
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
      Variable output = function->computeWithInputsObject(context, example);
      if (!updateScoreObject(context, scores, example, output))
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
      Variable output = function->computeWithInputsObject(context, example);
      if (!updateScoreObject(context, scores, example, output))
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

  ScoreObjectPtr res = createEmptyScoreObject();
  if (false)//context.isMultiThread())
    computeEvaluatorMultiThread(context, function, examples, res);
  else
    computeEvaluatorSingleThread(context, function, examples, res);
  finalizeScoreObject(res);
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
ScoreObjectPtr CompositeEvaluator::createEmptyScoreObject() const
{
  CompositeScoreObjectPtr res = new CompositeScoreObject();
  for (size_t i = 0; i < evaluators.size(); ++i)
    res->addScoreObject(evaluators[i]->createEmptyScoreObject());
  return res;
}

bool CompositeEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
{
  bool res = true;
  for (size_t i = 0; i < evaluators.size(); ++i)
    res &= evaluators[i]->updateScoreObject(context, scores.staticCast<CompositeScoreObject>()->getScoreObject(i), example, output);
  return res;
}

void CompositeEvaluator::finalizeScoreObject(const ScoreObjectPtr& scores) const
{
  for (size_t i = 0; i < evaluators.size(); ++i)
    evaluators[i]->finalizeScoreObject(scores.staticCast<CompositeScoreObject>()->getScoreObject(i));
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

ScoreObjectPtr ProxyEvaluator::createEmptyScoreObject() const
  {jassert(implementation); return implementation->createEmptyScoreObject();}

bool ProxyEvaluator::updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {jassert(implementation); return implementation->updateScoreObject(context, scores, example, output);}

void ProxyEvaluator::finalizeScoreObject(const ScoreObjectPtr& scores) const
  {jassert(implementation); implementation->finalizeScoreObject(scores);}

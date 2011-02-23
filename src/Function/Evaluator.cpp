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
    evaluator->updateScoreObject(scores, example, output);
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

  ObjectVectorPtr objectExamples = examples.dynamicCast<ObjectVector>();
  if (objectExamples)
  {
    // fast version for object vectors
    const std::vector<ObjectPtr>& objects = objectExamples->getObjects();
    for (size_t i = 0; i < n; ++i)
    {
      const ObjectPtr& example = objects[i];
      Variable output = function->computeWithInputsObject(context, example);
      updateScoreObject(scores, example, output);
    }
  }
  else
  { 
    // generic version
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr example = examples->getElement(i).getObject();
      Variable output = function->computeWithInputsObject(context, example);
      updateScoreObject(scores, example, output);
    }
  }
}

Variable Evaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const FunctionPtr& function = inputs[0].getObjectAndCast<Function>();
  const ContainerPtr& examples = inputs[1].getObjectAndCast<Container>();

  ScoreObjectPtr res = createEmptyScoreObject();
  if (context.isMultiThread())
    computeEvaluatorMultiThread(context, function, examples, res);
  else
    computeEvaluatorSingleThread(context, function, examples, res);
  finalizeScoreObject(res);
  return res;
}

/*
** SupervisedEvaluator
*/
void SupervisedEvaluator::updateScoreObject(const ScoreObjectPtr& scores, const ObjectPtr& inputsObject, const Variable& output) const
{
  Variable supervision = inputsObject->getVariable(inputsObject->getNumVariables() - 1);
  if (supervision.exists())
    addPrediction(defaultExecutionContext(), output, supervision, scores);
}

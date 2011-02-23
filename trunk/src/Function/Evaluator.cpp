/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 10:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/Evaluator.h>

using namespace lbcpp;

/*
** OutputEvaluator
*/
Variable OutputEvaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const ContainerPtr& outputs = inputs[1].getObjectAndCast<Container>();
  size_t n = outputs->getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (!outputs->getElement(i).exists())
    {
      context.errorCallback(T("Evaluator::computeFunction"), T("Cannot evaluate a missing output"));
      return ScoreObjectPtr();
    }

  return computeOutputEvaluator(context, outputs);
}

/*
** SupervisedEvaluator
*/
Variable SupervisedEvaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const ContainerPtr& inputObjects = inputs[0].getObjectAndCast<Container>();
  const ContainerPtr& predictions = inputs[1].getObjectAndCast<Container>();
  
  size_t n = inputObjects->getNumElements();
  if (predictions->getNumElements() != n)
  {
    context.errorCallback(T("Evaluator::computeFunction"), T("The number of predicted elements is different than the number of correct elements"));
    return Variable();
  }
  
  ScoreObjectPtr res = createEmptyScoreObject();
  for (size_t i = 0; i < n; ++i)
  {
    Variable predicted = predictions->getElement(i);
    ObjectPtr input = inputObjects->getElement(i).getObject();
    Variable correct = input->getVariable(input->getNumVariables() - 1);
    if (!correct.exists())
      continue;
    if (!predicted.exists())
    {
      context.errorCallback(T("Evaluator::computeFunction"), T("Cannot evaluate a missing predicted value"));
      return ScoreObjectPtr();
    }
    addPrediction(context, predicted, correct, res);
  }
  finalizeScoreObject(res);
  return res;
}

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
** Evaluator
*/
Variable Evaluator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ContainerPtr predictedElements = inputs[0].getObjectAndCast<Container>();
  ContainerPtr correctElements = inputs[1].getObjectAndCast<Container>();
  
  size_t n = correctElements->getNumElements();
  if (predictedElements->getNumElements() != n)
  {
    context.errorCallback(T("Evaluator::computeFunction"), T("The number of predicted elements is different than the number of correct elements"));
    return Variable();
  }
  
  ScoreObjectPtr res = createEmptyScoreObject();
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& predicted = predictedElements->getElement(i);
    const Variable& correct = correctElements->getElement(i);
    if (!correct.exists())
      continue;
    if (!predicted.exists())
    {
      context.errorCallback(T("Evaluator::computeFunction"), T("Cannot evaluate a missing predicted value."));
      return ScoreObjectPtr();
    }
    addPrediction(context, predicted, correct, res);
  }
  return res;
}

/*
** CompositeScoreObject
*/
void CompositeScoreObject::getScores(std::vector< std::pair<String, double> >& res) const
{
  for (size_t i = 0; i < scores.size(); ++i)
  {
    std::vector<std::pair<String, double> > childScores;
    scores[i]->getScores(childScores);
    for (size_t j = 0; j < scores.size(); ++j)
      res.push_back(std::make_pair(scores[i]->getName() + T("[") + childScores[j].first + T("]"), childScores[j].second));
  }
}

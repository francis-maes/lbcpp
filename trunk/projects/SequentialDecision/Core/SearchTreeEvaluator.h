/*-----------------------------------------.---------------------------------.
| Filename: SearchTreeEvaluator.h          | SearchSpace Evaluator           |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 16:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_

# include "SearchTree.h"
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class SearchTreeScoreObject : public ScoreObject
{
public:
  SearchTreeScoreObject()
    : bestReturn(new ScalarVariableStatistics()), numOpenedNodes(new ScalarVariableMean()) {}

  virtual double getScoreToMinimize() const
    {return -bestReturn->getMean();}

  void add(const SearchTreePtr& searchTree)
  {
    bestReturn->push(searchTree->getBestReturn());
    numOpenedNodes->push((double)searchTree->getNumOpenedNodes());
  }

protected:
  friend class SearchTreeScoreObjectClass;

  ScalarVariableStatisticsPtr bestReturn;
  ScalarVariableMeanPtr numOpenedNodes;
};

typedef ReferenceCountedObjectPtr<SearchTreeScoreObject> SearchTreeScoreObjectPtr;

class SearchTreeEvaluator : public Evaluator
{
public:
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new SearchTreeScoreObject();}

  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& inputsObject, const Variable& output) const
    {scores.staticCast<SearchTreeScoreObject>()->add(output.getObjectAndCast<SearchTree>()); return true;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_

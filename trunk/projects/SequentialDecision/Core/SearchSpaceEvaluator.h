/*-----------------------------------------.---------------------------------.
| Filename: SearchSpaceEvaluator.h         | SearchSpace Evaluator           |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 16:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_

# include "../Core/SearchSpace.h"
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class SearchSpaceScoreObject : public ScoreObject
{
public:
  SearchSpaceScoreObject()
    : bestReturn(new ScalarVariableStatistics()), numOpenedNodes(new ScalarVariableMean()) {}

  virtual double getScoreToMinimize() const
    {return -bestReturn->getMean();}

  void add(const SortedSearchSpacePtr& searchSpace)
  {
    bestReturn->push(searchSpace->getBestReturn());
    numOpenedNodes->push((double)searchSpace->getNumOpenedNodes());
  }

protected:
  friend class SearchSpaceScoreObjectClass;

  ScalarVariableStatisticsPtr bestReturn;
  ScalarVariableMeanPtr numOpenedNodes;
};

typedef ReferenceCountedObjectPtr<SearchSpaceScoreObject> SearchSpaceScoreObjectPtr;

class SearchSpaceEvaluator : public OutputEvaluator
{
public:
  virtual TypePtr getRequiredOutputType() const
    {return sortedSearchSpaceClass;}

  virtual ScoreObjectPtr computeOutputEvaluator(ExecutionContext& context, const ContainerPtr& outputs) const
  {
    SearchSpaceScoreObjectPtr res = new SearchSpaceScoreObject();
    size_t n = outputs->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      SortedSearchSpacePtr searchSpace = outputs->getElement(i).getObjectAndCast<SortedSearchSpace>();
      res->add(searchSpace);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_EVALUATOR_H_

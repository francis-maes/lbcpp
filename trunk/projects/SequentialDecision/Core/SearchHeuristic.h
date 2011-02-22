/*-----------------------------------------.---------------------------------.
| Filename: SearchHeuristic.h              | Search Heuristic                |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_HEURISTIC_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_HEURISTIC_H_

# include "SearchSpace.h"

namespace lbcpp
{

class MinDepthSearchHeuristic : public SimpleSearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return -(double)node->getDepth();}
};

class OptimisticPlanningSearchHeuristic : public SimpleSearchHeuristic
{
public:
  OptimisticPlanningSearchHeuristic(double discountFactor = 0.9) : discountFactor(discountFactor)
    {jassert(discountFactor > 0.0 && discountFactor < 1.0);}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return node->getCurrentReturn() + pow(discountFactor, (double)node->getDepth()) / (1.0 - discountFactor);}

private:
  friend class OptimisticPlanningSearchHeuristicClass;

  double discountFactor;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

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

class GreedySearchHeuristic : public SimpleSearchHeuristic
{
public:
  GreedySearchHeuristic(double discount = 1.0)
    : discount(discount) {}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return node->getReward() * pow(discount, (double)node->getDepth() - 1.0);}

  virtual String toShortString() const
    {return T("greedy(") + String(discount) + T(")");}

protected:
  friend class GreedySearchHeuristicClass;

  double discount;
};

class MaxReturnSearchHeuristic : public SimpleSearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return node->getCurrentReturn();}
};

class MinDepthSearchHeuristic : public SimpleSearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return -log10(1.0 + (double)node->getDepth());}
};

class OptimisticPlanningSearchHeuristic : public SimpleSearchHeuristic
{
public:
  OptimisticPlanningSearchHeuristic(double discount = 0.9) : discount(discount)
    {jassert(discount > 0.0 && discount < 1.0);}

  virtual String toShortString() const
    {return T("optimistic(") + String(discount) + T(")");}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return node->getCurrentReturn() + pow(discount, (double)node->getDepth()) / (1.0 - discount);}

private:
  friend class OptimisticPlanningSearchHeuristicClass;

  double discount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

/*-----------------------------------------.---------------------------------.
| Filename: SearchSpace.h                  | Search Space                    |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_H_

# include "SequentialDecisionProblem.h"

namespace lbcpp
{

class SearchSpaceNode;
typedef ReferenceCountedObjectPtr<SearchSpaceNode> SearchSpaceNodePtr;

class SearchSpaceNode : public Object
{
public:
  SearchSpaceNode(const SearchSpaceNodePtr& parentNode, size_t parentIndex, const Variable& action);
  SearchSpaceNode(const Variable& initialState = Variable());

  const Variable& getState() const
    {return state;}

  size_t getDepth() const
    {return depth;}
  
  double getCurrentReturn() const
    {return currentReturn;}

  void openNode(const SequentialDecisionSystemPtr& system, const SearchSpaceNodePtr& parentNode, double discount);

  void setChildrenIndices(size_t begin, size_t end)
    {childrenBeginIndex = (int)begin; childrenEndIndex = (int)end;}

  int getParentIndex() const
    {return parentIndex;}

protected:
  friend class SearchSpaceNodeClass;

  Variable state;
  size_t depth;

  Variable previousAction;
  double reward; // received when entering state
  double currentReturn; // received from the beginning until entering state

  int parentIndex;
  int childrenBeginIndex;
  int childrenEndIndex;
};

extern ClassPtr searchSpaceNodeClass;

class SearchHeuristic : public SimpleUnaryFunction
{
public:
  SearchHeuristic() : SimpleUnaryFunction(searchSpaceNodeClass, doubleType) {}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeHeuristic(input.getObjectAndCast<SearchSpaceNode>());}
};

typedef ReferenceCountedObjectPtr<SearchHeuristic> SearchHeuristicPtr;

class SortedSearchSpace : public Object
{
public:
  SortedSearchSpace(SequentialDecisionSystemPtr system, SearchHeuristicPtr heuristic, double discount, const Variable& initialState);
  SortedSearchSpace() {}

  // returns the current return
  double exploreBestNode(ExecutionContext& context);

  void addCandidate(SearchSpaceNodePtr node);
  SearchSpaceNodePtr popBestCandidate(size_t& nodeIndex);

private:
  SequentialDecisionSystemPtr system;
  SearchHeuristicPtr heuristic;
  double discount;
  std::vector<SearchSpaceNodePtr> nodes;
  std::multimap<double, size_t> candidates;
};

typedef ReferenceCountedObjectPtr<SortedSearchSpace> SortedSearchSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_H_

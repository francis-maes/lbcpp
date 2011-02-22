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

/*
** SearchSpaceNode
*/
class SearchSpaceNode;
typedef ReferenceCountedObjectPtr<SearchSpaceNode> SearchSpaceNodePtr;
typedef std::vector<SearchSpaceNodePtr> SearchSpaceNodeVector;

class SearchSpaceNode : public Object
{
public:
  SearchSpaceNode(const SearchSpaceNodeVector& allNodes, const SequentialDecisionProblemPtr& problem, size_t parentIndex, const Variable& action, double discount);
  SearchSpaceNode(const SearchSpaceNodeVector& allNodes, const Variable& initialState);
  SearchSpaceNode() : allNodes(*(const SearchSpaceNodeVector* )0) {}

  /*
  ** Properties
  ** Root Node: State
  ** Other Nodes: (PreviousAction, PreviousReward, State)
  */
  const Variable& getPreviousAction() const
    {return previousAction;}

  double getReward() const
    {return reward;}

  double getCurrentReturn() const
    {return currentReturn;}

  const Variable& getState() const
    {return state;}

  size_t getDepth() const
    {return depth;}

  /*
  ** Parent/Children Indices
  */
  int getParentIndex() const
    {return parentIndex;}

  void setChildrenIndices(size_t begin, size_t end)
    {childrenBeginIndex = (int)begin; childrenEndIndex = (int)end;}

protected:
  friend class SearchSpaceNodeClass;

  const SearchSpaceNodeVector& allNodes;

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

/*
** SearchHeuristic
*/
class SimpleSearchHeuristic : public SimpleUnaryFunction
{
public:
  SimpleSearchHeuristic() : SimpleUnaryFunction(searchSpaceNodeClass, doubleType) {}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeHeuristic(input.getObjectAndCast<SearchSpaceNode>());}
};

FunctionPtr minDepthSearchHeuristic();
FunctionPtr optimisticPlanningSearchHeuristic(double discountFactor);

/*
** SortedSearchSpace
*/
class SortedSearchSpace : public Object
{
public:
  SortedSearchSpace(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, double discount, const Variable& initialState);
  SortedSearchSpace() {}

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  // returns the current return
  double exploreBestNode(ExecutionContext& context);

  /*
  ** Candidates
  */
  void addCandidate(SearchSpaceNodePtr node);
  SearchSpaceNodePtr popBestCandidate(size_t& nodeIndex);

  /*
  ** Opened nodes
  */
  size_t getNumOpenedNodes() const
    {return openedNodes.size();}

  SearchSpaceNodePtr getOpenedNode(size_t index) const
    {return nodes[openedNodes[index]];}

private:
  SequentialDecisionProblemPtr problem;
  FunctionPtr heuristic;
  double discount;
  std::vector<SearchSpaceNodePtr> nodes;
  std::multimap<double, size_t> candidates;
  std::vector<size_t> openedNodes;
};

typedef ReferenceCountedObjectPtr<SortedSearchSpace> SortedSearchSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_H_

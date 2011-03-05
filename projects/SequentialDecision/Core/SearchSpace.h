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
  SearchSpaceNode(const SearchSpaceNodeVector& allNodes, size_t nodeIndex, const Variable& initialState = Variable());
  SearchSpaceNode() : nodeIndex(0), allNodes(*(const SearchSpaceNodeVector* )0) {}

  void open(const SequentialDecisionProblemPtr& problem, size_t parentIndex, const Variable& action, double discount);

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
  ** Heuristic
  */
  double getHeuristicScore() const
    {return heuristicScore;}

  void computeHeuristicScore(const FunctionPtr& heuristic);

  /*
  ** Parent
  */
  size_t getNodeIndex() const
    {return nodeIndex;}

  int getParentIndex() const
    {return parentIndex;}

  SearchSpaceNodePtr getParentNode() const
    {return parentIndex >= 0 ? allNodes[parentIndex] : SearchSpaceNodePtr();}

  /*
  ** Children
  */
  void setChildrenIndices(size_t begin, size_t end)
    {childBeginIndex = (int)begin; childEndIndex = (int)end;}

  int getChildBeginIndex() const
    {return childBeginIndex;}

  int getChildEndIndex() const
    {return childEndIndex;}

  bool isExplored() const
    {return childBeginIndex >= 0;}

  /*
  ** Best Return
  */
  void updateBestReturn(double newReturn, SearchSpaceNodePtr childNode);

  double getBestReturn() const
    {return bestReturn;}

  double getBestReturnWithoutChild(SearchSpaceNodePtr childNode) const;

  const Variable& getBestAction() const
    {static Variable empty; return bestChildNode ? bestChildNode->getPreviousAction() : empty;}

  const SearchSpaceNodePtr& getBestChildNode() const
    {return bestChildNode;}

protected:
  friend class SearchSpaceNodeClass;

  const SearchSpaceNodeVector& allNodes;
  size_t nodeIndex;

  Variable state;
  size_t depth;

  Variable previousAction;
  double reward; // received when entering state
  double currentReturn; // received from the beginning until entering state

  int parentIndex;
  int childBeginIndex;
  int childEndIndex;

  SearchSpaceNodePtr bestChildNode;
  double bestReturn;

  double heuristicScore;
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

FunctionPtr greedySearchHeuristic(double discount = 1.0);
FunctionPtr maxReturnSearchHeuristic();
FunctionPtr minDepthSearchHeuristic();
FunctionPtr optimisticPlanningSearchHeuristic(double discount);

/*
** SortedSearchSpace
*/
class SortedSearchSpace : public Object
{
public:
  SortedSearchSpace(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, double discount, size_t beamSize, const Variable& initialState);
  SortedSearchSpace() : beamSize(0) {}

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  void exploreBestNode(ExecutionContext& context);
  void exploreRandomNode(ExecutionContext& context);
  void exploreNode(ExecutionContext& context, size_t nodeIndex);


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

  size_t getOpenedNodeIndex(size_t index) const
    {return openedNodes[index];}

  /*
  ** All nodes
  */
  size_t getNumNodes() const
    {return nodes.size();}

  SearchSpaceNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  SearchSpaceNodePtr getRootNode() const
    {jassert(nodes.size()); return nodes[0];}

  double getBestReturn() const
    {return getRootNode()->getBestReturn();}

  const Variable& getBestAction() const
    {return getRootNode()->getBestAction();}

private:
  SequentialDecisionProblemPtr problem;
  FunctionPtr heuristic;
  double discount;
  std::vector<SearchSpaceNodePtr> nodes;
  

  size_t beamSize;
  std::multimap<double, size_t> candidates;
  double worstScore;

  std::vector<size_t> openedNodes;
};

typedef ReferenceCountedObjectPtr<SortedSearchSpace> SortedSearchSpacePtr;

extern ClassPtr sortedSearchSpaceClass;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_SPACE_H_

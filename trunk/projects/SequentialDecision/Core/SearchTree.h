/*-----------------------------------------.---------------------------------.
| Filename: SearchTree.h                   | Search Tree                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_H_

# include "SearchTreeNode.h"

namespace lbcpp
{

/*
** SearchHeuristic
*/
class SimpleSearchHeuristic : public SimpleUnaryFunction
{
public:
  SimpleSearchHeuristic() : SimpleUnaryFunction(searchTreeNodeClass, doubleType) {}

  virtual double computeHeuristic(const SearchTreeNodePtr& node) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeHeuristic(input.getObjectAndCast<SearchTreeNode>());}
};

FunctionPtr greedySearchHeuristic(double discount = 1.0);
FunctionPtr maxReturnSearchHeuristic();
FunctionPtr minDepthSearchHeuristic();
FunctionPtr optimisticPlanningSearchHeuristic(double discount);
FunctionPtr linearInterpolatedSearchHeuristic(FunctionPtr heuristic1, FunctionPtr heuristic2, double k);

/*
** SearchTree
*/
class SearchTree;
typedef ReferenceCountedObjectPtr<SearchTree> SearchTreePtr;

class SearchTreeCallback
{
public:
  virtual ~SearchTreeCallback() {}

  virtual void candidateAdded(ExecutionContext& context, const SearchTreePtr& searchTree, size_t nodeIndex) = 0;
};

typedef SearchTreeCallback* SearchTreeCallbackPtr;

class SearchTree : public Object
{
public:
  SearchTree(SequentialDecisionProblemPtr problem, const Variable& initialState, size_t maxOpenedNodes);
  SearchTree() {}

  /*
  ** Explore
  */
  void exploreNode(ExecutionContext& context, size_t nodeIndex);

  /*
  ** Opened nodes
  */
  size_t getNumOpenedNodes() const
    {return openedNodes.size();}

  SearchTreeNodePtr getOpenedNode(size_t index) const
    {return nodes[openedNodes[index]];}

  size_t getOpenedNodeIndex(size_t index) const
    {return openedNodes[index];}

  /*
  ** All nodes
  */
  size_t getNumNodes() const
    {return nodes.size();}

  SearchTreeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  SearchTreeNodePtr getRootNode() const
    {jassert(nodes.size()); return nodes[0];}

  double getBestReturn() const
    {return getRootNode()->getBestReturn();}

  const Variable& getBestAction() const
    {return getRootNode()->getBestAction();}

  /*
  ** Callbacks
  */
  void addCallback(SearchTreeCallbackPtr callback)
    {callbacks.push_back(callback);}

  void removeCallback(SearchTreeCallbackPtr callback);
  void clearCallbacks()
    {callbacks.clear();}

protected:
  SequentialDecisionProblemPtr problem;
  std::vector<SearchTreeNodePtr> nodes;
  std::vector<size_t> openedNodes;
  std::vector<SearchTreeCallbackPtr> callbacks;

  void addCandidate(ExecutionContext& context, SearchTreeNodePtr node);
};

extern ClassPtr searchTreeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_H_

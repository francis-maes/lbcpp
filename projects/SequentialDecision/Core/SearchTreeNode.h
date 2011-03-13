/*-----------------------------------------.---------------------------------.
| Filename: SearchTreeNode.h               | Search Tree Node                |
| Author  : Francis Maes                   |                                 |
| Started : 05/03/2011 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_

# include "DecisionProblem.h"

namespace lbcpp
{

/*
** SearchTreeNode
*/
class SearchTreeNode;
typedef ReferenceCountedObjectPtr<SearchTreeNode> SearchTreeNodePtr;
typedef std::vector<SearchTreeNodePtr> SearchTreeNodeVector;

class SearchTreeNode : public Object
{
public:
  SearchTreeNode(const SearchTreeNodeVector& allNodes, size_t nodeIndex, size_t nodeUid, const Variable& initialState = Variable());
  SearchTreeNode() : nodeIndex(0), allNodes(*(const SearchTreeNodeVector* )0) {}

  void open(const DecisionProblemPtr& problem, size_t parentIndex, const Variable& action);

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
  ** Parent
  */
  size_t getNodeIndex() const
    {return nodeIndex;}

  size_t getNodeUid() const
    {return nodeUid;}

  int getParentIndex() const
    {return parentIndex;}

  SearchTreeNodePtr getParentNode() const
    {return parentIndex >= 0 ? allNodes[parentIndex] : SearchTreeNodePtr();}

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
  void updateBestReturn(double newReturn, SearchTreeNodePtr childNode);

  double getBestReturn() const
    {return bestReturn;}

  double getBestReturnWithoutChild(SearchTreeNodePtr childNode) const;

  const Variable& getBestAction() const
    {static Variable empty; return bestChildNode ? bestChildNode->getPreviousAction() : empty;}

  const SearchTreeNodePtr& getBestChildNode() const
    {return bestChildNode;}

protected:
  friend class SearchTreeNodeClass;

  const SearchTreeNodeVector& allNodes;
  size_t nodeIndex;
  size_t nodeUid;

  Variable state;
  size_t depth;

  Variable previousAction;
  double reward; // received when entering state
  double currentReturn; // received from the beginning until entering state

  int parentIndex;
  int childBeginIndex;
  int childEndIndex;

  SearchTreeNodePtr bestChildNode;
  double bestReturn;
};

extern ClassPtr searchTreeNodeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_

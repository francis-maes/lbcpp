/*-----------------------------------------.---------------------------------.
| Filename: SearchTreeNode.h               | Search Tree Node                |
| Author  : Francis Maes                   |                                 |
| Started : 05/03/2011 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

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
  SearchTreeNode(ClassPtr thisClass, const SearchTreeNodeVector& allNodes, size_t nodeIndex, size_t nodeUid, const DecisionProblemStatePtr& initialState = DecisionProblemStatePtr());
  SearchTreeNode();

  ClassPtr getStateClass() const
    {return (ClassPtr)getClass()->getTemplateArgument(0);}

  TypePtr getActionType() const
    {return getClass()->getTemplateArgument(1);}

  void open(ExecutionContext& context, const DecisionProblemPtr& problem, size_t parentIndex, const Variable& action);

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

  const DecisionProblemStatePtr& getState() const
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

  Variable getBestAction() const
    {return bestChildNode ? bestChildNode->getPreviousAction() : Variable();}

  const SearchTreeNodePtr& getBestChildNode() const
    {return bestChildNode;}

protected:
  friend class SearchTreeNodeClass;

  const SearchTreeNodeVector& allNodes;
  size_t nodeIndex;
  size_t nodeUid;

  DecisionProblemStatePtr state;
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

extern ClassPtr searchTreeNodeClass(TypePtr stateClass = decisionProblemStateClass, TypePtr actionType = variableType);

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_TREE_NODE_H_

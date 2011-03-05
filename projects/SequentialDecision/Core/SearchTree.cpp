/*-----------------------------------------.---------------------------------.
| Filename: SearchTree.cpp                 | Search Tree                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "SearchTree.h"
using namespace lbcpp;

/*
** SearchTreeNode
*/
SearchTreeNode::SearchTreeNode(const SearchTreeNodeVector& allNodes, size_t nodeIndex, size_t nodeUid, const Variable& initialState)
  : allNodes(allNodes), nodeIndex(nodeIndex), nodeUid(nodeUid), state(initialState), depth(0), reward(0.0), currentReturn(0.0),
    parentIndex(-1), childBeginIndex(-1), childEndIndex(-1), bestReturn(0.0)
{
}

void SearchTreeNode::open(const SequentialDecisionProblemPtr& problem, size_t parentIndex, const Variable& action)
{
  this->parentIndex = parentIndex;
  this->previousAction = action;

  const SearchTreeNodePtr& parentNode = allNodes[parentIndex];
  jassert(parentNode);
  depth = parentNode->depth + 1;
  reward = problem->computeReward(parentNode->state, action);
  bestReturn = currentReturn = parentNode->currentReturn + reward * pow(problem->getDiscount(), (double)parentNode->depth);
  parentNode->updateBestReturn(currentReturn, refCountedPointerFromThis(this));
  state = problem->computeTransition(parentNode->state, action);
  jassert(state.exists());
}

void SearchTreeNode::updateBestReturn(double newReturn, SearchTreeNodePtr childNode)
{
  if (newReturn > bestReturn)
  {
    bestReturn = newReturn;
    bestChildNode = childNode;
    if (parentIndex >= 0)
      allNodes[parentIndex]->updateBestReturn(bestReturn, refCountedPointerFromThis(this));
  }
}

double SearchTreeNode::getBestReturnWithoutChild(SearchTreeNodePtr childNode) const
{
  if (childNode->getBestReturn() < bestReturn)
    return bestReturn;
  jassert(childBeginIndex >= 0);
  double best = 0.0;
  for (int i = childBeginIndex; i < childEndIndex; ++i)
    if (allNodes[i] != childNode)
    {
      double childBestReturn = allNodes[i]->getBestReturn();
      if (childBestReturn > best)
        best = childBestReturn;
    }
  return best;
}

/*
** SearchTree
*/
SearchTree::SearchTree(SequentialDecisionProblemPtr problem, const Variable& initialState, size_t maxOpenedNodes)
  : problem(problem)
{
  nodes.reserve(2 * maxOpenedNodes + 1);
  openedNodes.reserve(maxOpenedNodes);
  addCandidate(defaultExecutionContext(), new SearchTreeNode(nodes, 0, 1, initialState));
}

void SearchTree::exploreNode(ExecutionContext& context, size_t nodeIndex)
{
  SearchTreeNodePtr node = getNode(nodeIndex);
  jassert(node);
  openedNodes.push_back(nodeIndex);

  std::vector<Variable> actions;
  problem->getAvailableActions(node->getState(), actions);
  size_t firstChildIndex = nodes.size();
  node->setChildrenIndices(firstChildIndex, firstChildIndex + actions.size());
  for (size_t i = 0; i < actions.size(); ++i)
  {
    SearchTreeNodePtr childNode = new SearchTreeNode(nodes, firstChildIndex + i, node->getNodeUid() * actions.size() + i);
    childNode->open(problem, nodeIndex, actions[i]);
    addCandidate(context, childNode);
  }
}

void SearchTree::addCandidate(ExecutionContext& context, SearchTreeNodePtr node)
{
  size_t index = nodes.size();
  nodes.push_back(node);
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->candidateAdded(context, this, index);
}

void SearchTree::removeCallback(SearchTreeCallbackPtr callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      return;
    }
}

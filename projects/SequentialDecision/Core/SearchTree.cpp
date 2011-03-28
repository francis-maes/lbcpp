/*-----------------------------------------.---------------------------------.
| Filename: SearchTree.cpp                 | Search Tree                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "SearchTree.h"
#include "SearchPolicy.h"
using namespace lbcpp;

/*
** SearchTreeNode
*/
SearchTreeNode::SearchTreeNode(const SearchTreeNodeVector& allNodes, size_t nodeIndex, size_t nodeUid, const DecisionProblemStatePtr& initialState)
  : allNodes(allNodes), nodeIndex(nodeIndex), nodeUid(nodeUid), depth(0), reward(0.0), currentReturn(0.0),
    parentIndex(-1), childBeginIndex(-1), childEndIndex(-1), bestReturn(0.0)
{
  if (initialState)
    state = initialState->cloneAndCast<DecisionProblemState>();
}

void SearchTreeNode::open(const DecisionProblemPtr& problem, size_t parentIndex, const Variable& action)
{
  this->parentIndex = parentIndex;
  this->previousAction = action;

  const SearchTreeNodePtr& parentNode = allNodes[parentIndex];
  jassert(parentNode);
  depth = parentNode->depth + 1;

  state = parentNode->state->cloneAndCast<DecisionProblemState>();
  state->performTransition(action, reward);

  bestReturn = currentReturn = parentNode->currentReturn + reward * pow(problem->getDiscount(), (double)parentNode->depth);
  parentNode->updateBestReturn(currentReturn, refCountedPointerFromThis(this));
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
SearchTree::SearchTree(DecisionProblemPtr problem, const DecisionProblemStatePtr& initialState, size_t maxOpenedNodes)
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

  ContainerPtr actions = node->getState()->getAvailableActions();
  size_t n = actions->getNumElements();

  size_t firstChildIndex = nodes.size();
  node->setChildrenIndices(firstChildIndex, firstChildIndex + n);
  for (size_t i = 0; i < n; ++i)
  {
    SearchTreeNodePtr childNode = new SearchTreeNode(nodes, firstChildIndex + i, node->getNodeUid() * n + i);
    childNode->open(problem, nodeIndex, actions->getElement(i));
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

void SearchTree::doSearchEpisode(ExecutionContext& context, const PolicyPtr& policy, size_t maxSearchNodes)
{
  double lastReward = 0.0;
  double bestReturn = 0.0;

  SearchTreePtr pthis = refCountedPointerFromThis(this);

  for (size_t i = 0; i < maxSearchNodes; ++i)
  {
    ContainerPtr actions;
    Variable selectedNode;
    if (i == 0)
      selectedNode = policy->policyStart(context, pthis, actions);
    else
      selectedNode = policy->policyStep(context, lastReward, pthis, actions);
    if (!selectedNode.exists())
    {
      context.errorCallback(T("No selected node"));
      break;
    }

    exploreNode(context, (size_t)selectedNode.getInteger());
    double newBestReturn = getBestReturn();
    lastReward = newBestReturn - bestReturn;
    jassert(lastReward >= 0.0);
    bestReturn = newBestReturn;
  }
  policy->policyEnd(context, lastReward, pthis);
}
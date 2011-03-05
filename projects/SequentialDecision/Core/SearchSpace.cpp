/*-----------------------------------------.---------------------------------.
| Filename: SearchSpace.cpp                | Search Space                    |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "SearchSpace.h"
using namespace lbcpp;

/*
** SearchSpaceNode
*/
SearchSpaceNode::SearchSpaceNode(const SearchSpaceNodeVector& allNodes, size_t nodeIndex, const Variable& initialState)
  : allNodes(allNodes), nodeIndex(nodeIndex), state(initialState), depth(0), reward(0.0), currentReturn(0.0),
    parentIndex(-1), childBeginIndex(-1), childEndIndex(-1), bestReturn(0.0), heuristicScore(0.0)
{
}

void SearchSpaceNode::open(const SequentialDecisionProblemPtr& problem, size_t parentIndex, const Variable& action, double discount)
{
  this->parentIndex = parentIndex;
  this->previousAction = action;

  const SearchSpaceNodePtr& parentNode = allNodes[parentIndex];
  jassert(parentNode);
  depth = parentNode->depth + 1;
  reward = problem->computeReward(parentNode->state, action);
  bestReturn = currentReturn = parentNode->currentReturn + reward * pow(discount, (double)parentNode->depth);
  parentNode->updateBestReturn(currentReturn, refCountedPointerFromThis(this));
  state = problem->computeTransition(parentNode->state, action);
  jassert(state.exists());
}

void SearchSpaceNode::computeHeuristicScore(const FunctionPtr& heuristic)
  {heuristicScore = heuristic->compute(defaultExecutionContext(), Variable(this, searchSpaceNodeClass)).getDouble();}

void SearchSpaceNode::updateBestReturn(double newReturn, SearchSpaceNodePtr childNode)
{
  if (newReturn > bestReturn)
  {
    bestReturn = newReturn;
    bestChildNode = childNode;
    if (parentIndex >= 0)
      allNodes[parentIndex]->updateBestReturn(bestReturn, refCountedPointerFromThis(this));
  }
}

double SearchSpaceNode::getBestReturnWithoutChild(SearchSpaceNodePtr childNode) const
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
** SortedSearchSpace
*/
SortedSearchSpace::SortedSearchSpace(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, double discount, size_t beamSize, const Variable& initialState)
  : problem(problem), heuristic(heuristic), discount(discount), beamSize(beamSize), worstScore(DBL_MAX)
{
  nodes.reserve(beamSize);
  addCandidate(new SearchSpaceNode(nodes, 0, initialState));
}

void SortedSearchSpace::exploreNode(ExecutionContext& context, size_t nodeIndex)
{
  SearchSpaceNodePtr node = getNode(nodeIndex);
  jassert(node);

  std::vector<Variable> actions;
  problem->getAvailableActions(node->getState(), actions);
  size_t firstChildIndex = nodes.size();
  node->setChildrenIndices(firstChildIndex, firstChildIndex + actions.size());
  for (size_t i = 0; i < actions.size(); ++i)
  {
    SearchSpaceNodePtr node = new SearchSpaceNode(nodes, firstChildIndex + i);
    node->open(problem, nodeIndex, actions[i], discount);
    addCandidate(node);
  }
}

// returns the current return
void SortedSearchSpace::exploreBestNode(ExecutionContext& context)
{
  if (candidates.empty())
  {
    context.errorCallback(T("No more candidates to explore"));
    return;
  }

  size_t nodeIndex;
  popBestCandidate(nodeIndex);
  exploreNode(context, nodeIndex);
}

void SortedSearchSpace::exploreRandomNode(ExecutionContext& context)
{
  if (candidates.empty())
  {
    context.errorCallback(T("No more candidates to explore"));
    return;
  }
  size_t nodeIndex;
  while (true)
  {
    nodeIndex = RandomGenerator::getInstance()->sampleSize(nodes.size());
    if (!nodes[nodeIndex]->isExplored())
      break;
  }
  exploreNode(context, nodeIndex);
}

void SortedSearchSpace::addCandidate(SearchSpaceNodePtr node)
{
  size_t index = nodes.size();
  nodes.push_back(node);

  node->computeHeuristicScore(heuristic);
  double scoreToMinimize = -node->getHeuristicScore();
  if (candidates.size() >= beamSize && scoreToMinimize > worstScore)
    return; // score is not good enough

  candidates.insert(std::make_pair(scoreToMinimize, index));
  if (candidates.size() > beamSize)
  {
    jassert(candidates.size() == beamSize + 1);
    candidates.erase(--candidates.end());
    worstScore = candidates.rbegin()->first;
  }
}

SearchSpaceNodePtr SortedSearchSpace::popBestCandidate(size_t& nodeIndex)
{
  nodeIndex = candidates.begin()->second;
  candidates.erase(candidates.begin());
  openedNodes.push_back(nodeIndex);
  return nodes[nodeIndex];
}

/*-----------------------------------------.---------------------------------.
| Filename: SearchSpace.cpp                | Search Space                    |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "SearchSpace.h"
using namespace lbcpp;

/*
** SearchSpaceNode
*/
SearchSpaceNode::SearchSpaceNode(const Variable& initialState)
  : state(initialState), depth(0), reward(0.0), currentReturn(0.0),
    parentIndex(-1), childrenBeginIndex(-1), childrenEndIndex(-1)
{
}

SearchSpaceNode::SearchSpaceNode(const SearchSpaceNodePtr& parentNode, size_t parentIndex, const Variable& action)
  : depth(parentNode->depth + 1), previousAction(action), reward(0.0), currentReturn(parentNode->currentReturn),
    parentIndex((int)parentIndex), childrenBeginIndex(-1), childrenEndIndex(-1)
{
}

void SearchSpaceNode::openNode(const SequentialDecisionProblemPtr& system, const SearchSpaceNodePtr& parentNode, double discount)
{
  if (!state.exists())
  {
    jassert(parentNode && depth >= 1);

    state = system->computeTransition(parentNode->state, previousAction);
    reward = system->computeReward(parentNode->state, previousAction, state);
    jassert(state.exists());
    if (reward)
      currentReturn += reward * pow(discount, (double)(depth - 1));
  }
}


/*
** SortedSearchSpace
*/
SortedSearchSpace::SortedSearchSpace(SequentialDecisionProblemPtr system, SearchHeuristicPtr heuristic, double discount, const Variable& initialState)
  : system(system), heuristic(heuristic), discount(discount)
{
  addCandidate(new SearchSpaceNode(initialState));
}

// returns the current return
double SortedSearchSpace::exploreBestNode(ExecutionContext& context)
{
  if (candidates.empty())
  {
    context.errorCallback(T("No more candidates to explore"));
    return 0.0;
  }

  size_t nodeIndex;
  SearchSpaceNodePtr node = popBestCandidate(nodeIndex);
  jassert(node);
  int parentIndex = node->getParentIndex();
  SearchSpaceNodePtr parentNode = (parentIndex >= 0 ? nodes[parentIndex] : SearchSpaceNodePtr());
  
  node->openNode(system, parentNode, discount);

  std::vector<Variable> actions;
  system->getAvailableActions(node->getState(), actions);
  size_t firstChildIndex = nodes.size();
  node->setChildrenIndices(firstChildIndex, firstChildIndex + actions.size());

  for (size_t i = 0; i < actions.size(); ++i)
    addCandidate(new SearchSpaceNode(node, nodeIndex, actions[i]));

  return node->getCurrentReturn();
}

void SortedSearchSpace::addCandidate(SearchSpaceNodePtr node)
{
  size_t index = nodes.size();
  nodes.push_back(node);
  candidates.insert(std::make_pair(-heuristic->computeHeuristic(node), index));
}

SearchSpaceNodePtr SortedSearchSpace::popBestCandidate(size_t& nodeIndex)
{
  nodeIndex = candidates.begin()->second;
  candidates.erase(candidates.begin());
  return nodes[nodeIndex];
}

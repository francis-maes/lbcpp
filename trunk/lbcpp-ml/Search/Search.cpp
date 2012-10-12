/*-----------------------------------------.---------------------------------.
| Filename: Search.cpp                     | Search base classes             |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Search.h>
#include <lbcpp-ml/SolutionContainer.h>
using namespace lbcpp;

/*
** SearchTrajectory
*/
void SearchTrajectory::pop()
{
  if (states.size() == actions.size())
    states.pop_back();
  actions.pop_back();
}

bool SearchTrajectory::areStatesComputed() const
  {return states.size() == actions.size();}

void SearchTrajectory::ensureStatesAreComputed(ExecutionContext& context, SearchStatePtr initialState)
{
  if (areStatesComputed())
    return;
  states.resize(actions.size());
  states[0] = initialState;
  for (size_t i = 1; i < states.size(); ++i)
  {
    states[i] = states[i - 1]->cloneAndCast<SearchState>();
    states[i]->performTransition(context, actions[i - 1]);
  }
}

String SearchTrajectory::toShortString() const
{
  if (actions.empty())
    return "<empty trajectory>";
  String res;
  for (size_t i = 0; i < actions.size(); ++i)
  {
    res += actions[i]->toShortString();
    if (i < actions.size() - 1)
      res += ", ";
  }
  return res;
}

int SearchTrajectory::compare(const ObjectPtr& otherObject) const
  {return finalState->compare(otherObject.staticCast<SearchTrajectory>()->finalState);}
  
void SearchTrajectory::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  SearchTrajectoryPtr t = target.staticCast<SearchTrajectory>();
  t->states = states;
  t->actions = actions;
  t->finalState = finalState ? finalState->cloneAndCast<SearchState>() : SearchStatePtr();
}

/*
** SearchSampler
*/
void SearchSampler::initialize(ExecutionContext& context, const DomainPtr& domain)
  {this->domain = domain.staticCast<SearchDomain>();}
  
void SearchSampler::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const ReferenceCountedObjectPtr<SearchSampler>& target = t.staticCast<SearchSampler>();
  target->domain = domain;
}

/*
** SearchAlgorithm
*/
void SearchAlgorithm::configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
{
  Solver::configure(context, problem, solutions, initialSolution, verbosity);
  domain = problem->getDomain().staticCast<SearchDomain>();
  trajectory = initialSolution.staticCast<SearchTrajectory>();
  if (trajectory)
    trajectory = trajectory->cloneAndCast<SearchTrajectory>();
  else
  {
    trajectory = new SearchTrajectory();
    trajectory->setFinalState(domain->createInitialState());
  }
}

void SearchAlgorithm::clear(ExecutionContext& context)
{
  Solver::clear(context);
  domain = SearchDomainPtr();
  trajectory = SearchTrajectoryPtr();
}

/*
** DecoratorSearchAlgorithm
*/
void DecoratorSearchAlgorithm::subSearch(ExecutionContext& context)
{
  SearchStatePtr state = trajectory->getFinalState();
  if (state->isFinalState())
    evaluate(context, trajectory->cloneAndCast<SearchTrajectory>());
  else
  {
    SolutionContainerPtr subSolutions = algorithm->optimize(context, problem, trajectory, verbosity > verbosityQuiet ? (Verbosity)(verbosity - 1) : verbosityQuiet);
    solutions->insertSolutions(subSolutions);
  }
}

/*
** SearchNode
*/
SearchNode::SearchNode(SearchNode* parent, const SearchStatePtr& state)
  : parent(parent), state(state), actions(state->getActionDomain().staticCast<DiscreteDomain>())
{
  if (actions && actions->getNumElements())
  {
    successors.resize(actions->getNumElements(), NULL);
    fullyVisited = false;
  }
  else
    fullyVisited = true;
}

SearchNode::SearchNode() : parent(NULL)
{
}

DiscreteDomainPtr SearchNode::getPrunedActionDomain() const
{
  DiscreteDomainPtr res = new DiscreteDomain();
  std::vector<size_t> candidates;
  for (size_t i = 0; i < successors.size(); ++i)
    if (!successors[i] || !successors[i]->fullyVisited)
      res->addElement(actions->getElement(i));
  return res;
}

static bool isSameAction(const ObjectPtr& action1, const ObjectPtr& action2)
  {return Variable(action1) == Variable(action2);}

SearchNodePtr SearchNode::getSuccessor(ExecutionContext& context, const ObjectPtr& action)
{
  jassert(!state->isFinalState());
  jassert(actions);
  for (size_t i = 0; i < actions->getNumElements(); ++i)
    if (isSameAction(actions->getElement(i), action))
    {
      SearchNodePtr& succ = successors[i];
      if (!succ)
      {
        SearchStatePtr nextState = state->cloneAndCast<SearchState>();
        nextState->performTransition(context, action);
        succ = new SearchNode(this, nextState);
        updateIsFullyVisited();
      }
      return succ;
    }

  jassertfalse;
  return NULL;
}

void SearchNode::updateIsFullyVisited()
{
  bool previousValue = fullyVisited;
  fullyVisited = true;
  for (size_t i = 0; i < successors.size(); ++i)
    if (!successors[i] || !successors[i]->fullyVisited)
    {
      fullyVisited = false;
      break;
    }
  if (parent && previousValue != fullyVisited)
    parent->updateIsFullyVisited();
}

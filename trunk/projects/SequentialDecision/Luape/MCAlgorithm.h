/*-----------------------------------------.---------------------------------.
| Filename: MCAlgorithm.h                  | Monte Carlo Algorithms          |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2012 15:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_ALGORITHM_H_
# define LBCPP_LUAPE_MC_ALGORITHM_H_

# include "MCObjective.h"
# include "../Core/SinglePlayerMCTSOptimizer.h"

# include "../Problem/MorpionProblem.h" // tmp, for debugging

namespace lbcpp
{

/*
** Search Algorithm
*/
class MCAlgorithm : public Object
{
public:
  MCAlgorithm() : isSearching(false) {}

  double search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState, std::vector<Variable>& actions, DecisionProblemStatePtr& finalState)
  {
    jassert(!isSearching);
    isSearching = true;

    bestScore = -DBL_MAX;
    jassert(bestActions.empty());
    jassert(bestFinalState == DecisionProblemStatePtr());

    //String dbg = initialState->toShortString();

    /*ContainerPtr dbgActions = initialState->getAvailableActions();
    if (!dbgActions || !dbgActions->getNumElements())
      return bestScore;*/

    search(context, objective, actions, initialState);
    jassert((bestScore == -DBL_MAX) == (bestFinalState == DecisionProblemStatePtr()));
    jassert((bestScore == -DBL_MAX) == bestActions.empty());
    //String dbg2 = initialState->toShortString();
    //jassert(dbg == dbg2);

#ifdef JUCE_DEBUG
    for (size_t i = 0; i < actions.size(); ++i)
      jassert(actions[i] == bestActions[i]);
#endif // JUCE_DEBUG
    actions = bestActions;
    
    finalState = bestFinalState;
    /*if (bestScore != -DBL_MAX)
    {
      size_t i;
      for (i = 0; i < dbgActions->getNumElements(); ++i)
        if (firstAction.toShortString() == dbgActions->getElement(i).toShortString())
          break;
      jassert(i < dbgActions->getNumElements());
    }*/

    bestFinalState = DecisionProblemStatePtr(); // free memory
    bestActions.clear();
    isSearching = false;
    return bestScore;
  }    

  virtual void startSearch(ExecutionContext& context, DecisionProblemStatePtr initialState) {}
  virtual void finishSearch(ExecutionContext& context) {}

protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state) = 0;

  double bestScore;
  DecisionProblemStatePtr bestFinalState;
  std::vector<Variable> bestActions;
  bool isSearching;

  double submitFinalState(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& actions, DecisionProblemStatePtr state, double score = -DBL_MAX, bool verbose = false)
  {
    if (score == -DBL_MAX)
    {
      score = objective->evaluate(context, state);
      //context.informationCallback(state->toShortString() + T(" => ") + String(score));
    }
    if (score > bestScore)
    {
      bestScore = score;
      bestFinalState = state;
      bestActions = actions;
      if (verbose)
      {
        context.enterScope("New record: " + String(score));
        context.resultCallback("finalState", state->cloneAndCast<DecisionProblemState>());
        context.resultCallback("score", score);
        context.leaveScope();
      }
      jassert(actions.size());
    }
    return score;
  }
};

typedef ReferenceCountedObjectPtr<MCAlgorithm> MCAlgorithmPtr;
extern ClassPtr mcAlgorithmClass;

class RolloutMCAlgorithm : public MCAlgorithm
{
protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    state = state->cloneAndCast<DecisionProblemState>();
    size_t t = 0;
    std::vector<Variable> actions(previousActions);
    while (!state->isFinalState())
    {
      if (objective->shouldStop())
        return;
      ContainerPtr availableActions = state->getAvailableActions();
      size_t n = availableActions->getNumElements();
      Variable action = availableActions->getElement(context.getRandomGenerator()->sampleSize(n));
      actions.push_back(action);
      double reward;
      state->performTransition(context, action, reward);
      ++t;
    }
    submitFinalState(context, objective, actions, state);
  }
};

extern ClassPtr rolloutMCAlgorithmClass;

class DecoratorMCAlgorithm : public MCAlgorithm
{
public:
  DecoratorMCAlgorithm(MCAlgorithmPtr algorithm)
    : algorithm(algorithm) {}
  DecoratorMCAlgorithm() {}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    MCAlgorithm::clone(context, target);
    if (algorithm)
      target.staticCast<DecoratorMCAlgorithm>()->algorithm = algorithm->cloneAndCast<MCAlgorithm>();
  }

  const MCAlgorithmPtr& getAlgorithm() const
    {return algorithm;}

  virtual void startSearch(ExecutionContext& context, DecisionProblemStatePtr initialState)
    {if (algorithm) algorithm->startSearch(context, initialState);}

  virtual void finishSearch(ExecutionContext& context)
    {if (algorithm) algorithm->finishSearch(context);}

protected:
  friend class DecoratorMCAlgorithmClass;

  MCAlgorithmPtr algorithm;

  double subSearch(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr state, std::vector<Variable>& actions, DecisionProblemStatePtr& finalState)
  {
    if (state->isFinalState())
      return submitFinalState(context, objective, actions, state);
    else
    {
      double res = algorithm->search(context, objective, state, actions, finalState);
      if (res != -DBL_MAX)
        submitFinalState(context, objective, actions, finalState, res);
      return res;
    }
  }
};

typedef ReferenceCountedObjectPtr<DecoratorMCAlgorithm> DecoratorMCAlgorithmPtr;

class IterateMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  IterateMCAlgorithm(MCAlgorithmPtr algorithm, size_t numIterations)
    : DecoratorMCAlgorithm(algorithm), numIterations(numIterations) {}
  IterateMCAlgorithm() {}

  size_t getNumIterations() const
    {return numIterations;}

protected:
  friend class IterateMCAlgorithmClass;

  size_t numIterations;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    for (size_t i = 0; !numIterations || i < numIterations; ++i)
    {
      if (objective->shouldStop())
        break;

      std::vector<Variable> actions(previousActions);
      DecisionProblemStatePtr finalState;
      subSearch(context, objective, state, actions, finalState);
    }
  }
};

extern ClassPtr iterateMCAlgorithmClass;

class LookAheadMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  LookAheadMCAlgorithm(MCAlgorithmPtr algorithm, double numActions = 1.0)
    : DecoratorMCAlgorithm(algorithm), numActions(numActions) {}
  LookAheadMCAlgorithm() : numActions(0.0) {}

protected:
  friend class LookAheadMCAlgorithmClass;

  double numActions;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(n, order);
    std::vector<Variable> selectedActions((size_t)(juce::jmax(1.0, n * numActions)));
    for (size_t i = 0; i < selectedActions.size(); ++i)
      selectedActions[i] = actions->getElement(order[i]);

    for (size_t i = 0; i < selectedActions.size(); ++i)
    {
      if (objective->shouldStop())
        break;
      Variable action = selectedActions[i];
      double reward;
      Variable stateBackup;
      state->performTransition(context, action, reward, &stateBackup);

      std::vector<Variable> actions(previousActions);
      actions.push_back(action);

      DecisionProblemStatePtr finalState;
      subSearch(context, objective, state, actions, finalState);

      state->undoTransition(context, stateBackup);
    }
  }
};

extern ClassPtr lookAheadMCAlgorithmClass;

class StepByStepMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  StepByStepMCAlgorithm(MCAlgorithmPtr algorithm, bool useGlobalBest)
    : DecoratorMCAlgorithm(algorithm), useGlobalBest(useGlobalBest) {}
  StepByStepMCAlgorithm() : useGlobalBest(false) {}

protected:
  friend class StepByStepMCAlgorithmClass;

  bool useGlobalBest;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    state = state->cloneAndCast<DecisionProblemState>();
    std::vector<Variable> actions(previousActions);
//    context.informationCallback(T("----"));
    while (!state->isFinalState() && !objective->shouldStop())
    {
      std::vector<Variable> bestActions(actions);
      DecisionProblemStatePtr bestFinalState;
      /*double score = */subSearch(context, objective, state, bestActions, bestFinalState);

      /*if (bestFinalState)
      {
        String info = "Best Actions:";
        for (size_t i = 0; i < bestActions.size(); ++i)
          info += " " + bestActions[i].toShortString();
        context.informationCallback(info + T(" (") + String(score) + T(")"));
      }
      else
        context.informationCallback("No Best Final State");*/

      // select action
      Variable selectedAction;
      if (useGlobalBest && this->bestFinalState)
        selectedAction = this->bestActions[actions.size()];  // global best
      else if (bestFinalState)
        selectedAction = bestActions[actions.size()];       // local best
      if (!selectedAction.exists())
        break;

      double reward;
      actions.push_back(selectedAction);
      state->performTransition(context, selectedAction, reward);
      
      while (!state->isFinalState())
      {
        ContainerPtr availableActions = state->getAvailableActions();
        if (availableActions->getNumElements() > 1)
          break;
        Variable action = availableActions->getElement(0);
        actions.push_back(action);
        state->performTransition(context, action, reward);
      }
    }
  }
};

extern ClassPtr stepByStepMCAlgorithmClass;

// politique en dur:  rk + 2 / tk
class SelectMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  SelectMCAlgorithm(MCAlgorithmPtr algorithm, double explorationCoefficient = 0.5)
    : DecoratorMCAlgorithm(algorithm), explorationCoefficient(explorationCoefficient) {}
  SelectMCAlgorithm() {}

  virtual void startSearch(ExecutionContext& context, DecisionProblemStatePtr initialState)
  {
    tree = new SinglePlayerMCTSNode(initialState->cloneAndCast<DecisionProblemState>());
    DecoratorMCAlgorithm::startSearch(context, initialState);
	// initialize counter here
	maxLeaf=1000000;
	numLeaf=0;
	
  }

  virtual void finishSearch(ExecutionContext& context)
  {
    tree = SinglePlayerMCTSNodePtr();
    DecoratorMCAlgorithm::finishSearch(context);
  }

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr initialState)
  {
    // find local root
    SinglePlayerMCTSNodePtr root = this->tree;
    SinglePlayerMCTSNodePtr localRoot = root;
    for (size_t i = 0; i < previousActions.size(); ++i)
    {
      if (!localRoot->isExpanded())
        {localRoot->expand(context);
		// numLeaf++;                  
		// if(numLeaf<maxLeaf)				
		//   break;
		}
      localRoot = localRoot->getSubNodeByAction(previousActions[i]);
      jassert(localRoot);
    }

    // select leaf
    SinglePlayerMCTSNodePtr leaf = localRoot->select(context,explorationCoefficient);

    //std::cout << "Selected node: " << leaf->toShortString() << std::flush;

    // expand
    if (!leaf->isExpanded() && numLeaf<maxLeaf) // && counter > 1
      {leaf->expand(context); 
	    numLeaf++;
	  }
    // sub search
    std::vector<Variable> bestActions;//(previousActions);
    for (SinglePlayerMCTSNodePtr ptr = leaf; ptr != root; ptr = ptr->getParentNode())
      bestActions.insert(bestActions.begin(), ptr->getLastAction());

/*    size_t initialDepth = initialState.staticCast<LuapeNodeBuilderState>()->getCurrentStep();
    size_t leafDepth = leaf->getState().staticCast<LuapeNodeBuilderState>()->getCurrentStep();
    jassert(leafDepth - initialDepth == bestActions.size());*/

    DecisionProblemStatePtr bestFinalState;
    double reward = subSearch(context, objective, leaf->getState(), bestActions, bestFinalState);

    // normalize reward and back-propagate
    double worst, best;
    objective->getObjectiveRange(worst, best);
    leaf->backPropagate((reward - worst) / (best - worst));
  }

private:
  friend class SelectMCAlgorithmClass;

  double explorationCoefficient;
  size_t maxLeaf;
  size_t numLeaf;
  SinglePlayerMCTSNodePtr tree;
};

extern ClassPtr selectMCAlgorithmClass;

/*
** Constructors
*/
inline MCAlgorithmPtr rollout()
  {return new RolloutMCAlgorithm();}

inline MCAlgorithmPtr iterate(MCAlgorithmPtr algorithm, size_t numIterations)
  {return new IterateMCAlgorithm(algorithm, numIterations);}

inline MCAlgorithmPtr step(MCAlgorithmPtr algorithm, bool useGlobalBest = true)
  {return new StepByStepMCAlgorithm(algorithm, useGlobalBest);}

inline MCAlgorithmPtr lookAhead(MCAlgorithmPtr algorithm, double numActions = 1.0)
  {return new LookAheadMCAlgorithm(algorithm, numActions);}

inline MCAlgorithmPtr select(MCAlgorithmPtr algorithm)
  {return new SelectMCAlgorithm(algorithm);}

/*
** MCAlgorithmSet
*
class MCAlgorithmSet : public Object
{
public:
  MCAlgorithmSet(size_t maxSize)
  {
    addAlgorithm(rollout());
    for (size_t i = 1; i < maxSize; ++i)
      addAlgorithmSuccessors();
    pruneIterateAlgorithms();
  }

  MCAlgorithmSet() {}

  void addAlgorithm(MCAlgorithmPtr algorithm)
  {
    algorithm = canonizeAlgorithm(algorithm);
    String str = algorithm->toShortString();
    AlgorithmMap::const_iterator it = m.find(str);
    if (it == m.end())
      m[str] = algorithm;
  }

  void addAlgorithmSuccessors()
  {
    AlgorithmMap m = this->m;
    for (AlgorithmMap::const_iterator it = m.begin(); it != m.end(); ++it)
      expandAlgorithm(it->second);
  }

  void pruneIterateAlgorithms()
  {
    AlgorithmMap::iterator it, nxt;
    for (it = m.begin(); it != m.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (it->second.isInstanceOf<IterateMCAlgorithm>())
        m.erase(it);
    }
  }

  size_t getNumAlgorithms() const
    {return m.size();}

  void getAlgorithms(std::vector<MCAlgorithmPtr>& res)
  {
    res.clear();
    res.reserve(m.size());
    for (AlgorithmMap::const_iterator it = m.begin(); it != m.end(); ++it)
      res.push_back(it->second);
  }

protected:
  typedef std::map<String, MCAlgorithmPtr> AlgorithmMap;

  AlgorithmMap m;

  void expandAlgorithm(MCAlgorithmPtr algorithm)
  {
    addAlgorithm(iterate(algorithm, 2));
    addAlgorithm(iterate(algorithm, 5));
    addAlgorithm(step(algorithm, false));
    addAlgorithm(step(algorithm, true));
    addAlgorithm(lookAhead(algorithm, 0.1));
    addAlgorithm(lookAhead(algorithm, 0.5));
    addAlgorithm(lookAhead(algorithm, 1.0));
  }

  MCAlgorithmPtr canonizeAlgorithm(MCAlgorithmPtr algorithm)
  {
    DecoratorMCAlgorithmPtr decorator = algorithm.dynamicCast<DecoratorMCAlgorithm>();
    if (!decorator)
      return algorithm;
    MCAlgorithmPtr subAlgorithm = decorator->getAlgorithm();
    if (decorator.isInstanceOf<IterateMCAlgorithm>() && subAlgorithm.isInstanceOf<IterateMCAlgorithm>())
    {
      
      return iterate(subAlgorithm.staticCast<IterateMCAlgorithm>()->getAlgorithm(),
                decorator.staticCast<IterateMCAlgorithm>()->getNumIterations() * 
                      subAlgorithm.staticCast<IterateMCAlgorithm>()->getNumIterations());
    }

    return algorithm;
  }
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_ALGORITHM_H_

/*-----------------------------------------.---------------------------------.
| Filename: MetaMCSandBox.h                | Meta Monte Carlo SandBox        |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2012 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_
# define LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

/*
** Objective Function
*/
class MCObjective : public Object
{
public:
  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState) = 0;
  virtual bool shouldStop() const
    {return false;}
};

typedef ReferenceCountedObjectPtr<MCObjective> MCObjectivePtr;

/*
** Search Algorithm
*/
class MCAlgorithm : public Object
{
public:
  double search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState, Variable& firstAction, DecisionProblemStatePtr& finalState)
  {
    bestScore = -DBL_MAX;
    bestFirstAction = Variable();
    bestFinalState = DecisionProblemStatePtr();

    //String dbg = initialState->toShortString();

    /*ContainerPtr dbgActions = initialState->getAvailableActions();
    if (!dbgActions || !dbgActions->getNumElements())
      return bestScore;*/

    search(context, objective, initialState);
    jassert((bestScore == -DBL_MAX) == (bestFinalState == DecisionProblemStatePtr()));
    jassert((bestScore == -DBL_MAX) == !bestFirstAction.exists());
    //String dbg2 = initialState->toShortString();
    //jassert(dbg == dbg2);

    firstAction = bestFirstAction;
    finalState = bestFinalState;
    /*if (bestScore != -DBL_MAX)
    {
      size_t i;
      for (i = 0; i < dbgActions->getNumElements(); ++i)
        if (firstAction.toShortString() == dbgActions->getElement(i).toShortString())
          break;
      jassert(i < dbgActions->getNumElements());
    }*/
    return bestScore;
  }    

protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState) = 0;

  double bestScore;
  DecisionProblemStatePtr bestFinalState;
  Variable bestFirstAction;

  void submitFinalState(ExecutionContext& context, MCObjectivePtr objective, const Variable& firstAction, DecisionProblemStatePtr state, double score = -DBL_MAX)
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
      bestFirstAction = firstAction;
    }
  }
};

typedef ReferenceCountedObjectPtr<MCAlgorithm> MCAlgorithmPtr;

class RolloutMCAlgorithm : public MCAlgorithm
{
protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
    size_t t = 0;
    Variable firstAction;
    while (!state->isFinalState() && !objective->shouldStop())
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      Variable action = actions->getElement(context.getRandomGenerator()->sampleSize(n));
      if (t == 0)
        firstAction = action;
      double reward;
      state->performTransition(context, action, reward);
      ++t;
    }
    if (state->isFinalState())
      submitFinalState(context, objective, firstAction, state);
  }
};

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

protected:
  friend class DecoratorMCAlgorithmClass;

  MCAlgorithmPtr algorithm;

  double subSearch(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState, const Variable& currentFirstAction, Variable& firstAction, DecisionProblemStatePtr& finalState)
  {
    double res = algorithm->search(context, objective, initialState, firstAction, finalState);
    if (res != -DBL_MAX)
      submitFinalState(context, objective, currentFirstAction.exists() ? currentFirstAction : firstAction, finalState, res);
    return res;
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

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    for (size_t i = 0; i < numIterations; ++i)
    {
      if (objective->shouldStop())
        break;
      Variable firstAction;
      DecisionProblemStatePtr finalState;
      subSearch(context, objective, initialState, Variable(), firstAction, finalState);
    }
  }
};

class LookAheadMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  LookAheadMCAlgorithm(MCAlgorithmPtr algorithm, double numActions = 1.0)
    : DecoratorMCAlgorithm(algorithm), numActions(numActions) {}
  LookAheadMCAlgorithm() : numActions(0.0) {}

protected:
  friend class LookAheadMCAlgorithmClass;

  double numActions;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr state)
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

      if (state->isFinalState())
        submitFinalState(context, objective, action, state->cloneAndCast<DecisionProblemState>());
      else
      {
        DecisionProblemStatePtr finalState;
        Variable secondAction;
        double score = subSearch(context, objective, state, action, secondAction, finalState);
        jassert(score != -DBL_MAX && finalState);
      }

      state->undoTransition(context, stateBackup);
    }
  }
};

class StepByStepMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  StepByStepMCAlgorithm(MCAlgorithmPtr algorithm = MCAlgorithmPtr())
    : DecoratorMCAlgorithm(algorithm) {}

protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    Variable firstAction;
    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
    size_t t = 0;
    while (!state->isFinalState() && !objective->shouldStop())
    {
      Variable bestFirstAction;
      DecisionProblemStatePtr bestFinalState;
      subSearch(context, objective, state, firstAction, bestFirstAction, bestFinalState);
      if (!bestFinalState)
        break;

      if (t == 0)
        firstAction = bestFirstAction;

      double reward;
      state->performTransition(context, bestFirstAction, reward);
      ++t;
    }
  }
};

/*
** Optimizer
*/
class WrapperMCObjective : public MCObjective
{
public:
  WrapperMCObjective(FunctionPtr lbcppObjective, size_t budget)
    : lbcppObjective(lbcppObjective), budget(budget), numEvaluations(0) {}

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    String str = finalState->toShortString();
    std::map<String, double>::iterator it = cache.find(str);
    if (it != cache.end())
      return it->second;
    ++numEvaluations;
    double res = lbcppObjective->compute(context, finalState).toDouble();
    cache[str] = res;
    return res;
  }

  virtual bool shouldStop() const
    {return numEvaluations >= budget;}

protected:
  FunctionPtr lbcppObjective;
  size_t budget;
  size_t numEvaluations;
  std::map<String, double> cache;
};

class MCOptimizer : public Optimizer
{
public:
  MCOptimizer(MCAlgorithmPtr algorithm, size_t budget)
    : algorithm(algorithm), budget(budget) {}
  MCOptimizer() : budget(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    Variable firstAction;
    DecisionProblemStatePtr bestFinalState;
    MCAlgorithmPtr algorithm = new IterateMCAlgorithm(this->algorithm, 0x7FFFFFFF); // repeat base algorithm until budget is exhausted
    double bestScore = algorithm->search(context, new WrapperMCObjective(problem->getObjective(), budget), problem->getInitialState(), firstAction, bestFinalState);
    optimizerState->submitSolution(bestFinalState, bestScore);
    return optimizerState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new OptimizerState(problem);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Optimizer::clone(context, target);
    target.staticCast<MCOptimizer>()->algorithm = algorithm->cloneAndCast<MCAlgorithm>();
  }

protected:
  friend class MCOptimizerClass;

  MCAlgorithmPtr algorithm;
  size_t budget;
};

/*
** Misc
*/

inline MCAlgorithmPtr rollout()
  {return new RolloutMCAlgorithm();}

inline MCAlgorithmPtr iterate(MCAlgorithmPtr algorithm, size_t numIterations)
  {return new IterateMCAlgorithm(algorithm, numIterations);}

inline MCAlgorithmPtr step(MCAlgorithmPtr algorithm)
  {return new StepByStepMCAlgorithm(algorithm);}

inline MCAlgorithmPtr lookAhead(MCAlgorithmPtr algorithm, double numActions)
  {return new LookAheadMCAlgorithm(algorithm, numActions);}

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
    addAlgorithm(step(algorithm));
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
};

/*
** SandBox
*
class MetaMCSandBox : public WorkUnit
{
public:
  MetaMCSandBox() {}

  virtual Variable run(ExecutionContext& context)
  {
    return true;
  }

protected:
  friend class MetaMCSandBoxClass;
};
*/

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_

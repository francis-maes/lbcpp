/*-----------------------------------------.---------------------------------.
| Filename: SinglePlayerMCTSOptimizer.h    | Single Player MCTS Optimizer    |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_SINGLE_PLAYER_MCTS_H_
# define LBCPP_OPTIMIZER_SINGLE_PLAYER_MCTS_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include "SearchTree.h"

namespace lbcpp
{

class SinglePlayerMCTSNode;
typedef ReferenceCountedObjectPtr<SinglePlayerMCTSNode> SinglePlayerMCTSNodePtr;

class SinglePlayerMCTSNode : public Object
{
public:
  SinglePlayerMCTSNode(DecisionProblemStatePtr state)
    : state(state), parent(NULL), indexInParent((size_t)-1), rewards(new ScalarVariableStatistics("reward"))
    {computeActionsAndCreateSubNodes();}

  SinglePlayerMCTSNode(SinglePlayerMCTSNode* parent, size_t indexInParent)
    : parent(parent), indexInParent(indexInParent), rewards(new ScalarVariableStatistics("reward")) {}

  SinglePlayerMCTSNode() : indexInParent((size_t)-1) {}

  bool isExpanded() const
    {return state;}

  const DecisionProblemStatePtr& getState() const
    {return state;}

  bool isFinalState() const
    {jassert(isExpanded()); return state->isFinalState() || !actions->getNumElements();}

  ContainerPtr getActions() const
    {return actions;}

  void expand(ExecutionContext& context)
  {
    jassert(!isExpanded());
    jassert(parent && parent->isExpanded());
    Variable action = parent->getActions()->getElement(indexInParent);
    state = parent->getState()->cloneAndCast<DecisionProblemState>();
    double reward;
    state->performTransition(context, action, reward);
    computeActionsAndCreateSubNodes();
  }

  size_t getNumSubNodes() const
    {return subNodes.size();}

  const SinglePlayerMCTSNodePtr& getSubNode(size_t index) const
    {jassert(index < subNodes.size()); return subNodes[index];}

  SinglePlayerMCTSNodePtr getParentNode() const
    {return SinglePlayerMCTSNodePtr(parent);}

  double getIndexScore() const
  {
    size_t count = (size_t)rewards->getCount();
    if (!count)
      return DBL_MAX;
    else
      return rewards->getMean() + 5.0 / count;
  }

  void observeReward(double reward)
    {rewards->push(reward);}

  void getFinalStatesSortedByReward(std::multimap<double, SinglePlayerMCTSNodePtr>& res) const
  {
    if (isExpanded() && isFinalState())
      res.insert(std::make_pair(rewards->getMean(), refCountedPointerFromThis(this)));
    else
    {
      for (size_t i = 0; i < subNodes.size(); ++i)
        subNodes[i]->getFinalStatesSortedByReward(res);
    }
  }

protected:
  friend class SinglePlayerMCTSNodeClass;

  DecisionProblemStatePtr state;
  ContainerPtr actions;
  std::vector<SinglePlayerMCTSNodePtr> subNodes;
  SinglePlayerMCTSNode* parent;
  size_t indexInParent;
  ScalarVariableStatisticsPtr rewards;

  void computeActionsAndCreateSubNodes()
  {
    jassert(subNodes.empty());
    actions = state->getAvailableActions();
    subNodes.resize(actions->getNumElements());
    for (size_t i = 0; i < subNodes.size(); ++i)
      subNodes[i] = new SinglePlayerMCTSNode(this, i);
  }
};

class SinglePlayerMCTSOptimizerState : public OptimizerState
{
public:
  SinglePlayerMCTSOptimizerState(const OptimizationProblemPtr& problem, const DecisionProblemStatePtr& initialState, const FunctionPtr& objective)
    : OptimizerState(problem), rootNode(new SinglePlayerMCTSNode(initialState)), objective(objective) {}
  SinglePlayerMCTSOptimizerState() {}

  double doEpisode(ExecutionContext& context)
  { 
    SinglePlayerMCTSNodePtr leaf = select(context, rootNode);
    double reward = expandAndSimulate(context, leaf);
    backPropagate(leaf, reward);
    return reward;
  }

  const SinglePlayerMCTSNodePtr& getRootNode() const
    {return rootNode;}

protected:
  friend class SinglePlayerMCTSOptimizerStateClass;

  SinglePlayerMCTSNodePtr rootNode;
  FunctionPtr objective;

  SinglePlayerMCTSNodePtr select(ExecutionContext& context, SinglePlayerMCTSNodePtr node) const
  {
    while (node->isExpanded() && !node->isFinalState())
    {
      ContainerPtr actions = node->getActions();
      if (!actions->getNumElements())
        break;
      size_t bestSubNode = (size_t)-1;
      double bestIndexScore = -DBL_MAX;
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
      {
        double score = node->getSubNode(i)->getIndexScore();
        if (score > bestIndexScore)
        {
          bestIndexScore = score;
          bestSubNode = i;
        }
      }
      jassert(bestSubNode != (size_t)-1);
      node = node->getSubNode(bestSubNode);
    }
    return node;
  }

  double expandAndSimulate(ExecutionContext& context, const SinglePlayerMCTSNodePtr& node)
  {
    DecisionProblemStatePtr state;
    if (node->isExpanded())
      state = node->getState(); // the state is either final or has no actions => call the objective function directly on it, without random simulation
    else
    {
      node->expand(context);
      state = node->getState()->cloneAndCast<DecisionProblemState>();
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        if (!actions->getNumElements())
          break;
        size_t index = context.getRandomGenerator()->sampleSize(actions->getNumElements());
        double reward;
        state->performTransition(context, actions->getElement(index), reward);
      }
    }
    double res = objective->compute(context, state).toDouble();
    //context.informationCallback(state->toShortString() + T(" ==> ") + String(res));
    submitSolution(state, -res);
    return res;
  }

  void backPropagate(SinglePlayerMCTSNodePtr node, double reward)
  {
    while (node)
    {
      node->observeReward(reward);
      node = node->getParentNode();
    }
  }
};

typedef ReferenceCountedObjectPtr<SinglePlayerMCTSOptimizerState> SinglePlayerMCTSOptimizerStatePtr;

class SinglePlayerMCTSOptimizer : public Optimizer
{
public:
  SinglePlayerMCTSOptimizer(size_t budget = 0)
    : budget(budget) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    FunctionPtr objective = problem->getObjective();
    const SinglePlayerMCTSOptimizerStatePtr& state = optimizerState.staticCast<SinglePlayerMCTSOptimizerState>();
    for (size_t i = 0; i < budget; ++i)
      state->doEpisode(context);
   // context.resultCallback("mctsTree", state->getRootNode());
    return state;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new SinglePlayerMCTSOptimizerState(problem, problem->getInitialState(), problem->getObjective());}

protected:
  friend class SinglePlayerMCTSOptimizerClass;

  size_t budget;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_SINGLE_PLAYER_MCTS_H_

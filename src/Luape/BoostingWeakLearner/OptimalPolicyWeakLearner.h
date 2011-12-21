/*-----------------------------------------.---------------------------------.
| Filename: OptimalPolicyWeakLearner.h     | Optimal Policy Weak Learner     |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2011 13:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_OPTIMAL_POLICY_WEAK_H_
# define LBCPP_LUAPE_LEARNER_OPTIMAL_POLICY_WEAK_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include "LuapeGraphBuilder.h"

namespace lbcpp
{

class OptimalPolicyWeakLearner : public FiniteBoostingWeakLearner
{
public:
  OptimalPolicyWeakLearner(size_t maxDepth)
    : maxDepth(maxDepth) {}
  OptimalPolicyWeakLearner() : maxDepth(0) {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxDepth);
    typeSearchSpace->pruneStates(context);
    return true;
  }

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(structureLearner->getFunction(), typeSearchSpace);
    enumerateWeakNodes(context, structureLearner, builder, candidates);
    return true;
  }

protected:
  friend class OptimalPolicyWeakLearnerClass;

  size_t maxDepth;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;

  void enumerateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeGraphBuilderStatePtr& state, std::vector<LuapeNodePtr>& res) const
  {
    if (state->isFinalState())
    {
      if (state->getStackSize() == 1)
        res.push_back(state->getStackElement(0));
    }
    else
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable stateBackup;
        double reward;
        state->performTransition(context, actions->getElement(i), reward, &stateBackup);
        //context.enterScope(state->toShortString());
        enumerateWeakNodes(context, structureLearner, state, res);
        //context.leaveScope();
        state->undoTransition(context, stateBackup);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_OPTIMAL_POLICY_WEAK_H_

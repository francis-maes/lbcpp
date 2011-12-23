/*-----------------------------------------.---------------------------------.
| Filename: ExhaustiveWeakLearner.h        | Exhaustive Weak Learner         |
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

class ExhaustiveWeakLearner : public FiniteBoostingWeakLearner
{
public:
  ExhaustiveWeakLearner(size_t maxDepth)
    : maxDepth(maxDepth) {}
  ExhaustiveWeakLearner() : maxDepth(0) {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxDepth);
    typeSearchSpace->pruneStates(context);
    return true;
  }

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    // FIXME: see why this do not work in release
    enumerateCandidates(context, structureLearner, candidates);
   /* std::set<LuapeNodePtr> activeVariables = structureLearner->getFunction()->getActiveVariables();
    if (cachedActiveVariables != activeVariables || cachedCandidates.empty())
    {
      std::vector<LuapeNodePtr>& c = const_cast<ExhaustiveWeakLearner* >(this)->cachedCandidates;
      c.clear();
      enumerateCandidates(context, structureLearner, c);
      const_cast<ExhaustiveWeakLearner* >(this)->cachedActiveVariables = activeVariables;
    }
    size_t s = candidates.size();
    candidates.resize(s + cachedCandidates.size());
    for (size_t i = s; i < candidates.size(); ++i)
      candidates[i] = cachedCandidates[i - s];*/
    return true;
  }
  
protected:
  friend class ExhaustiveWeakLearnerClass;

  size_t maxDepth;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;

  std::set<LuapeNodePtr> cachedActiveVariables;
  std::vector<LuapeNodePtr> cachedCandidates;

  void enumerateCandidates(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(structureLearner->getFunction(), typeSearchSpace);
    std::set<LuapeNodePtr> weakNodes;
    enumerateWeakNodes(context, structureLearner, builder, weakNodes);
    candidates.reserve(candidates.size() + weakNodes.size());
    for (std::set<LuapeNodePtr>::const_iterator it = weakNodes.begin(); it != weakNodes.end(); ++it)
    {
      //context.informationCallback((*it)->toShortString());
      candidates.push_back(*it);
    }
  }

  void enumerateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeGraphBuilderStatePtr& state, std::set<LuapeNodePtr>& res) const
  {
    if (state->isFinalState())
    {
      if (state->getStackSize() == 1)
        res.insert(state->getStackElement(0));
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

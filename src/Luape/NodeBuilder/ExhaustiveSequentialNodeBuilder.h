/*-----------------------------------------.---------------------------------.
| Filename: ExhaustiveSequentialNodeBuilder.h | Exhaustive Weak Learner      |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 18:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_EXHAUSTIVE_SEQUENTIAL_H_
# define LBCPP_LUAPE_NODE_BUILDER_EXHAUSTIVE_SEQUENTIAL_H_

# include "NodeBuilderDecisionProblem.h"

namespace lbcpp
{

class ExhaustiveSequentialNodeBuilder : public LuapeNodeBuilder
{
public:
  ExhaustiveSequentialNodeBuilder(size_t complexity = 0)
    : complexity(complexity) {}

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
  {
    // FIXME: see why this do not work in release
    enumerateCandidates(context, function, res);
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
  }
  
protected:
  friend class ExhaustiveSequentialNodeBuilderClass;

  size_t complexity;

  //std::set<LuapeNodePtr> cachedActiveVariables;
  //std::vector<LuapeNodePtr> cachedCandidates;

  void enumerateCandidates(ExecutionContext& context, const LuapeInferencePtr& function, std::vector<LuapeNodePtr>& candidates) const
  {
    LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = function->getSearchSpace(context, complexity);
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(function, typeSearchSpace);
    std::set<LuapeNodePtr> weakNodes;
    enumerateWeakNodes(context, builder, weakNodes);
    candidates.reserve(candidates.size() + weakNodes.size());
    for (std::set<LuapeNodePtr>::const_iterator it = weakNodes.begin(); it != weakNodes.end(); ++it)
    {
      //context.informationCallback((*it)->toShortString());
      candidates.push_back(*it);
    }
  }

  void enumerateWeakNodes(ExecutionContext& context, const LuapeGraphBuilderStatePtr& state, std::set<LuapeNodePtr>& res) const
  {
    if (state->isFinalState())
    {
      if (state->getStackSize() == 1)
      {
        jassert(state->getStackElement(0));
        res.insert(state->getStackElement(0));
      }
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
        enumerateWeakNodes(context, state, res);
        //context.leaveScope();
        state->undoTransition(context, stateBackup);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_EXHAUSTIVE_SEQUENTIAL_H_

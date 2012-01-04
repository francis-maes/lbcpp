/*-----------------------------------------.---------------------------------.
| Filename: ExactWeakLearner.h             | Exact Weak Learner              |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 18:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_
# define LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_

# include <lbcpp/Luape/BoostingWeakLearner.h>

namespace lbcpp
{

class ExactWeakLearner : public BoostingWeakLearner
{
public:
  ExactWeakLearner(LuapeNodeBuilderPtr nodeBuilder) : nodeBuilder(nodeBuilder) {}
  ExactWeakLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    // make weak nodes
    std::vector<LuapeNodePtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 0, weakNodes);
    if (!weakNodes.size())
      return LuapeNodePtr();

    // evaluate each weak node
    LuapeNodePtr bestWeakNode;
    bestWeakObjectiveValue = -DBL_MAX;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      LuapeNodePtr weakNode = weakNodes[i];
      double objective = computeWeakObjectiveWithEventualStump(context, problem, weakNode, examples); // side effect on weakNode
      if (objective > bestWeakObjectiveValue)
      {
        bestWeakObjectiveValue = objective;
        bestWeakNode = weakNode;
      }
    }
    return bestWeakNode;
  }

protected:
  friend class ExactWeakLearnerClass;

  LuapeNodeBuilderPtr nodeBuilder;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_

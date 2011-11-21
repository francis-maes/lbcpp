/*-----------------------------------------.---------------------------------.
| Filename: SingleStumpWeakLearner.h       | Single Stump Weak Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_
# define LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_

# include "LuapeLearner.h"

namespace lbcpp
{

class SingleStumpWeakLearner : public BoostingWeakLearner
{
public:
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner) const
  {
    const LuapeGraphPtr& graph = structureLearner->getGraph();

    double bestScore = -DBL_MAX;
    LuapeNodePtr bestNode;

    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (!node.isInstanceOf<LuapeYieldNode>() && node->getType() == doubleType)
      {
        double score = structureLearner->computeWeakObjective(context, node);
        if (score > bestScore)
        {
          bestScore = score;
          bestNode = node;
        }
      }
    }

    context.informationCallback("Best node: " + bestNode->toShortString());
    context.informationCallback("Score: " + String(bestScore));
    return bestNode;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_

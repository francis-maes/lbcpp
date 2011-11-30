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
# include "LuapeSimpleFunctions.h"

namespace lbcpp
{

class NormalizedValueWeakLearner : public BoostingWeakLearner
{
public:
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner) const
  {
    const LuapeGraphPtr& graph = structureLearner->getGraph();

    double bestScore = -DBL_MAX;
    LuapeNodePtr bestNode;

    std::vector<LuapeNodePtr> doubleNodes;
    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (node->getType() == doubleType)
        doubleNodes.push_back(node);
    }

    std::vector<LuapeNodePtr> candidates;
    candidates.reserve(doubleNodes.size() + doubleNodes.size() * (doubleNodes.size() - 1) / 2);
    for (size_t i = 0; i < doubleNodes.size(); ++i)
    {
      candidates.push_back(doubleNodes[i]);
      for (size_t j = i; j < doubleNodes.size(); ++j)
      {
        std::vector<LuapeNodePtr> inputs(2);
        inputs[0] = doubleNodes[i];
        inputs[1] = doubleNodes[j];
        candidates.push_back(graph->getUniverse()->makeFunctionNode(mulDoubleLuapeFunction(), inputs));
      }
    }

    for (size_t i = 0; i < candidates.size(); ++i)
    {
      LuapeNodePtr node = candidates[i];
      LuapeFunctionNodePtr normalizer = graph->getUniverse()->makeFunctionNode(normalizerLuapeFunction(), node);
      normalizer->getFunction().staticCast<NormalizerLuapeFunction>()->initialize(graph->updateNodeCache(context, node, true).staticCast<DenseDoubleVector>());
      double score = structureLearner->computeWeakObjective(context, normalizer);
      context.informationCallback(node->toShortString() + T(" ==> ") + String(score));
      if (score > bestScore)
      {
        bestScore = score;
        bestNode = normalizer;
      }
    }

    context.informationCallback("Best node: " + bestNode->toShortString());
    context.informationCallback("Score: " + String(bestScore));
    context.informationCallback("Graph: " + graph->toShortString());
    return bestNode;
  }
};

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

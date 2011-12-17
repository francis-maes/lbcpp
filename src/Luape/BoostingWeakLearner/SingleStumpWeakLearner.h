/*-----------------------------------------.---------------------------------.
| Filename: SingleStumpWeakLearner.h       | Single Stump Weak Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_
# define LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include "../Function/SpecialLuapeFunctions.h"

namespace lbcpp
{

class SingleStumpWeakLearner : public FiniteBoostingWeakLearner
{
public:
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const
  {
    const LuapeInferencePtr& function = structureLearner->getFunction();
    for (size_t i = 0; i < function->getNumInputs(); ++i)
    {
      LuapeNodePtr node = function->getInput(i);
      if (node->getType() == doubleType)
        res.push_back(node);
    }
    return true;
  }
#if 0
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective) const
  {
    const LuapeInferencePtr& function = structureLearner->getFunction();

    weakObjective = -DBL_MAX;
    double bestThreshold = 0.0;
    LuapeNodePtr bestNode;

    for (size_t i = 0; i < function->getNumInputs(); ++i)
    {
      LuapeNodePtr node = function->getInput(i);
      if (node->getType() == doubleType)
      {
        double threshold;
        double score = computeWeakObjectiveWithStump(context, structureLearner, node, examples, threshold);
        if (score > weakObjective)
        {
          weakObjective = score;
          bestNode = node;
          bestThreshold = threshold;
        }
      }
    }
    if (!bestNode)
      return LuapeNodePtr();

    return makeContribution(context, structureLearner, makeStump(structureLearner, bestNode, bestThreshold), examples);
  }
#endif // 0
};

#if 0
class NormalizedValueWeakLearner : public BoostingWeakLearner
{
public:
 /* void countUseCount(const LuapeNodePtr& node, std::vector<size_t>& useCounts)
  {
    useCounts[node->getIndexInGraph()]++;
    if (node.isInstanceOf<LuapeFunctionNode>())
    {
      LuapeFunctionNodePtr functionNode = node.staticCast<LuapeFunctionNode>();
      for (size_t i = 0; i < functionNode->getNumArguments(); ++i)
        countUseCounts(functionNode->getArgument(i), useCounts);
    }
  }

  void countUseCounts(const LuapeGraphPtr& graph, std::vector<size_t>& useCounts)
  {
    useCounts.clear();
    useCounts.resize(graph->getNumNodes(), 0);
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeYieldNodePtr yieldNode = graph->getNode(i).dynamicCast<LuapeYieldNode>();
      if (yieldNode)
        countUseCount(yieldNode->getArgument(), useCounts);
    }
  }*/

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples) const
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
      if (doubleNodes[i].isInstanceOf<LuapeInputNode>())
      {
        candidates.push_back(doubleNodes[i]);
        for (size_t j = i; j < doubleNodes.size(); ++j)
          if (doubleNodes[j].isInstanceOf<LuapeInputNode>())
          {
            std::vector<LuapeNodePtr> inputs(2);
            inputs[0] = doubleNodes[i];
            inputs[1] = doubleNodes[j];
            LuapeNodePtr node = graph->getUniverse()->makeFunctionNode(mulDoubleLuapeFunction(), inputs);
            if (!graph->containsNode(node))
              candidates.push_back(node);
          }
      }

    if (!candidates.size())
      return LuapeNodePtr();
    for (size_t i = 0; i < candidates.size(); ++i)
    {
      LuapeNodePtr node = candidates[i];
      LuapeFunctionNodePtr normalizer = graph->getUniverse()->makeFunctionNode(normalizerLuapeFunction(), node);
      if (graph->containsNode(normalizer))
        continue;

      normalizer->getFunction().staticCast<NormalizerLuapeFunction>()->initialize(graph->updateNodeCache(context, node, true).staticCast<DenseDoubleVector>());
      double score = computeWeakObjective(context, structureLearner, normalizer, examples);
      //double score = context.getRandomGenerator()->sampleDouble();
      //context.informationCallback(node->toShortString() + T(" ==> ") + String(score));
      if (score > bestScore)
      {
        bestScore = score;
        bestNode = normalizer;
      }
    }

    if (bestNode)
    {
      context.informationCallback("Best node: " + bestNode->toShortString());
      context.informationCallback("Score: " + String(bestScore));
      context.informationCallback("Graph: " + graph->toShortString());
    }
    return bestNode;
  }
};
#endif // 0


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_SINGLE_STUMP_WEAK_H_

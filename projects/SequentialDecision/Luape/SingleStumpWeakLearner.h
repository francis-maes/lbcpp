/*-----------------------------------------.---------------------------------.
| Filename: SingleStumpWeakLearner.h       | Single Stump Weak Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_
# define LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_

# include "LuapeBatchLearner.h"

namespace lbcpp
{

class SingleStumpWeakLearner : public LuapeWeakLearner
{
public:
  SingleStumpWeakLearner() {}

  virtual LuapeGraphPtr learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->clearScores();
    size_t numExamples = supervisions->getNumElements();

    size_t bestVariable = (size_t)-1;
    double bestThreshold = 0.0;
    double bestEdge = -DBL_MAX;

    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (node->getType() == doubleType)
      {
        double edge;
        BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
        BooleanVectorPtr predictions = new BooleanVector(numExamples, true);
        edgeCalculator->initialize(function, predictions, supervisions, weights);
        double threshold = findBestThreshold(context, edgeCalculator, node, edge);
        if (edge > bestEdge)
        {
          bestEdge = edge;
          bestVariable = i;
          bestThreshold = threshold;
        }
      }
    }

    context.informationCallback("Best stump: " + graph->getNode(bestVariable)->toShortString() + " >= " + String(bestThreshold));
    context.informationCallback("Edge: " + String(bestEdge));

    graph->pushNode(context, new LuapeFunctionNode(new StumpFunction(bestThreshold), std::vector<size_t>(1, bestVariable))); // node index == n
    graph->pushNode(context, new LuapeYieldNode(n));
    graph->getLastNode()->getCache()->setScore(bestEdge);
    return graph;
  }

  double findBestThreshold(ExecutionContext& context, BoostingEdgeCalculatorPtr edgeCalculator, LuapeNodePtr node, double& edge) const
  {
    edge = -DBL_MAX;
    double res = 0.0;

    context.enterScope("Find best threshold for node " + node->toShortString());

    const std::vector< std::pair<size_t, double> >& sortedDoubleValues = node->getCache()->getSortedDoubleValues();
    jassert(sortedDoubleValues.size());
    double previousThreshold = sortedDoubleValues[0].second;
    for (size_t i = 0; i < sortedDoubleValues.size(); ++i)
    {
      double threshold = sortedDoubleValues[i].second;

      jassert(threshold >= previousThreshold);
      if (threshold > previousThreshold)
      {
        double e = edgeCalculator->computeEdge();

      context.enterScope("Iteration " + String((int)i));
      context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
      context.resultCallback("edge", e);
      context.leaveScope();

        if (e > edge)
          edge = e, res = (threshold + previousThreshold) / 2.0;
        previousThreshold = threshold;
      }
      edgeCalculator->flipPrediction(sortedDoubleValues[i].first);
    }

    context.leaveScope();
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_

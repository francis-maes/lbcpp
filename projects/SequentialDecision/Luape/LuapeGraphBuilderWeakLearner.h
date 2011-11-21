/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphBuilderWeakLearner.h | Graph builder based weak learner|
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_GRAPH_BUILDER_H_
# define LBCPP_LUAPE_WEAK_LEARNER_GRAPH_BUILDER_H_

# include "LuapeBatchLearner.h"
# include "LuapeGraphBuilder.h"

namespace lbcpp
{

#if 0 // FIXME
class LuapeGraphBuilderWeakLearner : public LuapeWeakLearner
{
public:
  LuapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps)
    : optimizer(optimizer), maxSteps(maxSteps) {}
  LuapeGraphBuilderWeakLearner() {}

  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    LuapeGraphPtr graph = function->getGraph();
    size_t numInitialNodes = graph->getNumNodes();
    graph->clearScores();
    FunctionPtr objective = new Objective(batchLearner, function, supervisions, weights);

    OptimizationProblemPtr optimizationProblem(new OptimizationProblem(objective));
    optimizationProblem->setInitialState(new LuapeRPNGraphBuilderState(batchLearner->getProblem(), graph, maxSteps));
    OptimizerStatePtr optimizerState = optimizer->optimize(context, optimizationProblem);
    LuapeRPNGraphBuilderStatePtr bestFinalState = optimizerState->getBestSolution().getObjectAndCast<LuapeRPNGraphBuilderState>();
    if (!bestFinalState)
      return std::vector<LuapeNodePtr>();

    LuapeGraphPtr bestGraph = bestFinalState->getGraph();
    context.informationCallback(String("Best Graph: ") + bestGraph->getLastNode()->toShortString() + T(" [") + String(optimizerState->getBestScore()) + T("]"));
    //context.informationCallback(String("Num cached nodes: ") + String(graphCache ? (int)graphCache->getNumCachedNodes() : 0));
    
    jassert(graph->getNumNodes() == numInitialNodes);
    jassert(bestGraph->getNumNodes() > numInitialNodes);
    jassert(bestGraph->getLastNode().isInstanceOf<LuapeYieldNode>());
    
    std::vector<LuapeNodePtr> res(bestGraph->getNumNodes() - 1 - graph->getNumNodes());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = bestGraph->getNode(graph->getNumNodes() + i);
    return res;
  }

protected:
  friend class LuapeGraphBuilderWeakLearnerClass;

  OptimizerPtr optimizer;
  size_t maxSteps;

  struct Objective : public SimpleUnaryFunction
  {
    Objective(const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function, ContainerPtr supervisions, DenseDoubleVectorPtr weights)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), batchLearner(batchLearner), function(function), supervisions(supervisions), weights(weights) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      LuapeRPNGraphBuilderStatePtr state = input.getObjectAndCast<LuapeRPNGraphBuilderState>();
      const LuapeGraphPtr& graph = state->getGraph();
      LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
      if (!yieldNode)
        return 0.0; // non-terminal state
      LuapeNodeCachePtr yieldNodeCache = yieldNode->getCache();

      LuapeNodePtr valueNode = yieldNode->getArgument();
      double score;
      if (yieldNodeCache->isScoreComputed())
        score = yieldNodeCache->getScore();
      else
      {
        BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
        edgeCalculator->initialize(function, valueNode->getCache()->getTrainingSamples().staticCast<BooleanVector>(), supervisions, weights);
        score = edgeCalculator->computeEdge();
        yieldNodeCache->setScore(score);
      }
      return score;
    }

  protected:
    const BoostingLuapeLearnerPtr batchLearner;
    const LuapeInferencePtr& function;
    ContainerPtr supervisions;
    DenseDoubleVectorPtr weights;
  };
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_GRAPH_BUILDER_H_

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

class LuapeGraphBuilderWeakLearner : public LuapeWeakLearner
{
public:
  LuapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps)
    : optimizer(optimizer), maxSteps(maxSteps) {}
  LuapeGraphBuilderWeakLearner() {}

  virtual LuapeGraphPtr learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->clearScores();
    LuapeGraphCachePtr graphCache = graph->getCache();
    FunctionPtr objective = new Objective(batchLearner, function, supervisions, weights);

    OptimizationProblemPtr optimizationProblem(new OptimizationProblem(objective));
    optimizationProblem->setInitialState(new LuapeRPNGraphBuilderState(batchLearner->getProblem(), graph, maxSteps));
    OptimizerStatePtr optimizerState = optimizer->optimize(context, optimizationProblem);
    LuapeRPNGraphBuilderStatePtr bestFinalState = optimizerState->getBestSolution().getObjectAndCast<LuapeRPNGraphBuilderState>();
    if (!bestFinalState)
      return LuapeGraphPtr();

    LuapeGraphPtr bestGraph = bestFinalState->getGraph();
    context.informationCallback(String("Best Graph: ") + bestGraph->getLastNode()->toShortString() + T(" [") + String(optimizerState->getBestScore()) + T("]"));
    context.informationCallback(String("Num cached nodes: ") + String((int)graphCache ? graphCache->getNumCachedNodes() : 0));
    return bestGraph;
  }

protected:
  friend class LuapeGraphBuilderWeakLearnerClass;

  OptimizerPtr optimizer;
  size_t maxSteps;

  struct Objective : public SimpleUnaryFunction
  {
    Objective(const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, ContainerPtr supervisions, DenseDoubleVectorPtr weights)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), batchLearner(batchLearner), function(function), supervisions(supervisions), weights(weights) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      LuapeRPNGraphBuilderStatePtr state = input.getObjectAndCast<LuapeRPNGraphBuilderState>();
      const LuapeGraphPtr& graph = state->getGraph();
      LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
      if (!yieldNode)
        return 0.0; // non-terminal state
      LuapeNodeCachePtr yieldNodeCache = yieldNode->getCache();

      LuapeNodePtr valueNode = graph->getNode(yieldNode->getArgument());
      double score;
      if (yieldNodeCache->isScoreComputed())
        score = yieldNodeCache->getScore();
      else
      {
        BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
        edgeCalculator->initialize(function, valueNode->getCache()->getExamples().staticCast<BooleanVector>(), supervisions, weights);
        score = edgeCalculator->computeEdge();
        yieldNodeCache->setScore(score);
      }
      return score;
    }

  protected:
    const BoostingLuapeLearnerPtr batchLearner;
    const LuapeFunctionPtr& function;
    ContainerPtr supervisions;
    DenseDoubleVectorPtr weights;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_GRAPH_BUILDER_H_

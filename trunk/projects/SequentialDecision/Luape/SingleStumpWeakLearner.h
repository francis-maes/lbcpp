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

  virtual LuapeGraphPtr learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const
  {
    /*
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
    return bestGraph;*/
    return LuapeGraphPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_

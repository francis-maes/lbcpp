/*-----------------------------------------.---------------------------------.
| Filename: LuapeBatchLearner.h            | Luape Batch Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_H_
# define LBCPP_LUAPE_BATCH_LEARNER_H_

# include "LuapeProblem.h"
# include "LuapeFunction.h"
# include "LuapeGraphBuilder.h"
# include <lbcpp/Learning/BatchLearner.h>
# include "../GP/NestedMonteCarloOptimizer.h"

namespace lbcpp
{

class LuapeBatchLearner : public BatchLearner
{
public:
  LuapeBatchLearner(LuapeProblemPtr problem = LuapeProblemPtr())
    : problem(problem) {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeFunctionClass;}

protected:
  friend class LuapeBatchLearnerClass;

  LuapeProblemPtr problem;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

class BoostingLuapeLearner : public LuapeBatchLearner
{
public:
  BoostingLuapeLearner(LuapeProblemPtr problem, size_t maxSteps, size_t maxIterations)
    : LuapeBatchLearner(problem), maxSteps(maxSteps), maxIterations(maxIterations) {}
  BoostingLuapeLearner() : maxIterations(0) {}

  virtual DenseDoubleVectorPtr makeInitialWeights(const std::vector<ObjectPtr>& examples) const = 0;

  // the absolute value of this quantity should be maximized
  virtual double computeWeakObjective(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const Variable& prediction, const Variable& supervision, double currentWeight, const Variable& vote) const = 0;

  virtual VectorPtr createVoteVector() const = 0;
  virtual Variable computeVote(double weakObjectiveValue) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeFunctionPtr& function = f.staticCast<LuapeFunction>();
    LuapeGraphPtr initialGraph = problem->createInitialGraph(context);
    DenseDoubleVectorPtr weights = makeInitialWeights(trainingData);
    function->setGraph(initialGraph->cloneAndCast<LuapeGraph>());
    VectorPtr supervisions;
    addExamplesToGraph(trainingData, function->getGraph(), supervisions);
    function->setVotes(createVoteVector());

    bool stopped = false;
    double weightsSum = 1.0;
    for (size_t i = 0; i < maxIterations && !stopped; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i+1));
      context.resultCallback(T("iteration"), i + 1);

      // weak graph completion
      LuapeGraphPtr newGraph = learnWeakModel(context, function->getGraph(), weights, supervisions);
      LuapeYieldNodePtr yieldNode = newGraph->getLastNode().dynamicCast<LuapeYieldNode>();
      jassert(yieldNode);
      double score = yieldNode->getCache()->getScore();
      LuapeNodeCachePtr cache = newGraph->getNode(yieldNode->getArgument())->getCache();

      // stop test
      if (shouldStop(score))
      {
        context.informationCallback(T("Stopping, score = ") + String(score));
        stopped = true;
      }
      else
      {
        function->setGraph(newGraph);

        // compute vote
        Variable vote = computeVote(score);
        function->getVotes()->append(vote);

        // update weights
        weightsSum *= updateWeights(cache->getExamples(), supervisions, weights, vote);
        {
          DenseDoubleVectorPtr w = weights->cloneAndCast<DenseDoubleVector>();
          w->multiplyByScalar(weightsSum);
          context.resultCallback("loss", w->l1norm());
        }

        context.resultCallback("score", score);
        context.resultCallback("vote", vote);
        context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

        context.resultCallback(T("trainScore"), function->evaluate(context, trainingData, EvaluatorPtr(), "Train evaluation")->getScoreToMinimize());
        context.resultCallback(T("validationScore"), function->evaluate(context, validationData, EvaluatorPtr(), "Validation evaluation")->getScoreToMinimize());
      }

      context.leaveScope();
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    }
    return true;
  }

protected:
  size_t maxSteps;
  size_t maxIterations;

  void addExamplesToGraph(const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph, VectorPtr& supervisions) const
  {
    size_t n = examples.size();
    graph->reserveExamples(n);
    supervisions = vector(examples[0]->getClass()->getTemplateArgument(1), n);
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = examples[i].staticCast<Pair>();
      graph->addExample(example->getFirst().getObjectAndCast<Container>());
      supervisions->setElement(i, example->getSecond());
    }
  }

  double updateWeights(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
  {
    size_t n = predictions->getNumElements();
    jassert(n == supervisions->getNumElements());
    jassert(n == weights->getNumElements());

    double alpha = vote.toDouble();
    double* values = weights->getValuePointer(0);
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      values[i] = updateWeight(predictions->getElement(i), supervisions->getElement(i), values[i], alpha);
      sum += values[i];
    }
    weights->multiplyByScalar(1.0 / sum);
    return sum;
  }

  void exhaustiveSearch(ExecutionContext& context, LuapeRPNGraphBuilderStatePtr state, const FunctionPtr& objective, double& bestScore, LuapeGraphPtr& bestGraph) const
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = actions->getElement(i);
      double reward;
      //context.enterScope(action.toShortString());
      Variable stateBackup;
      state->performTransition(context, action, reward, &stateBackup);

      if (!state->isFinalState())
        exhaustiveSearch(context, state, objective, bestScore, bestGraph);
      else
      {
        double score = objective->compute(context, state->getGraph()).toDouble();
        if (score != DBL_MAX)
        {
          //context.informationCallback(state->toShortString() + T(" => ") + String(score));
          if (score < bestScore)
          {
            bestScore = score;
            bestGraph = state->getGraph()->cloneAndCast<LuapeGraph>();
            context.informationCallback(T("New best: ") + state->toShortString() + T(" [") + String(score) + T("]"));
          }
        }
      }
      state->undoTransition(context, stateBackup);
      //context.leaveScope(true);
    }
  }


  struct Objective : public SimpleUnaryFunction
  {
    Objective(const BoostingLuapeLearner* pthis, ContainerPtr supervisions, DenseDoubleVectorPtr weights)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), pthis(pthis), supervisions(supervisions), weights(weights) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      LuapeRPNGraphBuilderStatePtr state = input.getObjectAndCast<LuapeRPNGraphBuilderState>();
      const LuapeGraphPtr& graph = state->getGraph();
      LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
      if (!yieldNode)
        return DBL_MAX; // non-terminal state
      LuapeNodeCachePtr yieldNodeCache = yieldNode->getCache();

      LuapeNodePtr valueNode = graph->getNode(yieldNode->getArgument());
      double score;
      if (yieldNodeCache->isScoreComputed())
        score = yieldNodeCache->getScore();
      else
      {
        score = pthis->computeWeakObjective(valueNode->getCache()->getExamples(), supervisions, weights);
        yieldNodeCache->setScore(score);
      }
      return -fabs(score);
    }

  protected:
    const BoostingLuapeLearner* pthis;
    ContainerPtr supervisions;
    DenseDoubleVectorPtr weights;
  };

  LuapeGraphPtr learnWeakModel(ExecutionContext& context, LuapeGraphPtr graph, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const
  {
    graph->getCache()->clearScores();
    FunctionPtr objective = new Objective(this, supervisions, weights);

    LuapeRPNGraphBuilderStatePtr state = new LuapeRPNGraphBuilderState(problem, graph, maxSteps);
    double bestScore = DBL_MAX;

    OptimizerPtr optimizer = new NestedMonteCarloOptimizer(state, 3, 1);
    OptimizerStatePtr optimizerState = optimizer->optimize(context, new OptimizationProblem(objective));

    LuapeGraphPtr bestGraph = optimizerState->getBestSolution().getObjectAndCast<LuapeRPNGraphBuilderState>()->getGraph();
    //exhaustiveSearch(context, state, objective, bestScore, bestGraph);
    context.informationCallback(String("Best Graph: ") + bestGraph->toShortString() + T(" [") + String(optimizerState->getBestScore()) + T("]"));
    context.informationCallback(String("Num cached nodes: ") + String((int)graph->getCache()->getNumCachedNodes()));
    return bestGraph;
  }
};

class AdaBoostLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostLuapeLearner(LuapeProblemPtr problem, size_t maxSteps, size_t maxIterations)
    : BoostingLuapeLearner(problem, maxSteps, maxIterations) {}
  AdaBoostLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeBinaryClassifierClass;}

  virtual VectorPtr createVoteVector() const
    {return new DenseDoubleVector(0, 0.0);}

  virtual Variable computeVote(double weakObjectiveValue) const
  {
    double accuracy = weakObjectiveValue + 0.5;
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log(accuracy / (1.0 - accuracy));
  }

  virtual DenseDoubleVectorPtr makeInitialWeights(const std::vector<ObjectPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual double computeWeakObjective(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions->getNumElements());
    jassert(numExamples == weights->getNumElements());

    double accuracy = 0.0;
    for (size_t i = 0; i < numExamples; ++i)
      if (predictions->getElement(i) == supervisions->getElement(i))
        accuracy += weights->getValue(i);
    return accuracy - 0.5;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0 || weakObjectiveValue == 0.5;}

  virtual double updateWeight(const Variable& prediction, const Variable& supervision, double currentWeight, const Variable& vote) const
  {
    double alpha = vote.toDouble();
    return currentWeight * exp(-alpha * (supervision.getBoolean() == prediction.getBoolean() ? 1.0 : -1.0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

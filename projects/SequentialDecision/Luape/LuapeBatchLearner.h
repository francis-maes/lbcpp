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

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const = 0;

  // the absolute value of this quantity should be maximized
  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const ContainerPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const = 0;
  virtual Variable computeVote(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeFunctionPtr& function = f.staticCast<LuapeFunction>();
    LuapeGraphPtr initialGraph = problem->createInitialGraph(context);
    DenseDoubleVectorPtr weights = makeInitialWeights(function, *(std::vector<PairPtr>* )&trainingData);
    function->setGraph(initialGraph->cloneAndCast<LuapeGraph>());
    VectorPtr supervisions;
    addExamplesToGraph(trainingData, function->getGraph(), supervisions);
    function->setVotes(createVoteVector(function));

    bool stopped = false;
    double weightsSum = 1.0;
    for (size_t i = 0; i < maxIterations && !stopped; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i+1));
      context.resultCallback(T("iteration"), i + 1);

      // weak graph completion
      LuapeGraphPtr newGraph = learnWeakModel(context, function, weights, supervisions);
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
        Variable vote = computeVote(function, cache->getExamples(), supervisions, weights, score);
        function->getVotes()->append(vote);

        // update weights
        weightsSum *= updateWeights(function, cache->getExamples(), supervisions, weights, vote);
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
  friend class BoostingLuapeLearnerClass;

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
      graph->addExample(example->getFirst().getObject());
      supervisions->setElement(i, example->getSecond());
    }
  }

  double updateWeights(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
  {
    jassert(predictions->getNumElements() == supervisions->getNumElements());
    size_t n = weights->getNumValues();

    double* values = weights->getValuePointer(0);
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      values[i] = updateWeight(function, i, values[i], predictions, supervisions, vote);
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
    Objective(const BoostingLuapeLearner* pthis, const LuapeFunctionPtr& function, ContainerPtr supervisions, DenseDoubleVectorPtr weights)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), pthis(pthis), function(function), supervisions(supervisions), weights(weights) {}

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
        score = pthis->computeWeakObjective(function, valueNode->getCache()->getExamples(), supervisions, weights);
        yieldNodeCache->setScore(score);
      }
      return -fabs(score);
    }

  protected:
    const BoostingLuapeLearner* pthis;
    const LuapeFunctionPtr& function;
    ContainerPtr supervisions;
    DenseDoubleVectorPtr weights;
  };

  LuapeGraphPtr learnWeakModel(ExecutionContext& context, const LuapeFunctionPtr& function, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->getCache()->clearScores();
    FunctionPtr objective = new Objective(this, function, supervisions, weights);

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

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const
    {return new DenseDoubleVector(0, 0.0);}

  virtual Variable computeVote(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const
  {
    double accuracy = weakObjectiveValue + 0.5;
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log(accuracy / (1.0 - accuracy));
  }

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
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


  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const ContainerPtr& predictions, const ContainerPtr& supervisions, const Variable& vote) const
  {
    double alpha = vote.toDouble();
    bool isPredictionCorrect = (supervisions->getElement(index).getBoolean() == predictions->getElement(index).getBoolean());
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }
};

class AdaBoostMHLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostMHLuapeLearner(LuapeProblemPtr problem, size_t maxSteps, size_t maxIterations)
    : BoostingLuapeLearner(problem, maxSteps, maxIterations) {}
  AdaBoostMHLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeClassifierClass;}

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    return new ObjectVector(classifier->getDoubleVectorClass());
  }

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t K = labels->getNumElements();
    size_t n = examples.size();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n * K, 1.0 / (2 * n * (K - 1))));
    double invZ = 1.0 / (2 * n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t k = (size_t)examples[i]->getSecond().getInteger();
      jassert(k >= 0 && k < K);
      res->setValue(i * K + k, invZ);
    }
    return res;
  }

  // the absolute value of this should be maximized
  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    size_t numLabels = classifier->getLabels()->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions->getNumElements());

    // compute mu_l-, mu_l+ and v_l values
    DenseDoubleVectorPtr muNegatives, muPositives, votes;
    computeMuAndVoteValues(function, predictions, supervisions, weights, muNegatives, muPositives, votes);

    // compute edge
    double edge = 0.0;
    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = predictions->getElement(i).getBoolean();
      size_t correct = (size_t)supervisions->getElement(i).getInteger();
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == (j == correct));
        double wij = *weightsPtr++;
        double vl = votes->getValue(j);
        edge += wij * vl * (isPredictionCorrect ? 1.0 : -1.0);
      }
    }
    return edge;
  }

  virtual Variable computeVote(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const
  {
    DenseDoubleVectorPtr muNegatives, muPositives, votes;
    computeMuAndVoteValues(function, predictions, supervisions, weights, muNegatives, muPositives, votes);
    double correctWeight = 0.0;
    double errorWeight = 0.0;
    size_t n = votes->getNumValues();
    for (size_t i = 0; i < n; ++i)
    {
      if (votes->getValue(i) > 0)
      {
        correctWeight += muPositives->getValue(i);
        errorWeight += muNegatives->getValue(i);
      }
      else
      {
        correctWeight += muNegatives->getValue(i);
        errorWeight += muPositives->getValue(i);
      }
    }
    double alpha = 0.5 * log(correctWeight / errorWeight);
    votes->multiplyByScalar(alpha);
    return votes;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0;}

  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const ContainerPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const
  {
    size_t numLabels = function.staticCast<LuapeClassifier>()->getLabels()->getNumElements();
    size_t example = index / numLabels;
    size_t k = index % numLabels;
    double alpha = vote.getObjectAndCast<DenseDoubleVector>()->getValue(k);
    bool isCorrectClass = (k == (size_t)supervision->getElement(example).getInteger());
    bool isPredictionCorrect = (prediction->getElement(example).getBoolean() == isCorrectClass);
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }

protected:
  void computeMuAndVoteValues(const LuapeFunctionPtr& function, const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, DenseDoubleVectorPtr& muNegatives, DenseDoubleVectorPtr& muPositives, DenseDoubleVectorPtr& votes) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    size_t numLabels = classifier->getLabels()->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions->getNumElements());

    muNegatives = new DenseDoubleVector(classifier->getDoubleVectorClass());
    muPositives = new DenseDoubleVector(classifier->getDoubleVectorClass());
    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = predictions->getElement(i).getBoolean();
      size_t correct = (size_t)supervisions->getElement(i).getInteger();
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == (j == correct));
        (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
      }
    }

    // compute v_l values
    votes = new DenseDoubleVector(classifier->getDoubleVectorClass());
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > muNegatives->getValue(i) ? 1.0 : -1.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

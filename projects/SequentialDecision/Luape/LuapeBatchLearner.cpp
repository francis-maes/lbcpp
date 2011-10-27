/*-----------------------------------------.---------------------------------.
| Filename: LuapeBatchLearner.cpp          | Luape Batch Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeBatchLearner.h"
using namespace lbcpp;

BoostingLuapeLearner::BoostingLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
  : LuapeBatchLearner(problem), weakLearner(weakLearner), maxIterations(maxIterations)
{
}

BoostingLuapeLearner::BoostingLuapeLearner() : maxIterations(0)
{
}

bool BoostingLuapeLearner::train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
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

  double lastLoss = 0.0;
  double lastTrain = 0.0;
  double lastValidation = 0.0;

  for (size_t i = 0; i < maxIterations && !stopped; ++i)
  {
    context.enterScope(T("Iteration ") + String((int)i+1));
    context.resultCallback(T("iteration"), i + 1);

    // weak graph completion
    LuapeGraphPtr newGraph = weakLearner->learn(context, refCountedPointerFromThis(this), function, supervisions, weights);
    if (newGraph)
    {
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
        BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
        edgeCalculator->initialize(function,  cache->getExamples(), supervisions, weights);
        Variable vote = edgeCalculator->computeVote();
        function->getVotes()->append(vote);

        // update weights
        weightsSum *= updateWeights(function, cache->getExamples(), supervisions, weights, vote);
        {
          DenseDoubleVectorPtr w = weights->cloneAndCast<DenseDoubleVector>();
          w->multiplyByScalar(weightsSum);
          context.resultCallback("loss", lastLoss = w->l1norm());
        }

        context.resultCallback("score", score);
        context.resultCallback("vote", vote);
        context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

        context.resultCallback(T("trainScore"), lastTrain = function->evaluate(context, trainingData, EvaluatorPtr(), "Train evaluation")->getScoreToMinimize());
        context.resultCallback(T("validationScore"), lastValidation = function->evaluate(context, validationData, EvaluatorPtr(), "Validation evaluation")->getScoreToMinimize());
      }
    }
    else
      context.informationCallback("Failed to find a weak learner");

    context.leaveScope();
    context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
  }
  context.informationCallback(T("Loss: ") + String(lastLoss) + T(" train: ") + String(lastTrain) + T(" validation: ") + String(lastValidation));
  return true;
}

void BoostingLuapeLearner::addExamplesToGraph(const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph, VectorPtr& supervisions) const
{
  size_t n = examples.size();
  graph->resizeExamples(n);
  supervisions = vector(examples[0]->getClass()->getTemplateArgument(1), n);
  for (size_t i = 0; i < n; ++i)
  {
    const PairPtr& example = examples[i].staticCast<Pair>();
    graph->setExample(i, example->getFirst().getObject());
    supervisions->setElement(i, example->getSecond());
  }
}

double BoostingLuapeLearner::updateWeights(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
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

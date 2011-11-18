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

/*
** BoostingEdgeCalculator
*/
double BoostingEdgeCalculator::findBestThreshold(ExecutionContext& context, LuapeNodePtr node, double& edge, bool verbose)
{
  edge = -DBL_MAX;
  double res = 0.0;

  if (verbose)
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
      double e = computeEdge();

      if (verbose)
      {
        context.enterScope("Iteration " + String((int)i));
        context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
        context.resultCallback("edge", e);
        context.leaveScope();
      }

      if (e > edge)
        edge = e, res = (threshold + previousThreshold) / 2.0;
      previousThreshold = threshold;
    }
    flipPrediction(sortedDoubleValues[i].first);
  }

  if (verbose)
    context.leaveScope();
  return res;
}

/*
** BoostingLuapeLearner
*/
BoostingLuapeLearner::BoostingLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
  : LuapeBatchLearner(problem), weakLearner(weakLearner), maxIterations(maxIterations)
{
}

BoostingLuapeLearner::BoostingLuapeLearner() : maxIterations(0)
{
}

bool BoostingLuapeLearner::train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  const LuapeInferencePtr& function = f.staticCast<LuapeInference>();
  LuapeGraphPtr graph = function->getGraph();
  if (!graph)
    function->setGraph(graph = problem->createInitialGraph(context));

  DenseDoubleVectorPtr weights = makeInitialWeights(function, *(std::vector<PairPtr>* )&trainingData);
  function->setGraph(graph);
  function->setVotes(function->createVoteVector(0));

  graph->resizeSamples(trainingData.size(), validationData.size());
  addExamplesToGraph(true, trainingData, graph);
  addExamplesToGraph(false, validationData, graph);

  VectorPtr trainingSupervisions = makeSupervisions(trainingData);
  VectorPtr validationSupervisions = makeSupervisions(validationData);

  VectorPtr trainingPredictions = function->createVoteVector(trainingData.size());
  VectorPtr validationPredictions = function->createVoteVector(validationData.size());

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
    std::vector<LuapeNodePtr> newNodes = weakLearner->learn(context, refCountedPointerFromThis(this), function, trainingSupervisions, weights);
    if (newNodes.size())
    {
      for (size_t i = 0; i < newNodes.size(); ++i)
        graph->pushNode(context, newNodes[i]);
      
      LuapeYieldNodePtr yieldNode = new LuapeYieldNode(newNodes.back());
      graph->pushNode(context, yieldNode);

      LuapeNodeCachePtr cache = newNodes.back()->getCache();
      BooleanVectorPtr weakPredictions = cache->getTrainingSamples();
      
      // compute vote
      BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
      edgeCalculator->initialize(function, weakPredictions, trainingSupervisions, weights);
      Variable vote = edgeCalculator->computeVote();
      function->getVotes()->append(vote);
      
      double edge = edgeCalculator->computeEdge();
      context.resultCallback("edge", edge);
      context.resultCallback("vote", vote);
      
      // stop test
      if (shouldStop(edge))
      {
        context.informationCallback(T("Stopping, edge = ") + String(edge));
        stopped = true;
      }
      else
      {
        // update weights
        weightsSum *= updateWeights(function, weakPredictions, trainingSupervisions, weights, vote);
        {
          DenseDoubleVectorPtr w = weights->cloneAndCast<DenseDoubleVector>();
          w->multiplyByScalar(weightsSum);
          context.resultCallback("loss", lastLoss = w->l1norm());
        }
      }
      //context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

      // update predictions and compute train/test score
      updatePredictions(function, trainingPredictions, cache->getTrainingSamples(), vote);
      updatePredictions(function, validationPredictions, cache->getValidationSamples(), vote);

      context.resultCallback(T("train error"), lastTrain = computeError(trainingPredictions, trainingSupervisions));
      context.resultCallback(T("validation error"), lastValidation = computeError(validationPredictions, validationSupervisions));
    }
    else
      context.informationCallback("Failed to find a weak learner");

    context.leaveScope();
    context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
  }
  context.informationCallback(T("Loss: ") + String(lastLoss) + T(" train: ") + String(lastTrain) + T(" validation: ") + String(lastValidation));
  return true;
}

void BoostingLuapeLearner::addExamplesToGraph(bool areTrainingSamples, const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph) const
{
  size_t n = examples.size();
  for (size_t i = 0; i < n; ++i)
  {
    const PairPtr& example = examples[i].staticCast<Pair>();
    graph->setSample(areTrainingSamples, i, example->getFirst().getObject());
  }
}

VectorPtr BoostingLuapeLearner::makeSupervisions(const std::vector<ObjectPtr>& examples) const
{
  size_t n = examples.size();
  VectorPtr res = vector(examples[0]->getClass()->getTemplateArgument(1), n);
  for (size_t i = 0; i < n; ++i)
  {
    const PairPtr& example = examples[i].staticCast<Pair>();
    res->setElement(i, example->getSecond());
  }
  return res;
}

double BoostingLuapeLearner::updateWeights(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
{
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

void BoostingLuapeLearner::updatePredictions(const LuapeInferencePtr& function, VectorPtr predictions, const BooleanVectorPtr& weakPredictions, const Variable& vote) const
{
  size_t n = predictions->getNumElements();
  jassert(n == weakPredictions->getNumElements());
  for (size_t i = 0; i < n; ++i)
  {
    Variable target = predictions->getElement(i);
    function->aggregateVote(target, vote, weakPredictions->get(i));
    predictions->setElement(i, target);
  }
}

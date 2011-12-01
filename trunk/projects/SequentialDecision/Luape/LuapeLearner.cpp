/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp          | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeLearner.h"
using namespace lbcpp;

/*
** LuapeLearner
*/
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  this->function = function;
  return true;
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  // we store a local copy to perform (fast) evaluation at each iteration
  if (isTrainingData)
  {
    trainingData = data;
    trainingSamples = function->createSamplesCache(context, data);
  }
  else
  {
    validationData = data;
    validationSamples = function->createSamplesCache(context, data);
  }
  return true;
}

VectorPtr LuapeLearner::getTrainingPredictions() const
{
  return trainingSamples->compute(defaultExecutionContext(), function->getRootNode(), NULL, false);
}

VectorPtr LuapeLearner::getValidationPredictions() const
{
  if (validationSamples)
    return validationSamples->compute(defaultExecutionContext(), function->getRootNode(), NULL, false);
  else
    return VectorPtr();
}

/*
** BoostingLearner
*/
BoostingLearner::BoostingLearner(BoostingWeakLearnerPtr weakLearner)
  : weakLearner(weakLearner) {}

bool BoostingLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  if (!LuapeLearner::initialize(context, function))
    return false;
  jassert(weakLearner);
  return weakLearner->initialize(context, function);
}

bool BoostingLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  if (!LuapeLearner::setExamples(context, isTrainingData, data))
    return false;


  //(isTrainingData ? predictions : validationPredictions) = function->makeCachedPredictions(context, isTrainingData);
  if (isTrainingData)
  {
    allExamples.resize(data.size());
    for (size_t i = 0; i < allExamples.size(); ++i)
      allExamples[i] = i;
  }
  return true;
}

LuapeNodePtr BoostingLearner::turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode) const
{
  Variable successVote, failureVote;
  computeVotes(context, weakNode, successVote, failureVote);
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote));
}

bool BoostingLearner::doLearningIteration(ExecutionContext& context)
{
  LuapeNodePtr weakNode;
  double weakObjective;
 
  // do weak learning
  {
    TimedScope _(context, "weak learning");
    weakNode = weakLearner->learn(context, refCountedPointerFromThis(this), allExamples, weakObjective);
    if (!weakNode)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return false;
    }
    context.resultCallback(T("weakNode"), weakNode);
    context.resultCallback(T("weakObjective"), weakObjective);
  }

  // turn into contribution
  {
    TimedScope _(context, "turn into contribution");
    LuapeNodePtr contribution = turnWeakNodeIntoContribution(context, weakNode);
    if (!contribution)
    {
      context.errorCallback(T("Failed to turn into contribution"));
      return false;
    }
    context.resultCallback(T("contribution"), contribution);

    // add contribution to sequence node
    std::vector<LuapeSamplesCachePtr> caches;
    caches.push_back(trainingSamples);
    if (validationSamples)
      caches.push_back(validationSamples);
    function->getRootNode().staticCast<LuapeSequenceNode>()->pushNode(contribution, caches);
  }

  // evaluate
  {
    TimedScope _(context, "evaluate");
    VectorPtr trainingPredictions = getTrainingPredictions();
    context.resultCallback(T("train error"), function->evaluatePredictions(context, trainingPredictions, trainingData));

    VectorPtr validationPredictions = getValidationPredictions();
    if (validationPredictions)
      context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
  }
  return true;
}
#if 0
LuapeNodePtr BoostingLearner::doWeakLearningAndAddToGraph(ExecutionContext& context)
{
  LuapeNodePtr weakNode;
  double weakObjective;

  // do weak learning
  {
    TimedScope _(context, "weak learning");
    weakNode = weakLearner->learn(context, refCountedPointerFromThis(this), allExamples, weakObjective);
    if (!weakNode)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return LuapeNodePtr();
    }
  }
 
  {
    TimedScope _(context, "add to graph");

    // add missing nodes to graph
    if (weakNode->getType() == booleanType || weakNode->getType() == probabilityType)
    {
      //jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
      LuapeNodePtr yieldSuccess = graph->makeYieldNode();
      LuapeNodePtr yieldFailure = graph->makeYieldNode();
      weakNode = new LuapeTestNode(weakNode, yieldSuccess, yieldFailure);
    }

    graph->pushMissingNodes(context, weakNode); // simple pushNode ?

    // update the weak learner
    //weakLearner->update(context, refCountedPointerFromThis(this), weakNode);

    context.resultCallback(T("numNodes"), graph->getNumNodes());
    context.resultCallback(T("numYields"), graph->getNumYieldNodes());
  }
  return weakNode;
}

class FillLuapeWeakPredictionVectorCallback : public LuapeGraphCallback
{
public:
  FillLuapeWeakPredictionVectorCallback(LuapeWeakPredictionVectorPtr res)
    : res(res), exampleIndex(0) {}

  LuapeWeakPredictionVectorPtr res;
  size_t exampleIndex;

  virtual void graphYielded(const LuapeYieldNodePtr& yieldNode, const Variable& value)
  {
    //graphYielded(index, 1.0);
  }

  ///virtual void graphYielded(size_t index, double value)
  //  {res->yield(exampleIndex, index, value);}
};

LuapeWeakPredictionVectorPtr BoostingLearner::makeWeakPredictions(ExecutionContext& context, const LuapeNodePtr& weakNode, bool useTrainingSamples) const
{
  size_t n = graph->getNumSamples(useTrainingSamples);
  LuapeWeakPredictionVectorPtr res = new LuapeWeakPredictionVector(n);
  FillLuapeWeakPredictionVectorCallback callback(res);
  for (size_t i = 0; i < n; ++i)
  {
    callback.exampleIndex = i;
    std::vector<Variable> state(graph->getNumNodes()); // tmp
    weakNode->compute(context, state, &callback);
  }
  return res;
}

void BoostingLearner::updatePredictionsAndEvaluate(ExecutionContext& context, size_t yieldIndex, const LuapeNodePtr& weakNode) const
{
  VectorPtr trainYields = graph->updateNodeCache(context, weakNode, true);
  function->updatePredictions(predictions, yieldIndex, trainYields);
  context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainingData));

  if (validationPredictions)
  {
    VectorPtr validationYields = graph->updateNodeCache(context, weakNode, false);
    function->updatePredictions(validationPredictions, yieldIndex, validationYields);
    context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
  }
}

void BoostingLearner::recomputePredictions(ExecutionContext& context)
{
  if (graph->getNumTrainingSamples() > 0)
    predictions = function->makeCachedPredictions(context, true);
  if (graph->getNumValidationSamples() > 0)
    validationPredictions = function->makeCachedPredictions(context, false);
}

double BoostingLearner::getSignedScalarPrediction(const VectorPtr& predictions, size_t index) const
{
  BooleanVectorPtr booleanPredictions = predictions.dynamicCast<BooleanVector>();
  if (booleanPredictions)
    return booleanPredictions->get(index) ? 1.0 : -1.0;
  else
    return predictions.staticCast<DenseDoubleVector>()->getValue(index) * 2.0 - 1.0;
}
#endif // 0

/*
** BoostingWeakObjective
*/
double BoostingWeakObjective::compute(const VectorPtr& predictions)
{
  setPredictions(predictions);
  return computeObjective();
}

double BoostingWeakObjective::findBestThreshold(ExecutionContext& context, const SparseDoubleVectorPtr& sortedDoubleValues, double& edge, bool verbose)
{
  setPredictions(new BooleanVector(sortedDoubleValues->getNumValues(), true));

  edge = -DBL_MAX;
  double res = 0.0;

  if (verbose)
    context.enterScope("Find best threshold for node");

  jassert(sortedDoubleValues->getNumValues());
  double previousThreshold = sortedDoubleValues->getValue(0).second;
  for (size_t i = 0; i < sortedDoubleValues->getNumValues(); ++i)
  {
    double threshold = sortedDoubleValues->getValue(i).second;

    jassert(threshold >= previousThreshold);
    if (threshold > previousThreshold)
    {
      double e = computeObjective();

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
    flipPrediction(sortedDoubleValues->getValue(i).first);
  }

  if (verbose)
    context.leaveScope();
  return res;
}

/*
** BoostingWeakLearner
*/
double BoostingWeakLearner::computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
{
  if (weakNode->getType() == booleanType)
    return computeWeakObjective(context, structureLearner, weakNode, examples);
  else
  {
    jassert(weakNode->getType()->isConvertibleToDouble());
    double threshold;
    double res = computeWeakObjectiveWithStump(context, structureLearner, weakNode, examples, threshold);
    weakNode = makeStump(structureLearner, weakNode, threshold);
    return res;
  }
}

double BoostingWeakLearner::computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
{
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective(examples);
  VectorPtr weakPredictions = structureLearner->getTrainingSamples()->compute(context, weakNode);
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return edgeCalculator->compute(weakPredictions);
}

double BoostingWeakLearner::computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const std::vector<size_t>& examples, double& bestThreshold) const
{
  SparseDoubleVectorPtr sortedDoubleValues;
  structureLearner->getTrainingSamples()->compute(context, numberNode, &sortedDoubleValues);
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective(examples);
  double edge;
  bestThreshold = edgeCalculator->findBestThreshold(context, sortedDoubleValues, edge, false);
  return edge;
}

LuapeNodePtr BoostingWeakLearner::makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const
  {return structureLearner->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);}

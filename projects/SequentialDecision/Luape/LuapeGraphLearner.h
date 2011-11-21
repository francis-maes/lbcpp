/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphLearner.h            | Luape Graph Learners            |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_H_

# include "LuapeInference.h"
# include "LuapeGraphBuilder.h"
# include "LuapeProblem.h"

# include "LuapeBatchLearner.h" // tmp, for EdgeCalculator
# include "AdaBoostMHLuapeLearner.h"

namespace lbcpp
{

class LuapeGraphLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual bool doLearningIteration(ExecutionContext& context) = 0;

  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const = 0;

protected:
  LuapeProblemPtr problem;
  LuapeInferencePtr function;
  LuapeGraphPtr graph;
};

typedef ReferenceCountedObjectPtr<LuapeGraphLearner> LuapeGraphLearnerPtr;

class LuapeGradientBoostingLearner : public LuapeGraphLearner
{
public:
  LuapeGradientBoostingLearner(double learningRate, size_t maxDepth);
  LuapeGradientBoostingLearner() : learningRate(0.0), maxDepth(maxDepth) {}

  // gradient boosting
  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const = 0;
  virtual bool addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, LuapeNodePtr completion);

  // LuapeGraphLearner
  virtual bool doLearningIteration(ExecutionContext& context);
  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const;

protected:
  double learningRate;
  size_t maxDepth;

  DenseDoubleVectorPtr pseudoResiduals;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;

class LuapeBoostingLearner : public LuapeGraphLearner
{
public:
  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const = 0;

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const = 0;

  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;

  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context) const
  {
    // todo: externalize weak learner
    
    LuapeNodePtr res;
    double bestEdge = -DBL_MAX;
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      TypePtr type = graph->getNodeType(i);
      if (type == booleanType || type == doubleType)
      {
        double edge = computeCompletionReward(context, graph->getNode(i));
        if (edge > bestEdge)
          bestEdge = edge, res = graph->getNode(i);
      }
    }
    return res;
  }

  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    if (!LuapeGraphLearner::setExamples(context, isTrainingData, data))
      return false;
      
    size_t n = data.size();
    graph->resizeSamples(isTrainingData, n);
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = data[i].staticCast<Pair>();
      graph->setSample(isTrainingData, i, example->getFirst().getObject());
    }

    if (isTrainingData)
    {
      weights = makeInitialWeights(function, *(std::vector<PairPtr>* )&data);
      weightsSum = 1.0;
      supervisions = makeSupervisions(data);
    }
    return true;
  }
  
  virtual bool doLearningIteration(ExecutionContext& context)
  {
    LuapeNodePtr weakNode = doWeakLearning(context);
    if (!weakNode)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return false;
    }
    
    if (weakNode->getType() != booleanType)
      weakNode = createDecisionStump(context, weakNode);
    
    jassert(weakNode->getType() == booleanType);
    LuapeYieldNodePtr yieldNode = new LuapeYieldNode(weakNode);
    graph->pushMissingNodes(context, yieldNode);

    weakNode->updateCache(context, true);
    LuapeNodeCachePtr cache = weakNode->getCache();
    BooleanVectorPtr weakPredictions = cache->getTrainingSamples();
    
    // compute vote
    BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
    edgeCalculator->initialize(function, weakPredictions, supervisions, weights);
    Variable vote = edgeCalculator->computeVote();
    function->getVotes()->append(vote);
    
    double edge = edgeCalculator->computeEdge();
    context.resultCallback("edge", edge);
    context.resultCallback("vote", vote);
    
    // stop test
    bool stopped = false;
    if (shouldStop(edge))
    {
      context.informationCallback(T("Stopping, edge = ") + String(edge));
      stopped = true;
    }
    else
    {
      // update weights
      weightsSum *= updateWeights(function, weakPredictions, supervisions, weights, vote);
      {
        DenseDoubleVectorPtr w = weights->cloneAndCast<DenseDoubleVector>();
        w->multiplyByScalar(weightsSum);
        context.resultCallback("loss", w->l1norm());
      }
    }
    context.informationCallback(T("Graph: ") + graph->toShortString());
    //context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

    // update predictions and compute train/test score
//    updatePredictions(function, predictions, cache->getTrainingSamples(), vote);
//    updatePredictions(function, validationPredictions, cache->getValidationSamples(), vote);

//    context.resultCallback(T("train error"), computeError(trainingPredictions, trainingSupervisions));
  //  context.resultCallback(T("validation error"), lastValidation = computeError(validationPredictions, validationSupervisions));
    return !stopped;
  }
  
  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const
  {
    BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
    
    if (completion->getType() == booleanType)
    {
      BooleanVectorPtr weakPredictions = completion->getCache()->getTrainingSamples().staticCast<BooleanVector>();
      edgeCalculator->initialize(function, weakPredictions, supervisions, weights);
      return edgeCalculator->computeEdge();
      
    }
    else
    {
      jassert(completion->getType()->isConvertibleToDouble());
      double edge;
      edgeCalculator->initialize(function, new BooleanVector(graph->getNumTrainingSamples(), true), supervisions, weights);
      edgeCalculator->findBestThreshold(context, completion, edge, false);
      return edge;
    }
  }
  
  
protected:
  DenseDoubleVectorPtr weights;
  VectorPtr supervisions;
  double weightsSum;

  LuapeNodePtr createDecisionStump(ExecutionContext& context, const LuapeNodePtr& inputNode) const
  {
    BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
    double edge;
    edgeCalculator->initialize(function, new BooleanVector(graph->getNumTrainingSamples(), true), supervisions, weights);
    double threshold = edgeCalculator->findBestThreshold(context, inputNode, edge, false);
    return graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), inputNode);
  }
      
  double updateWeights(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
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

  void updatePredictions(const LuapeInferencePtr& function, VectorPtr predictions, const BooleanVectorPtr& weakPredictions, const Variable& vote) const
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
};

class LuapeAdaBoostMHLearner : public LuapeBoostingLearner
{
public:
  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const
    {return new AdaBoostMHEdgeCalculator();}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = examples.size();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n * numLabels, 1.0 / (2 * n * (numLabels - 1))));
    double invZ = 1.0 / (2 * n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t k = (size_t)examples[i]->getSecond().getInteger();
      jassert(k >= 0 && k < numLabels);
      res->setValue(i * numLabels + k, invZ);
    }
    return res;
  }

  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const
  {
    EnumerationPtr labels = examples[0]->getClass()->getTemplateArgument(1).staticCast<Enumeration>();
    size_t n = examples.size();
    size_t m = labels->getNumElements();
    BooleanVectorPtr res = new BooleanVector(n * m);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = examples[i].staticCast<Pair>();
      size_t label = (size_t)example->getSecond().getInteger();
      for (size_t j = 0; j < m; ++j, ++index)
        res->set(index, j == label);
    }
    return res;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0;}

  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const
  {
    size_t numLabels = function.staticCast<LuapeClassifier>()->getLabels()->getNumElements();
    size_t example = index / numLabels;
    size_t k = index % numLabels;
    double alpha = vote.getObjectAndCast<DenseDoubleVector>()->getValue(k);
    
    bool isCorrectClass = supervision.staticCast<BooleanVector>()->get(index);
    bool isPredictionCorrect = (prediction->get(example) == isCorrectClass);
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_H_

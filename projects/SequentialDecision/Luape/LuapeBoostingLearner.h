/*-----------------------------------------.---------------------------------.
| Filename: LuapeBoostingLearner.h         | Boosting Learner base class     |
| Author  : Francis Maes                   |                                 |
| Started : 21/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_BOOSTING_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_BOOSTING_H_

# include "LuapeGraphLearner.h"

namespace lbcpp
{

class BoostingEdgeCalculator : public Object
{
public:
  virtual void initialize(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) = 0;
  virtual void flipPrediction(size_t index) = 0;
  virtual double computeEdge() const = 0;
  virtual Variable computeVote() const = 0;
  
  double findBestThreshold(ExecutionContext& context, LuapeNodePtr node, double& edge, bool verbose = false) // this modifies the prediction vector
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
};

typedef ReferenceCountedObjectPtr<BoostingEdgeCalculator> BoostingEdgeCalculatorPtr;

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

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_BOOSTING_H_

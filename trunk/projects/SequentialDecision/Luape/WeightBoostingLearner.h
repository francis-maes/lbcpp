/*-----------------------------------------.---------------------------------.
| Filename: WeightBoostingLearner.h        | Base class AdaBoost style       |
| Author  : Francis Maes                   |  algorithms                     |
| Started : 21/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_

# include "LuapeLearner.h"

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

class WeightBoostingLearner : public BoostingLearner
{
public:
  WeightBoostingLearner(BoostingWeakLearnerPtr weakLearner)
   : BoostingLearner(weakLearner) {}
  WeightBoostingLearner() {}

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const = 0;

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const = 0;
  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;

  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const = 0;

  virtual bool shouldStop(double weakObjectiveValue) const = 0;

  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    if (!BoostingLearner::setExamples(context, isTrainingData, data))
      return false;
     
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
    BooleanVectorPtr weakPredictions;
    LuapeNodePtr weakNode = doWeakLearningAndAddToGraph(context, weakPredictions);
    if (!weakNode)
      return false;
    
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
      context.resultCallback("loss", weights->l1norm() * weightsSum);
    }
    //context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

    // update predictions and compute train/test score
    updatePredictionsAndEvaluate(context, graph->getNumYieldNodes() - 1, weakNode);
    return !stopped;
  }

  virtual double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& weakNode) const
  {
    BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
    VectorPtr weakPredictions = graph->updateNodeCache(context, weakNode, true);
    
    if (weakNode->getType() == booleanType)
    {
      edgeCalculator->initialize(function, weakPredictions.staticCast<BooleanVector>(), supervisions, weights);
      return edgeCalculator->computeEdge();
      
    }
    else
    {
      jassert(weakNode->getType()->isConvertibleToDouble());
      double edge;
      edgeCalculator->initialize(function, new BooleanVector(graph->getNumTrainingSamples(), true), supervisions, weights);
      edgeCalculator->findBestThreshold(context, weakNode, edge, false);
      return edge;
    }
  }
  
  virtual double computeBestStumpThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode) const
  {
    BoostingEdgeCalculatorPtr edgeCalculator = createEdgeCalculator();
    double edge;
    edgeCalculator->initialize(function, new BooleanVector(graph->getNumTrainingSamples(), true), supervisions, weights);
    graph->updateNodeCache(context, numberNode, true);
    return edgeCalculator->findBestThreshold(context, numberNode, edge, false);
  }

protected:
  DenseDoubleVectorPtr weights;
  VectorPtr supervisions;
  double weightsSum;

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
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

extern WeightBoostingLearnerPtr adaBoostMHLearner(BoostingWeakLearnerPtr weakLearner);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_

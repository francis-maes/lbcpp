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

class WeightBoostingLearner : public BoostingLearner
{
public:
  WeightBoostingLearner(BoostingWeakLearnerPtr weakLearner)
   : BoostingLearner(weakLearner) {}
  WeightBoostingLearner() {}

  // new 
  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const VectorPtr& predictions, double& loss) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const = 0;
  
  // old
  //virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const = 0;
  //virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const VectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;
  //virtual bool shouldStop(double weakObjectiveValue) const = 0;
  //virtual Variable computeVote(BoostingWeakObjectivePtr edgeCalculator) const = 0;

  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    if (!BoostingLearner::setExamples(context, isTrainingData, data))
      return false;
     
    if (isTrainingData)
    {
      //weights = makeInitialWeights(function, *(std::vector<PairPtr>* )&data);
      //weightsSum = 1.0;
      supervisions = makeSupervisions(data);
    }
    return true;
  }

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    double loss;
    weights = computeSampleWeights(context, getTrainingPredictions(), loss);
    context.resultCallback(T("loss"), loss);
    return BoostingLearner::doLearningIteration(context);
  }
#if 0
  virtual bool doLearningIteration(ExecutionContext& context)
  {
    LuapeNodePtr weakNode = doWeakLearningAndAddToGraph(context);
    if (!weakNode)
      return false;
    
    // compute vote
    LuapeWeakPredictionVectorPtr weakPredictions = makeWeakPredictions(context, weakNode);
    BoostingWeakObjectivePtr edgeCalculator = createWeakObjective(allExamples);
    edgeCalculator->setPredictions(weakPredictions);
    Variable vote = computeVote(edgeCalculator);
    function->getVotes()->append(vote);
    
    double edge = edgeCalculator->computeObjective();
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
      context.resultCallback("loss", weightsSum);
    }
    //context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());

    // update predictions and compute train/test score
    updatePredictionsAndEvaluate(context, graph->getNumYieldNodes() - 1, weakNode);
    return !stopped;
  }
#endif // 0

protected:
  DenseDoubleVectorPtr weights;
  VectorPtr supervisions;
  //double weightsSum;
/*
  double updateWeights(const LuapeInferencePtr& function, const VectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const
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
  }*/
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

extern WeightBoostingLearnerPtr adaBoostMHLearner(BoostingWeakLearnerPtr weakLearner);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_

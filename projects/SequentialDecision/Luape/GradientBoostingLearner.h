/*-----------------------------------------.---------------------------------.
| Filename: GradientBoostingLearner.h      | Luape Gradient Boosting Learner |
| Author  : Francis Maes                   |  base class                     |
| Started : 21/11/2011 15:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_

# include "LuapeLearner.h"
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class GradientBoostingLearner : public BoostingLearner
{
public:
  GradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate)
    : BoostingLearner(weakLearner), learningRate(learningRate)
  {
  }  
  GradientBoostingLearner() : learningRate(0.0) {}

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const = 0;

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    // 1- compute pseudo residuals 
    {
      TimedScope _(context, "compute residuals");

      double lossValue;
      computeLoss(predictions.staticCast<DenseDoubleVector>(), &lossValue, &pseudoResiduals);
      context.resultCallback(T("loss"), lossValue);
      context.resultCallback(T("predictions"), predictions);
      context.resultCallback(T("pseudoResiduals"), pseudoResiduals);
    }

    // 2- find best weak learner
    BooleanVectorPtr weakPredictions;
    LuapeNodePtr weakNode = doWeakLearningAndAddToGraph(context, weakPredictions);
    if (!weakNode)
      return false;

    // 3- add weak learner to graph
    {
      TimedScope _(context, "optimize weight");
      double optimalWeight = optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
      function->getVotes()->append(optimalWeight * learningRate);
    }

    // 4- update predictions and evaluate
    updatePredictionsAndEvaluate(context, function->getNumYields() - 1, weakNode);
    return true;
  }

  virtual double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& completion) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    completion->updateCache(context, true);

    ContainerPtr samples = completion->getCache()->getTrainingSamples();
    DenseDoubleVectorPtr scalars = samples.dynamicCast<DenseDoubleVector>();
    if (scalars)
    {
      jassert(false); // FIXME: evaluate quality of decision stump
      return 0.0;
    }
    else
    {
      std::vector<bool>::const_iterator it = samples.staticCast<BooleanVector>()->getElements().begin();

      ScalarVariableMeanAndVariance trainPositive;
      ScalarVariableMeanAndVariance validationPositive;
      ScalarVariableMeanAndVariance trainNegative;
      ScalarVariableMeanAndVariance validationNegative;

      for (size_t i = 0; i < pseudoResiduals->getNumValues(); ++i)
      {
        bool isPositive = *it++;

        double value = pseudoResiduals->getValue(i);
        double weight = 1.0; // fabs(value)

        //if (random->sampleBool(p))
          (isPositive ? trainPositive : trainNegative).push(value, weight);
        //else
          (isPositive ? validationPositive : validationNegative).push(value, weight);
      }
      
      double meanSquareError = 0.0;
      if (validationPositive.getCount())
        meanSquareError += validationPositive.getCount() * (trainPositive.getSquaresMean() + validationPositive.getSquaresMean() 
                                                              - 2 * trainPositive.getMean() * validationPositive.getMean());
      if (validationNegative.getCount())
        meanSquareError += validationNegative.getCount() * validationNegative.getSquaresMean(); // when negative, we always predict 0
  //      meanSquareError += validationNegative.getCount() * (trainNegative.getSquaresMean() + validationNegative.getSquaresMean() 
  //                                                            - 2 * trainNegative.getMean() * validationNegative.getMean());

      if (validationPositive.getCount() || validationNegative.getCount())
        meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
      
      return 1.0 - sqrt(meanSquareError) / 2.0; // ... bring into [0,1]
    }
  }

  virtual double computeBestStumpThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode) const
  {
    jassert(false); // todo: implement
    return 0.0;
  }

  virtual double optimizeWeightOfWeakLearner(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, const BooleanVectorPtr& weakPredictions) const
  {
    context.enterScope(T("Optimize weight"));

    double bestLoss = DBL_MAX;
    double bestWeight = 0.0;

    for (double K = -2.5; K <= 2.5; K += 0.02)
    {
      context.enterScope(T("K = ") + String(K));
      context.resultCallback(T("K"), K);

      DenseDoubleVectorPtr newPredictions = predictions->cloneAndCast<DenseDoubleVector>();
      for (size_t i = 0; i < weakPredictions->getNumElements(); ++i)
        if (weakPredictions->get(i))
          newPredictions->incrementValue(i, K);
      double lossValue;
      computeLoss(newPredictions, &lossValue, NULL);

      if (lossValue < bestLoss)
      {
        bestLoss = lossValue;
        bestWeight = K;
      }

      context.resultCallback(T("loss"), lossValue);
      context.leaveScope(lossValue);
    }

    context.leaveScope(bestLoss);
    return bestWeight;
  }
protected:
  double learningRate;
  DenseDoubleVectorPtr pseudoResiduals;
};

typedef ReferenceCountedObjectPtr<GradientBoostingLearner> GradientBoostingLearnerPtr;

extern GradientBoostingLearnerPtr l2BoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate);
extern GradientBoostingLearnerPtr rankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate, RankingLossFunctionPtr rankingLoss);

class L2BoostingLearner : public GradientBoostingLearner
{
public:
  L2BoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate)
    : GradientBoostingLearner(weakLearner, learningRate) {}
  L2BoostingLearner() {}

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  { 
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    size_t n = trainData.size();
    jassert(n == predictions->getNumValues());
    for (size_t i = 0; i < n; ++i)
    {
      double predicted = predictions->getValue(i);
      double correct = trainData[i].staticCast<Pair>()->getSecond().getDouble();

      if (lossValue)
        *lossValue += (predicted - correct) * (predicted - correct);
      if (lossGradient)
        (*lossGradient)->setValue(i, correct - predicted);
    }
    if (lossValue)
      *lossValue /= n;
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

  virtual double optimizeWeightOfWeakLearner(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, const BooleanVectorPtr& weakPredictions) const
  {
    // todo: specialized version
    return GradientBoostingLearner::optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
  }
};

class RankingGradientBoostingLearner : public GradientBoostingLearner
{
public:
  RankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate, RankingLossFunctionPtr rankingLoss)
    : GradientBoostingLearner(weakLearner, learningRate), rankingLoss(rankingLoss) {}
  RankingGradientBoostingLearner() {}

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  {
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    size_t index = 0;
    for (size_t i = 0; i < trainData.size(); ++i)
    {
      const PairPtr& example = trainData[i].staticCast<Pair>();

      size_t n = example->getFirst().getObjectAndCast<Container>()->getNumElements();
      DenseDoubleVectorPtr costs = example->getSecond().getObjectAndCast<DenseDoubleVector>();

      DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
      memcpy(scores->getValuePointer(0), predictions->getValuePointer(index), sizeof (double) * n);

      double v = 0.0;
      DenseDoubleVectorPtr g = lossGradient ? new DenseDoubleVector(n, 0.0) : DenseDoubleVectorPtr();
      rankingLoss->computeRankingLoss(scores, costs, lossValue ? &v : NULL, lossGradient ? &g : NULL, 1.0);
      if (lossValue)
        *lossValue += v;
      if (g)      
        memcpy((*lossGradient)->getValuePointer(index), g->getValuePointer(0), sizeof (double) * n);
      index += n;
    }
    if (lossValue)
      *lossValue /= trainData.size();
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

protected:
  friend class RankingGradientBoostingLearnerClass;

  RankingLossFunctionPtr rankingLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_

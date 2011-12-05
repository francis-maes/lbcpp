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

class L2BoostingWeakObjective : public BoostingWeakObjective
{
public:
  L2BoostingWeakObjective(const DenseDoubleVectorPtr& targets, const std::vector<size_t>& examples)
    : targets(targets), examples(examples) {}

  virtual void setPredictions(const VectorPtr& predictions)
  {
    this->predictions = predictions;
    positives.clear();
    negatives.clear();
    missings.clear();

    BooleanVectorPtr booleanPredictions = predictions.dynamicCast<BooleanVector>();
    if (booleanPredictions)
    {
      const unsigned char* predictionsPtr = booleanPredictions->getData();
      for (size_t i = 0; i < examples.size(); ++i)
      {
        double value = targets->getValue(examples[i]);
        unsigned char prediction = predictionsPtr[examples[i]];
        switch (prediction)
        {
        case 0: negatives.push(value); break;
        case 1: positives.push(value); break;
        case 2: missings.push(value); break;
        default: jassert(false);
        }
      }
    }
    else
    {
      DenseDoubleVectorPtr scalarPredictions = predictions.dynamicCast<DenseDoubleVector>();
      for (size_t i = 0; i < examples.size(); ++i)
      {
        double value = targets->getValue(examples[i]);
        double prediction = scalarPredictions->getValue(examples[i]);
        if (prediction == doubleMissingValue)
          missings.push(value);
        else if (prediction > 0)
          positives.push(value);
        else
          negatives.push(value);
      }
    }
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(predictions.isInstanceOf<BooleanVector>());
    unsigned char& prediction = predictions.staticCast<BooleanVector>()->getData()[index]; // fast unprotected access
    if (prediction < 2)
    {
      prediction = 1 - prediction;
    
      double value = targets->getValue(index);
      if (prediction > 0)
      {
        negatives.push(value, -1.0);
        positives.push(value);
      }
      else
      {
        positives.push(value, -1.0);
        negatives.push(value);
      }
    }
  }

  virtual double computeObjective() const
  {
    double res = 0.0;
    if (positives.getCount())
      res += positives.getCount() * positives.getVariance();
    if (negatives.getCount())
      res += negatives.getCount() * negatives.getVariance();
    if (missings.getCount())
      res += missings.getCount() * missings.getVariance();
    jassert(examples.size() == (size_t)(positives.getCount() + negatives.getCount() + missings.getCount()));
    if (examples.size())
      res /= (double)examples.size();
    return -res;
  }

  double getPositivesMean() const
    {return positives.getMean();}

  double getNegativesMean() const
    {return negatives.getMean();}
    
  double getMissingsMean() const
    {return missings.getMean();}

protected:
  DenseDoubleVectorPtr targets;
  VectorPtr predictions;
  const std::vector<size_t>& examples;

  ScalarVariableMeanAndVariance positives;
  ScalarVariableMeanAndVariance negatives;
  ScalarVariableMeanAndVariance missings;
};

typedef ReferenceCountedObjectPtr<L2BoostingWeakObjective> L2BoostingWeakObjectivePtr;

class GradientBoostingLearner : public BoostingLearner
{
public:
  GradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate)
    : BoostingLearner(weakLearner), learningRate(learningRate) {}
  GradientBoostingLearner() : learningRate(0.0) {}

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const = 0;

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    {
      TimedScope _(context, "compute residuals");

      double lossValue;
      DenseDoubleVectorPtr predictions = getTrainingPredictions().staticCast<DenseDoubleVector>();
      computeLoss(predictions, &lossValue, &pseudoResiduals);
      context.resultCallback(T("loss"), lossValue);
      //context.resultCallback(T("predictions"), predictions);
      //context.resultCallback(T("pseudoResiduals"), pseudoResiduals);
    }
    return BoostingLearner::doLearningIteration(context);
  }

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const
    {return new L2BoostingWeakObjective(pseudoResiduals, examples);}

  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    jassert(false); // broken
    LuapeSequenceNodePtr sequence = function->getRootNode().staticCast<LuapeSequenceNode>();
    VectorPtr predictions = getTrainingPredictions();

    context.enterScope(T("Optimize weight"));

    double bestLoss = DBL_MAX;
    double bestWeight = 0.0;

    // FIXME: improve this !
    for (double K = -2.5; K <= 2.5; K += 0.02)
    {
      context.enterScope(T("K = ") + String(K));
      context.resultCallback(T("K"), K);

      DenseDoubleVectorPtr newPredictions = predictions->cloneAndCast<DenseDoubleVector>();

      jassert(false); // FIXME
      //size_t n = weakPredictions->getNumElements();
      //for (size_t i = 0; i < n; ++i)
      //  newPredictions->incrementValue(i, K * getSignedScalarPrediction(weakPredictions, i));

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
    successVote = bestWeight;
    failureVote = -bestWeight;
    return true;
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
  
    size_t n = trainingData.size();
    jassert(n == predictions->getNumValues());
    for (size_t i = 0; i < n; ++i)
    {
      double predicted = predictions->getValue(i);
      double correct = trainingData[i].staticCast<Pair>()->getSecond().getDouble();
      double delta = predicted - correct;

      if (lossValue)
        *lossValue += delta * delta; 
      if (lossGradient)
        (*lossGradient)->setValue(i, delta);
    }
    if (lossValue)
      *lossValue /= n;
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    L2BoostingWeakObjectivePtr objective(new L2BoostingWeakObjective(pseudoResiduals, examples));
    objective->setPredictions(trainingSamples->compute(context, weakNode));
    successVote = objective->getPositivesMean();
    failureVote = objective->getNegativesMean();
    missingVote = objective->getMissingsMean();
    return true;
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
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      const PairPtr& example = trainingData[i].staticCast<Pair>();

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
      *lossValue /= trainingData.size();
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

protected:
  friend class RankingGradientBoostingLearnerClass;

  RankingLossFunctionPtr rankingLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_

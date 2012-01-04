/*-----------------------------------------.---------------------------------.
| Filename: GradientBoostingLearner.h      | Luape Gradient Boosting Learner |
| Author  : Francis Maes                   |  base class                     |
| Started : 21/11/2011 15:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_

# include "BoostingLearner.h"

namespace lbcpp
{

class GradientBoostingLearner : public BoostingLearner
{
public:
  GradientBoostingLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, double learningRate, size_t treeDepth)
    : BoostingLearner(new RegressionLearningObjective(), weakLearner, maxIterations, treeDepth), learningRate(learningRate) {}
  GradientBoostingLearner() : learningRate(0.0) {}

  virtual void computeLoss(const LuapeInferencePtr& problem, const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const = 0;

  virtual bool doLearningIteration(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    {
      TimedScope _(context, "compute residuals");

      double lossValue;
      DenseDoubleVectorPtr predictions = problem->getTrainingPredictions().staticCast<DenseDoubleVector>();
      DenseDoubleVectorPtr pseudoResiduals;
      computeLoss(problem, predictions, &lossValue, &pseudoResiduals);
      context.resultCallback(T("loss"), lossValue);
      objective->setSupervisions(pseudoResiduals);
      //context.resultCallback(T("predictions"), predictions);
      //context.resultCallback(T("pseudoResiduals"), pseudoResiduals);
    }
    return BoostingLearner::doLearningIteration(context, node, problem, examples, trainingScore, validationScore);
  }

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const
  {
    LuapeSequenceNodePtr sequence = problem->getRootNode().staticCast<LuapeSequenceNode>();
    VectorPtr predictions = problem->getTrainingPredictions();

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
      computeLoss(problem, newPredictions, &lossValue, NULL);

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
    missingVote = 0.0;
    return true;
  }

protected:
  double learningRate;
};

typedef ReferenceCountedObjectPtr<GradientBoostingLearner> GradientBoostingLearnerPtr;

class L2BoostingLearner : public GradientBoostingLearner
{
public:
  L2BoostingLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, double learningRate, size_t treeDepth)
    : GradientBoostingLearner(weakLearner, maxIterations, learningRate, treeDepth) {}
  L2BoostingLearner() {}

  virtual void computeLoss(const LuapeInferencePtr& problem, const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  { 
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions().staticCast<DenseDoubleVector>();

    size_t n = supervisions->getNumValues();
    jassert(n == predictions->getNumValues());
    for (size_t i = 0; i < n; ++i)
    {
      double delta = predictions->getValue(i) - supervisions->getValue(i);

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

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const
  {
    RegressionLearningObjectivePtr objective = this->objective.staticCast<RegressionLearningObjective>();
    objective->setPredictions(weakPredictions);
    successVote = objective->getPositivesMean();
    failureVote = objective->getNegativesMean();
    missingVote = objective->getMissingsMean();
    return true;
  }
};

class RankingGradientBoostingLearner : public GradientBoostingLearner
{
public:
  RankingGradientBoostingLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, double learningRate, RankingLossFunctionPtr rankingLoss, size_t treeDepth)
    : GradientBoostingLearner(weakLearner, maxIterations, learningRate, treeDepth), rankingLoss(rankingLoss) {}
  RankingGradientBoostingLearner() {}

  virtual void computeLoss(const LuapeInferencePtr& problem, const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  {
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions().staticCast<DenseDoubleVector>();

    const std::vector<size_t>& exampleSizes = problem.staticCast<LuapeRanker>()->getTrainingExampleSizes();
    size_t index = 0;
    for (size_t i = 0; i < exampleSizes.size(); ++i)
    {
      size_t n = exampleSizes[i];

      DenseDoubleVectorPtr costs = new DenseDoubleVector(n, 0.0);
      memcpy(costs->getValuePointer(0), supervisions->getValuePointer(index), sizeof (double) * n);
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
      *lossValue /= exampleSizes.size();
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

protected:
  friend class RankingGradientBoostingLearnerClass;

  RankingLossFunctionPtr rankingLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GRADIENT_BOOSTING_H_

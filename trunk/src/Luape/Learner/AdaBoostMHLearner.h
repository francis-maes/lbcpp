/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostMHLearner.h            | AdaBoost.MH learner             |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_
# define LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_

# include "WeightBoostingLearner.h"

namespace lbcpp
{

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : WeightBoostingLearner(new ClassificationLearningObjective(), weakLearner, maxIterations, treeDepth) {}
  AdaBoostMHLearner() {}

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
    {return new LuapeVectorSumNode(problem.staticCast<LuapeClassifier>()->getLabels());}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& loss) const
  {
    ObjectVectorPtr predictions = problem->getTrainingPredictions().staticCast<ObjectVector>();

    const LuapeClassifierPtr& classifier = problem.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = predictions->getNumElements();

    DenseDoubleVectorPtr signedSupervisions = objective.staticCast<ClassificationLearningObjective>()->getSupervisions();
    jassert(signedSupervisions->getNumElements() == n * numLabels);
    double* supervisionsPtr = signedSupervisions.staticCast<DenseDoubleVector>()->getValuePointer(0);

    DenseDoubleVectorPtr res = new DenseDoubleVector(n * numLabels, 0.0);
    double* weightsPtr = res->getValuePointer(0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    loss = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      const DenseDoubleVectorPtr& activations = predictions->getAndCast<DenseDoubleVector>(i);
      double* activationsPtr = activations->getValuePointer(0);
      for (size_t j = 0; j < numLabels; ++j)
      {
        double supervision = *supervisionsPtr++;
        double w0 = supervision > 0 ? positiveWeight : negativeWeight;
        double weight = w0 * exp(-supervision * (*activationsPtr++));
        jassert(isNumberValid(weight));
        *weightsPtr++ = weight;
        loss += weight;
      }
    }
    jassert(isNumberValid(loss));
    res->multiplyByScalar(1.0 / loss);
    return res;
  }
};

class DiscreteAdaBoostMHLearner : public AdaBoostMHLearner
{
public:
  DiscreteAdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : AdaBoostMHLearner(weakLearner, maxIterations, treeDepth) {}
  DiscreteAdaBoostMHLearner() {}

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const
  {
    ClassPtr doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();

    ClassificationLearningObjectivePtr objective = this->objective.staticCast<ClassificationLearningObjective>();
    objective->setPredictions(weakPredictions);
    objective->ensureIsUpToDate();
    
    const double* muNegNegPtr = objective->getMu(0, false)->getValuePointer(0);
    const double* muPosNegPtr = objective->getMu(1, false)->getValuePointer(0);
    const double* muNegPosPtr = objective->getMu(0, true)->getValuePointer(0);
    const double* muPosPosPtr = objective->getMu(1, true)->getValuePointer(0);
    DenseDoubleVectorPtr votes = new DenseDoubleVector(doubleVectorClass);

    double correctWeight = 0.0;
    double errorWeight = 0.0;
    size_t n = votes->getNumValues();
  
    for (size_t j = 0; j < n; ++j)
    {
      double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
      double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
      double vote = (muPositive > muNegative + 1e-9 ? 1.0 : -1.0);
      votes->setValue(j, vote);
      if (vote > 0)
      {
        correctWeight += muPositive;
        errorWeight += muNegative;
      }
      else
      {
        correctWeight += muNegative;
        errorWeight += muPositive;
      }
    }

    DenseDoubleVectorPtr failureVector, successVector;
    if (errorWeight && correctWeight)
    {
      double alpha = 0.5 * log(correctWeight / errorWeight);
      jassert(alpha > -1e-9);
      //context.resultCallback(T("alpha"), alpha);
      failureVector = votes->cloneAndCast<DenseDoubleVector>();
      failureVector->multiplyByScalar(-alpha);
      successVector = votes->cloneAndCast<DenseDoubleVector>();
      successVector->multiplyByScalar(alpha);
    }
    
    failureVote = Variable(failureVector, doubleVectorClass);
    successVote = Variable(successVector, doubleVectorClass);
    missingVote = Variable::missingValue(doubleVectorClass);
    return true;
  }
};

class RealAdaBoostMHLearner : public AdaBoostMHLearner
{
public:
  RealAdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : AdaBoostMHLearner(weakLearner, maxIterations, treeDepth) {}
  RealAdaBoostMHLearner() {}

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const
  {
    ClassPtr doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();

    ClassificationLearningObjectivePtr objective = this->objective.staticCast<ClassificationLearningObjective>();
    objective->setPredictions(weakPredictions);
    
    const double* muNegNegPtr = objective->getMu(0, false)->getValuePointer(0);
    const double* muPosNegPtr = objective->getMu(1, false)->getValuePointer(0);
    const double* muNegPosPtr = objective->getMu(0, true)->getValuePointer(0);
    const double* muPosPosPtr = objective->getMu(1, true)->getValuePointer(0);
    
    DenseDoubleVectorPtr votes = new DenseDoubleVector(doubleVectorClass);
    size_t n = votes->getNumValues();
    for (size_t j = 0; j < n; ++j)
    {
      double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
      double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
      if (muPositive && muNegative)
        votes->setValue(j, 0.5 * log(muPositive / muNegative));
    }

    DenseDoubleVectorPtr failureVotes = votes->cloneAndCast<DenseDoubleVector>();
    failureVotes->multiplyByScalar(-1.0);
    failureVote = Variable(failureVotes, doubleVectorClass);
    successVote = Variable(votes, doubleVectorClass);
    missingVote = Variable::missingValue(doubleVectorClass);
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_

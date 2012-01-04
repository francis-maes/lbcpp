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

class AdaBoostMHWeakObjective : public WeakLearnerObjective
{
public:
  AdaBoostMHWeakObjective(const ClassPtr& doubleVectorClass, const DenseDoubleVectorPtr& supervisions, const DenseDoubleVectorPtr& weights)
    : doubleVectorClass(doubleVectorClass), supervisions(supervisions), weights(weights), votesUpToDate(false)
  {
    labels = DoubleVector::getElementsEnumeration(doubleVectorClass);
    jassert(supervisions->getNumValues() == weights->getNumValues());
    numLabels = labels->getNumElements();
    for (size_t i = 0; i < 3; ++i)
      for (size_t j = 0; j < 2; ++j)
      {
        mu[i][j] = new DenseDoubleVector(doubleVectorClass, numLabels);
        votes[i] = new DenseDoubleVector(doubleVectorClass, numLabels);
      }
    votesUpToDate = false;
  }

  virtual Variable computeVote(const IndexSetPtr& indices) const
  {
    std::vector<ScalarVariableMean> stats(numLabels);
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    {
      size_t example = *it;
      double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
      double* weightsPtr = weights->getValuePointer(numLabels * example);
      for (size_t i = 0; i < numLabels; ++i)
        stats[i].push(*supervisionsPtr++, *weightsPtr++);
    }
    DenseDoubleVectorPtr res = new DenseDoubleVector(labels, probabilityType);
    double sum = 0.0;
    for (size_t i = 0; i < numLabels; ++i)
    {
      double value = stats[i].getMean() > 1e-9 ? 1.0 : 0.0;
      sum += value;
      res->setValue(i, value);
    }
    if (sum > 1.0)
      res->multiplyByScalar(1.0 / sum);
    return Variable(res, probabilityType);
  }

  virtual void setPredictions(const LuapeSampleVectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  virtual void flipPrediction(size_t index)
  {
    double* weightsPtr = weights->getValuePointer(index * numLabels);
    double* muNegNegPtr = mu[0][0]->getValuePointer(0);
    double* muNegPosPtr = mu[0][1]->getValuePointer(0);
    double* muPosNegPtr = mu[1][0]->getValuePointer(0);
    double* muPosPosPtr = mu[1][1]->getValuePointer(0);
    double* supervisionsPtr = supervisions->getValuePointer(index * numLabels);
    for (size_t i = 0; i < numLabels; ++i)
    {
      double weight = *weightsPtr++;
      
      double& muNegNeg = *muNegNegPtr++;
      double& muNegPos = *muNegPosPtr++;
      double& muPosNeg = *muPosNegPtr++;
      double& muPosPos = *muPosPosPtr++;
      double supervision = *supervisionsPtr++;
      
      if (supervision < 0)
        muNegNeg -= weight, muPosNeg += weight;
      else
        muNegPos -= weight, muPosPos += weight;
    }
    votesUpToDate = false;
  }

  virtual double computeObjective() const
  {
    const_cast<AdaBoostMHWeakObjective* >(this)->ensureVotesAreComputed();
    size_t n = labels->getNumElements();
    double edge = 0.0;

    const double* muNegNegPtr = mu[0][0]->getValuePointer(0);
    const double* muPosNegPtr = mu[1][0]->getValuePointer(0);
    const double* muNegPosPtr = mu[0][1]->getValuePointer(0);
    const double* muPosPosPtr = mu[1][1]->getValuePointer(0);
    
    for (size_t j = 0; j < n; ++j)
    {
      double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
      double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
      double vote = (muPositive > muNegative + 1e-9 ? 1.0 : -1.0);
      edge += vote * (muPositive - muNegative);
    }
    return edge;
  }
  
  const DenseDoubleVectorPtr* getVotes() const
  {
    const_cast<AdaBoostMHWeakObjective* >(this)->ensureVotesAreComputed();
    return votes;
  }
  
  const DenseDoubleVectorPtr getMu(unsigned char prediction, bool supervision) const
    {return mu[prediction][supervision ? 1 : 0];}

protected:
  EnumerationPtr labels;
  size_t numLabels;
  ClassPtr doubleVectorClass;
  LuapeSampleVectorPtr predictions;
  DenseDoubleVectorPtr supervisions;  // size = numExamples * numLabels
  DenseDoubleVectorPtr weights;   // size = numExamples * numLabels

  DenseDoubleVectorPtr mu[3][2]; // prediction -> supervision -> label -> weight
  DenseDoubleVectorPtr votes[3]; // prediction -> label -> {-1, 1}
  bool votesUpToDate;

  void computeMuAndVoteValues()
  {
    for (size_t i = 0; i < 3; ++i)
      for (size_t j = 0; j < 2; ++j)
        mu[i][j]->multiplyByScalar(0.0);

    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t example = it.getIndex();
      unsigned char prediction = it.getRawBoolean();
      double* weightsPtr = weights->getValuePointer(numLabels * example);
      double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
      for (size_t j = 0; j < numLabels; ++j)
      {
        double supervision = *supervisionsPtr++;
        mu[prediction][supervision > 0 ? 1 : 0]->incrementValue(j, *weightsPtr++);
      }
    }
    votesUpToDate = false;
  }
  
  void ensureVotesAreComputed()
  {
    if (!votesUpToDate)
    {
      for (size_t i = 0; i < 3; ++i)
      {
        double* votesPtr = votes[i]->getValuePointer(0);
        double* muNegativesPtr = mu[i][0]->getValuePointer(0);
        double* muPositivesPtr = mu[i][1]->getValuePointer(0);
        for (size_t j = 0; j < numLabels; ++j)
          *votesPtr++ = *muPositivesPtr++ > (*muNegativesPtr++ + 1e-09) ? 1.0 : -1.0;
      }
      votesUpToDate = true;
    }
  }  
};

typedef ReferenceCountedObjectPtr<AdaBoostMHWeakObjective> AdaBoostMHWeakObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(WeakLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : WeightBoostingLearner(weakLearner, maxIterations, treeDepth) {}
  AdaBoostMHLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    signedSupervisions = computeSignedSupervisions(problem);
    return WeightBoostingLearner::initialize(context, node, problem, examples);
  }

  virtual WeakLearnerObjectivePtr createWeakObjective(const LuapeInferencePtr& problem) const
    {return new AdaBoostMHWeakObjective(problem.staticCast<LuapeClassifier>()->getDoubleVectorClass(), signedSupervisions, weights);}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& loss) const
  {
    ObjectVectorPtr predictions = problem->getTrainingPredictions().staticCast<ObjectVector>();

    const LuapeClassifierPtr& classifier = problem.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = predictions->getNumElements();
    jassert(signedSupervisions->getNumElements() == n * numLabels);
    DenseDoubleVectorPtr res = new DenseDoubleVector(n * numLabels, 0.0);
    double* weightsPtr = res->getValuePointer(0);
    double* supervisionsPtr = signedSupervisions.staticCast<DenseDoubleVector>()->getValuePointer(0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    const ObjectVectorPtr& predictedActivations = predictions.staticCast<ObjectVector>();
    loss = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      const DenseDoubleVectorPtr& activations = predictedActivations->getAndCast<DenseDoubleVector>(i);
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

protected:
  DenseDoubleVectorPtr signedSupervisions;

  DenseDoubleVectorPtr computeSignedSupervisions(const LuapeInferencePtr& problem) const
  {
    VectorPtr supervisions = problem->getTrainingSupervisions();
    EnumerationPtr labels = LuapeClassifier::getLabelsFromSupervision(supervisions->getElementsType());
    size_t n = supervisions->getNumElements();
    size_t m = labels->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(n * m, 0.0);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = supervisions->getElement(i);
      size_t label;
      if (!lbcpp::convertSupervisionVariableToEnumValue(supervision, label))
      {
        jassert(false);
        return DenseDoubleVectorPtr();
      }
      for (size_t j = 0; j < m; ++j, ++index)
        res->setValue(index, j == label ? 1.0 : -1.0);
    }
    return res;
  }
};

class DiscreteAdaBoostMHLearner : public AdaBoostMHLearner
{
public:
  DiscreteAdaBoostMHLearner(WeakLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : AdaBoostMHLearner(weakLearner, maxIterations, treeDepth) {}
  DiscreteAdaBoostMHLearner() {}

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    ClassPtr doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();

    AdaBoostMHWeakObjectivePtr objective = createWeakObjective(problem).staticCast<AdaBoostMHWeakObjective>();
    objective->setPredictions(weakPredictions);
    
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
  RealAdaBoostMHLearner(WeakLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : AdaBoostMHLearner(weakLearner, maxIterations, treeDepth) {}
  RealAdaBoostMHLearner() {}

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    ClassPtr doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();

    AdaBoostMHWeakObjectivePtr objective = createWeakObjective(problem).staticCast<AdaBoostMHWeakObjective>();
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

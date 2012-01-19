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

class AdaBoostMHLearningObjective : public ClassificationLearningObjective
{
public:
  virtual void initialize(const LuapeInferencePtr& problem)
  {
    ClassificationLearningObjective::initialize(problem);
    for (size_t i = 0; i < 3; ++i)
      for (size_t j = 0; j < 2; ++j)
        mu[i][j] = new DenseDoubleVector(doubleVectorClass, numLabels);
  }

  virtual void setSupervisions(const VectorPtr& sup)
  {
    EnumerationPtr labels = LuapeClassifier::getLabelsFromSupervision(sup->getElementsType());
    size_t n = sup->getNumElements();
    size_t m = labels->getNumElements();
    supervisions = new DenseDoubleVector(n * m, 0.0);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = sup->getElement(i);
      size_t label;
      if (!lbcpp::convertSupervisionVariableToEnumValue(supervision, label))
        jassertfalse;
      for (size_t j = 0; j < m; ++j, ++index)
        supervisions->setValue(index, j == label ? 1.0 : -1.0);
    }
  }

  virtual void update()
  {
    if (!weights)
    {
      jassert(supervisions);
      weights = makeDefaultWeights(supervisions);
    }
    jassert(supervisions->getNumValues() == weights->getNumValues());
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
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(upToDate);
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
  }

  virtual Variable computeVote(const IndexSetPtr& indices)
  {
    std::vector<ScalarVariableMean> stats(numLabels);
    
    DenseDoubleVectorPtr res = new DenseDoubleVector(labels, probabilityType);
    double sum = 0.0;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    {
      size_t example = *it;
      double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
      double* weightsPtr = weights->getValuePointer(numLabels * example);
      for (size_t i = 0; i < numLabels; ++i)
      {
        double value = *weightsPtr++ * (*supervisionsPtr++ + 1.0) / 2.0; // transform signed supervision into probability
        res->incrementValue(i, value);
        sum += value;
      }
    }
    if (sum)
      res->multiplyByScalar(1.0 / sum); // normalize probability distribution
    return Variable(res, denseDoubleVectorClass(labels, probabilityType));
  }
  
  virtual double computeObjective()
  {
    ensureIsUpToDate();
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

  const DenseDoubleVectorPtr getMu(unsigned char prediction, bool supervision) const
    {return mu[prediction][supervision ? 1 : 0];}

  const DenseDoubleVectorPtr& getSupervisions() const
    {return supervisions;}

protected:
  DenseDoubleVectorPtr supervisions;  // size = numExamples * numLabels

  DenseDoubleVectorPtr mu[3][2]; // prediction -> supervision -> label -> weight
  
  DenseDoubleVectorPtr makeDefaultWeights(const DenseDoubleVectorPtr& supervisions) const
  {
    size_t n = supervisions->getNumValues();
    size_t numExamples = n / numLabels;
    double positiveWeight =  1.0 / (2 * numExamples);
    double negativeWeight = 1.0 / (2 * numExamples * (numLabels - 1));
    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, supervisions->getValue(i) > 0.0 ? positiveWeight : negativeWeight);
    return res;
  }
};

typedef ReferenceCountedObjectPtr<AdaBoostMHLearningObjective> AdaBoostMHLearningObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : WeightBoostingLearner(new AdaBoostMHLearningObjective(), weakLearner, maxIterations, treeDepth) {}
  AdaBoostMHLearner() {}

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
    {return new LuapeVectorSumNode(problem.staticCast<LuapeClassifier>()->getLabels(), true);}

//  virtual bool shouldStop(double weakObjectiveValue) const
//    {return weakObjectiveValue == 0.0;}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& logLoss) const
  {
    ObjectVectorPtr predictions = problem->getTrainingPredictions().staticCast<ObjectVector>();

    const LuapeClassifierPtr& classifier = problem.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = predictions->getNumElements();

    DenseDoubleVectorPtr signedSupervisions = objective.staticCast<AdaBoostMHLearningObjective>()->getSupervisions();
    jassert(signedSupervisions->getNumElements() == n * numLabels);
    double* supervisionsPtr = signedSupervisions.staticCast<DenseDoubleVector>()->getValuePointer(0);

    DenseDoubleVectorPtr res = new DenseDoubleVector(n * numLabels, 0.0);
    double* weightsPtr = res->getValuePointer(0);

    double positiveWeight =  1.0 / (2 * n);
    double negativeWeight = 1.0 / (2 * n * (numLabels - 1));
    double sumOfWeights = 0.0;
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
        sumOfWeights += weight;
      }
    }
    jassert(isNumberValid(sumOfWeights));
    res->multiplyByScalar(1.0 / sumOfWeights);
    logLoss = log10(sumOfWeights);
    return res;
  }

  virtual void updateSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& contribution, const DenseDoubleVectorPtr& weights, double& logLoss) const
  {
    LuapeSampleVectorPtr predictions = problem->getTrainingCache()->getSamples(context, contribution);

    const LuapeClassifierPtr& classifier = problem.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();
    size_t n = predictions->size();

    DenseDoubleVectorPtr signedSupervisions = objective.staticCast<AdaBoostMHLearningObjective>()->getSupervisions();
    jassert(signedSupervisions->getNumElements() == n * numLabels);
    double sumOfWeights = 0.0;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      const DenseDoubleVectorPtr& activations = it.getRawObject().staticCast<DenseDoubleVector>();
      if (activations)
      {
        size_t index = it.getIndex() * numLabels;
        double* activationsPtr = activations->getValuePointer(0);
        double* supervisionsPtr = signedSupervisions->getValuePointer(index);
        double* weightsPtr = weights->getValuePointer(index);

        for (size_t j = 0; j < numLabels; ++j)
        {
          double supervision = *supervisionsPtr++;
          double weight = *weightsPtr * exp(-supervision * (*activationsPtr++));
          sumOfWeights += weight;
          *weightsPtr++ = weight;
        }
      }
    }
    if (sumOfWeights != 0.0)
    {
      logLoss += log10(sumOfWeights);
      weights->multiplyByScalar(1.0 / sumOfWeights);
    }
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

    AdaBoostMHLearningObjectivePtr objective = this->objective.staticCast<AdaBoostMHLearningObjective>();
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

    AdaBoostMHLearningObjectivePtr objective = this->objective.staticCast<AdaBoostMHLearningObjective>();
    objective->setPredictions(weakPredictions);
    objective->ensureIsUpToDate();
    
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

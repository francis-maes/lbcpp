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

class AdaBoostMHWeakObjective : public BoostingWeakObjective
{
public:
  AdaBoostMHWeakObjective(const LuapeClassifierPtr& classifier, const BooleanVectorPtr& supervisions, const DenseDoubleVectorPtr& weights)
    : labels(classifier->getLabels()), doubleVectorClass(classifier->getDoubleVectorClass()), supervisions(supervisions), weights(weights)
  {
  }

  virtual void setPredictions(const BooleanVectorPtr& predictions)
  {
    this->predictions = predictions;
    computeMuAndVoteValues();
  }

  virtual void flipPrediction(size_t index)
  {
    bool newPrediction = predictions->flip(index);
    size_t numLabels = labels->getNumElements();
    double* weightsPtr = weights->getValuePointer(index * numLabels);
    double* muNegativesPtr = muNegatives->getValuePointer(0);
    double* muPositivesPtr = muPositives->getValuePointer(0);
    double* votesPtr = votes->getValuePointer(0);
    for (size_t i = 0; i < numLabels; ++i)
    {
      double weight = *weightsPtr++;
      double& muNegative = *muNegativesPtr++;
      double& muPositive = *muPositivesPtr++;
      if (newPrediction == supervisions->get(index * numLabels + i))
        {muNegative -= weight; muPositive += weight;}
      else
        {muPositive -= weight; muNegative += weight;}
      *votesPtr++ = muPositive > muNegative ? 1.0 : -1.0;
    }
  }

  virtual double computeObjective() const
  {
    size_t n = labels->getNumElements();
    double edge = 0.0;
    for (size_t i = 0; i < n; ++i)
      edge += votes->getValue(i) * (muPositives->getValue(i) - muNegatives->getValue(i));
    return edge;
  }

  const DenseDoubleVectorPtr& getVotes() const
    {return votes;}

  const DenseDoubleVectorPtr& getMuNegatives() const
    {return muNegatives;}

  const DenseDoubleVectorPtr& getMuPositives() const
    {return muPositives;}

protected:
  EnumerationPtr labels;
  ClassPtr doubleVectorClass;
  BooleanVectorPtr predictions;   // size = numExamples
  BooleanVectorPtr supervisions;  // size = numExamples * numLabels
  DenseDoubleVectorPtr weights;   // size = numExamples * numLabels
  
  DenseDoubleVectorPtr muNegatives; // size = numLabels
  DenseDoubleVectorPtr muPositives; // size = numLabels
  DenseDoubleVectorPtr votes;       // size = numLabels

  void computeMuAndVoteValues()
  {
    muNegatives = new DenseDoubleVector(doubleVectorClass);
    muPositives = new DenseDoubleVector(doubleVectorClass);
    votes = new DenseDoubleVector(doubleVectorClass);

    size_t numLabels = labels->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(supervisions->getNumElements() == numExamples * numLabels);
    std::vector<bool>::const_iterator itpred = predictions->getElements().begin();

    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = *itpred++;
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == supervisions->get(i * numLabels + j));
        (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
      }
    }

    // compute v_l values
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > muNegatives->getValue(i) ? 1.0 : -1.0);
  }
};

typedef ReferenceCountedObjectPtr<AdaBoostMHWeakObjective> AdaBoostMHWeakObjectivePtr;

class AdaBoostMHLearner : public WeightBoostingLearner
{
public:
  AdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner)
    : WeightBoostingLearner(weakLearner) {}
  AdaBoostMHLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective() const
    {return new AdaBoostMHWeakObjective(function.staticCast<LuapeClassifier>(), supervisions, weights);}

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

  virtual Variable computeVote(BoostingWeakObjectivePtr edgeCalculator) const
  {
    const AdaBoostMHWeakObjectivePtr& objective = edgeCalculator.staticCast<AdaBoostMHWeakObjective>();

    const DenseDoubleVectorPtr& votes = objective->getVotes();
    const DenseDoubleVectorPtr& muNegatives = objective->getMuNegatives();
    const DenseDoubleVectorPtr& muPositives = objective->getMuPositives();

    double correctWeight = 0.0;
    double errorWeight = 0.0;
    size_t n = votes->getNumValues();
    const double* votesPtr = votes->getValuePointer(0);
    const double* muNegativesPtr = muNegatives->getValuePointer(0);
    const double* muPositivesPtr = muPositives->getValuePointer(0);
    for (size_t i = 0; i < n; ++i)
    {
      if (*votesPtr++ > 0)
      {
        correctWeight += *muPositivesPtr++;
        errorWeight += *muNegativesPtr++;
      }
      else
      {
        correctWeight += *muNegativesPtr++;
        errorWeight += *muPositivesPtr++;
      }
    }
    double alpha = 0.5 * log(correctWeight / errorWeight);
    jassert(fabs(correctWeight + errorWeight - 1.0) < 1e-9 && alpha > 0.0);

    DenseDoubleVectorPtr res = votes->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(alpha);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADA_BOOST_MH_H_

/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostMHLuapeLearner.h       | AdaBoost.MH learner             |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_MH_H_
# define LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_MH_H_

# include "LuapeBatchLearner.h"

namespace lbcpp
{

class AdaBoostMHLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostMHLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
    : BoostingLuapeLearner(problem, weakLearner, maxIterations) {}
  AdaBoostMHLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeClassifierClass;}

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    return new ObjectVector(classifier->getDoubleVectorClass());
  }

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    EnumerationPtr labels = classifier->getLabels();
    size_t K = labels->getNumElements();
    size_t n = examples.size();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n * K, 1.0 / (2 * n * (K - 1))));
    double invZ = 1.0 / (2 * n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t k = (size_t)examples[i]->getSecond().getInteger();
      jassert(k >= 0 && k < K);
      res->setValue(i * K + k, invZ);
    }
    return res;
  }

  const std::vector<juce::int64>& getSupervisions(const ContainerPtr& sup) const
    {return *(const std::vector<juce::int64>* )&sup.staticCast<GenericVector>()->getValues();}

  // the absolute value of this should be maximized
  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights) const
  {
    const std::vector<juce::int64>& supervisions = getSupervisions(sup);

    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    size_t numLabels = classifier->getLabels()->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions.size());

    // compute mu_l-, mu_l+ and v_l values
    DenseDoubleVectorPtr muNegatives, muPositives, votes;
    computeMuAndVoteValues(function, predictions, supervisions, weights, muNegatives, muPositives, votes);

    // compute edge
    double edge = 0.0;
    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = predictions->get(i);
      size_t correct = (size_t)supervisions[i];
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == (j == correct));
        double wij = *weightsPtr++;
        double vl = votes->getValue(j);
        edge += wij * vl * (isPredictionCorrect ? 1.0 : -1.0);
      }
    }
    return edge;
  }

  virtual Variable computeVote(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const
  {
    const std::vector<juce::int64>& supervisions = getSupervisions(sup);

    DenseDoubleVectorPtr muNegatives, muPositives, votes;
    computeMuAndVoteValues(function, predictions, supervisions, weights, muNegatives, muPositives, votes);
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
    votes->multiplyByScalar(alpha);
    return votes;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0;}

  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const
  {
    size_t numLabels = function.staticCast<LuapeClassifier>()->getLabels()->getNumElements();
    size_t example = index / numLabels;
    size_t k = index % numLabels;
    double alpha = vote.getObjectAndCast<DenseDoubleVector>()->getValue(k);
    bool isCorrectClass = (k == (size_t)supervision->getElement(example).getInteger());
    bool isPredictionCorrect = (prediction->getElement(example).getBoolean() == isCorrectClass);
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }

protected:
  void computeMuAndVoteValues(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const std::vector<juce::int64>& supervisions, const DenseDoubleVectorPtr& weights, DenseDoubleVectorPtr& muNegatives, DenseDoubleVectorPtr& muPositives, DenseDoubleVectorPtr& votes) const
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    size_t numLabels = classifier->getLabels()->getNumElements();
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions.size());

    muNegatives = new DenseDoubleVector(classifier->getDoubleVectorClass());
    muPositives = new DenseDoubleVector(classifier->getDoubleVectorClass());
    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = predictions->get(i);
      size_t correct = (size_t)supervisions[i];
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == (j == correct));
        (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
      }
    }

    // compute v_l values
    votes = new DenseDoubleVector(classifier->getDoubleVectorClass());
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > muNegatives->getValue(i) ? 1.0 : -1.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_MH_H_

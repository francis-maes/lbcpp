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

class AdaBoostMHEdgeCalculator : public BoostingEdgeCalculator
{
public:
  virtual void initialize(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights, const BooleanVectorPtr& labelCorrections)
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    labels = classifier->getLabels();
    doubleVectorClass = classifier->getDoubleVectorClass();
    
    this->predictions = predictions;
    this->supervisions = (const std::vector<juce::int64>* )&sup.staticCast<GenericVector>()->getValues();
    this->weights = weights;
    this->labelCorrections = labelCorrections;
    jassert(supervisions);

    ClassPtr doubleVectorClass = classifier->getDoubleVectorClass();
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
      if (newPrediction == getLabel(index, i))
        {muNegative -= weight; muPositive += weight;}
      else
        {muPositive -= weight; muNegative += weight;}
      *votesPtr++ = muPositive > muNegative ? 1.0 : -1.0;
    }
  }

  virtual double computeEdge() const
  {
    size_t n = labels->getNumElements();
    double edge = 0.0;
    for (size_t i = 0; i < n; ++i)
      edge += votes->getValue(i) * (muPositives->getValue(i) - muNegatives->getValue(i));
    return edge;
  }

  virtual Variable computeVote() const
  {
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

protected:
  EnumerationPtr labels;
  ClassPtr doubleVectorClass;
  BooleanVectorPtr predictions;
  const std::vector<juce::int64>* supervisions;
  DenseDoubleVectorPtr weights;
  BooleanVectorPtr labelCorrections;
  
  DenseDoubleVectorPtr muNegatives;
  DenseDoubleVectorPtr muPositives;
  DenseDoubleVectorPtr votes;

  bool getLabel(size_t exampleIndex, size_t classIndex) const
  {
    size_t correct = (size_t)(*supervisions)[exampleIndex];
    bool res = (correct == classIndex);
    if (labelCorrections && labelCorrections->get(exampleIndex * labels->getNumElements() + classIndex))
      res = !res;
    return res;
  }

  void computeMuAndVoteValues()
  {
    muNegatives = new DenseDoubleVector(doubleVectorClass);
    muPositives = new DenseDoubleVector(doubleVectorClass);
    votes = new DenseDoubleVector(doubleVectorClass);

    size_t numLabels = labels->getNumElements();
    size_t numExamples = supervisions->size();
    jassert(predictions->getNumElements()>= numExamples);
    std::vector<bool>::const_iterator itpred = predictions->getElements().begin();

    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      bool prediction = *itpred++;
      for (size_t j = 0; j < numLabels; ++j)
      {
        bool isPredictionCorrect = (prediction == getLabel(i, j));
        (isPredictionCorrect ? muPositives : muNegatives)->incrementValue(j, *weightsPtr++);
      }
    }

    // compute v_l values
    for (size_t i = 0; i < numLabels; ++i)
      votes->setValue(i, muPositives->getValue(i) > muNegatives->getValue(i) ? 1.0 : -1.0);
  }
};

class AdaBoostMHLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostMHLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
    : BoostingLuapeLearner(problem, weakLearner, maxIterations) {}
  AdaBoostMHLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeClassifierClass;}

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const
    {return new AdaBoostMHEdgeCalculator();}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
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

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0;}

  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const
  {
    size_t numLabels = function.staticCast<LuapeClassifier>()->getLabels()->getNumElements();
    size_t example = index / numLabels;
    size_t k = index % numLabels;
    double alpha = vote.getObjectAndCast<DenseDoubleVector>()->getValue(k);
    
    size_t correctLabel = (size_t)(*(const std::vector<juce::int64>* )&supervision.staticCast<GenericVector>()->getValues())[example]; // faster access than getElement()
    bool isCorrectClass = (k == correctLabel);
    bool isPredictionCorrect = (prediction->get(example) == isCorrectClass);
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }

  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const
  {
    ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
    const std::vector<juce::int64>& sup = *(const std::vector<juce::int64>* )&supervisions.staticCast<GenericVector>()->getValues();
 
    size_t numErrors = 0;
    size_t n = supervisions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      if (pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue() != sup[i])
        ++numErrors;
    }
    return numErrors / (double)n;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_MH_H_

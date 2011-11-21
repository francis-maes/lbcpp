/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostMHLuapeLearner.h       | AdaBoost.MH learner             |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_ADA_BOOST_MH_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_ADA_BOOST_MH_H_

# include "LuapeBoostingLearner.h"

namespace lbcpp
{

class AdaBoostMHEdgeCalculator : public BoostingEdgeCalculator
{
public:
  virtual void initialize(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights)
  {
    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    labels = classifier->getLabels();
    doubleVectorClass = classifier->getDoubleVectorClass();
    
    this->predictions = predictions;
    this->supervisions = sup.staticCast<BooleanVector>();
    this->weights = weights;

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
      if (newPrediction == supervisions->get(index * numLabels + i))
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

class LuapeAdaBoostMHLearner : public LuapeBoostingLearner
{
public:
  LuapeAdaBoostMHLearner(LuapeWeakLearnerPtr weakLearner)
    : LuapeBoostingLearner(weakLearner) {}
  LuapeAdaBoostMHLearner() {}

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const
    {return new AdaBoostMHEdgeCalculator();}

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

  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const
  {
    ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
    BooleanVectorPtr sup = supervisions.staticCast<BooleanVector>();
 
    size_t numErrors = 0;
    size_t n = pred->getNumElements();
    size_t m = sup->getNumElements() / n;
    for (size_t i = 0; i < n; ++i)
    {
      size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
      if (!sup->get(i * m + j))
        ++numErrors;
    }
    return numErrors / (double)n;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_ADA_BOOST_MH_H_

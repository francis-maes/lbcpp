/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostLuapeLearner.h         | AdaBoost learner                |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_H_
# define LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_H_

# include "WeightBoostingLearner.h"

namespace lbcpp
{
  
class AdaBoostEdgeCalculator : public BoostingEdgeCalculator
{
public:
  virtual void initialize(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights)
  {
    this->predictions = predictions;
    this->supervisions = sup.dynamicCast<BooleanVector>();
    this->weights = weights;
    jassert(supervisions);

    size_t n = weights->getNumElements();
    jassert(supervisions->getNumElements() == n);
    jassert(predictions->getNumElements() >= n);

    accuracy = 0.0;
    double* weightsPtr = weights->getValuePointer(0);
    for (size_t i = 0; i < n; ++i, ++weightsPtr)
      if (predictions->get(i) == supervisions->get(i))
        accuracy += *weightsPtr;
  }

  virtual void flipPrediction(size_t index)
  {
    if (predictions->flip(index) == supervisions->get(index))
      accuracy += weights->getValue(index);
    else
      accuracy -= weights->getValue(index);
  }

  virtual double computeEdge() const
    {return juce::jmax(accuracy, 1.0 - accuracy);}

  virtual Variable computeVote() const
  {
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log(accuracy / (1.0 - accuracy));
  }

protected:
  BooleanVectorPtr predictions;
  BooleanVectorPtr supervisions;
  DenseDoubleVectorPtr weights;
  double accuracy;
};

// FIXME: convert this
#if 0 
class AdaBoostLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostLuapeLearner(LuapeProblemPtr problem, BoostingWeakLearnerPtr weakLearner, size_t maxIterations)
    : BoostingLuapeLearner(problem, weakLearner, maxIterations) {}
  AdaBoostLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeBinaryClassifierClass;}

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const
    {return new AdaBoostEdgeCalculator();}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual bool shouldStop(double accuracy) const
    {return accuracy == 0.0 || accuracy == 1.0;}

  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const Variable& vote) const
  {
    double alpha = vote.toDouble();
    bool isPredictionCorrect = (supervisions->getElement(index).getBoolean() == predictions->get(index));
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }

  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const
  {
    BooleanVectorPtr pred = predictions.staticCast<BooleanVector>();
    BooleanVectorPtr sup = supervisions.staticCast<BooleanVector>();
    size_t numErrors = 0;
    size_t n = supervisions->getNumElements();
    for (size_t i = 0; i < n; ++i)
      if (pred->get(i) != sup->get(i))
        ++numErrors;
    return numErrors / (double)n;
  }
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

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

// FIXME: implement with real-valued weak predictions
class AdaBoostWeakObjective : public BoostingWeakObjective
{
public:
  AdaBoostWeakObjective(const LuapeInferencePtr& classifier, const BooleanVectorPtr& supervisions, const DenseDoubleVectorPtr& weights)
    : supervisions(supervisions), weights(weights)
  {
  }

  virtual void setPredictions(const VectorPtr& predictions)
  {
    this->predictions = predictions;

    size_t n = weights->getNumElements();
    jassert(supervisions->getNumElements() == n);
    jassert(predictions->getNumElements() >= n);

    accuracy = 0.0;

    BooleanVectorPtr booleanPredictions = predictions.dynamicCast<BooleanVector>();
    if (booleanPredictions)
    {
      double* weightsPtr = weights->getValuePointer(0);
      for (size_t i = 0; i < n; ++i, ++weightsPtr)
        if (booleanPredictions->get(i) == supervisions->get(i))
          accuracy += *weightsPtr;
    }
    else
      jassert(false);
  }

  virtual void flipPrediction(size_t index)
  {
    if (predictions.staticCast<BooleanVector>()->flip(index) == supervisions->get(index))
      accuracy += weights->getValue(index);
    else
      accuracy -= weights->getValue(index);
  }

  virtual double computeObjective() const
    {return juce::jmax(accuracy, 1.0 - accuracy);}

  double getAccuracy() const
    {return accuracy;}

protected:
  VectorPtr predictions;
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

  virtual Variable computeVote() const
  {
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log(accuracy / (1.0 - accuracy));
  }

  virtual TypePtr getRequiredFunctionType() const
    {return luapeBinaryClassifierClass;}

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const
    {return new AdaBoostWeakObjective(examples);}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual bool shouldStop(double accuracy) const
    {return accuracy == 0.0 || accuracy == 1.0;}

  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const VectorPtr& predictions, const ContainerPtr& supervisions, const Variable& vote) const
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

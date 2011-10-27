/*-----------------------------------------.---------------------------------.
| Filename: AdaBoostLuapeLearner.h         | AdaBoost learner                |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_H_
# define LBCPP_LUAPE_BATCH_LEARNER_ADA_BOOST_H_

# include "LuapeBatchLearner.h"

namespace lbcpp
{
  
class AdaBoostEdgeCalculator : public BoostingEdgeCalculator
{
public:
  virtual void initialize(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& sup, const DenseDoubleVectorPtr& weights)
  {
    this->predictions = predictions;
    this->supervisions = sup.dynamicCast<BooleanVector>();
    this->weights = weights;
    jassert(supervisions);

    size_t n = weights->getNumElements();
    jassert(n == supervisions->getNumElements());
    jassert(n == predictions->getNumElements());

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

class AdaBoostLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
    : BoostingLuapeLearner(problem, weakLearner, maxIterations) {}
  AdaBoostLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeBinaryClassifierClass;}

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const
    {return new AdaBoostEdgeCalculator();}

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const
    {return new DenseDoubleVector(0, 0.0);}

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual bool shouldStop(double accuracy) const
    {return accuracy == 0.0 || accuracy == 1.0;}

  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const Variable& vote) const
  {
    double alpha = vote.toDouble();
    bool isPredictionCorrect = (supervisions->getElement(index).getBoolean() == predictions->get(index));
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

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
  
class AdaBoostLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations)
    : BoostingLuapeLearner(problem, weakLearner, maxIterations) {}
  AdaBoostLuapeLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeBinaryClassifierClass;}

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const
    {return new DenseDoubleVector(0, 0.0);}

  virtual Variable computeVote(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const
  {
    double accuracy = weakObjectiveValue + 0.5;
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log(accuracy / (1.0 - accuracy));
  }

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const
    {size_t n = examples.size(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions->getNumElements());
    jassert(numExamples == weights->getNumElements());

    double accuracy = 0.0;
    for (size_t i = 0; i < numExamples; ++i)
      if (predictions->get(i) == supervisions->getElement(i).getBoolean())
        accuracy += weights->getValue(i);
    return accuracy - 0.5;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0 || weakObjectiveValue == 0.5;}


  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const Variable& vote) const
  {
    double alpha = vote.toDouble();
    bool isPredictionCorrect = (supervisions->getElement(index).getBoolean() == predictions->get(index));
    return currentWeight * exp(-alpha * (isPredictionCorrect ? 1.0 : -1.0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

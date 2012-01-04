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
# include <lbcpp/Learning/Numerical.h> // for convertSupervisionVariableToBoolean
# include <lbcpp/Luape/WeakLearner.h>

namespace lbcpp
{

class AdaBoostWeakObjective : public WeakLearnerObjective
{
public:
  AdaBoostWeakObjective(const DenseDoubleVectorPtr& supervisions, const DenseDoubleVectorPtr& weights)
    : supervisions(supervisions), weights(weights), correctWeight(0.0), errorWeight(0.0), missingWeight(0.0)
  {
    jassert(supervisions->getElementsType() == probabilityType);
    jassert(supervisions->getNumValues() == weights->getNumValues());
  }

  virtual Variable computeVote(const IndexSetPtr& indices) const
  {
    ScalarVariableMean res;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
      res.push(supervisions->getValue(*it), weights->getValue(*it));
    return Variable(res.getMean(), probabilityType);
  }

  virtual void setPredictions(const LuapeSampleVectorPtr& predictions)
  {
    this->predictions = predictions;

    correctWeight = 0.0;
    errorWeight = 0.0;
    missingWeight = 0.0;

    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t example = it.getIndex();
      bool sup = (supervisions->getValue(example) > 0.5);
      double weight = weights->getValue(example);
      unsigned char pred = it.getRawBoolean();
      if (pred == 2)
        missingWeight += weight;
      else if ((pred == 0 && !sup) || (pred == 1 && sup))
        correctWeight += weight;
      else
        errorWeight += weight;
    }
  }

  virtual void flipPrediction(size_t index)
  {
    bool sup = supervisions->getValue(index) > 0.5;
    double weight = weights->getValue(index);
    if (sup)
    {
      correctWeight += weight;
      errorWeight -= weight;
    }
    else
    {
      errorWeight += weight;
      correctWeight -= weight;
    }
  }

  virtual double computeObjective() const
  {
    double totalWeight = (missingWeight + correctWeight + errorWeight);
    jassert(totalWeight);
    return juce::jmax(correctWeight / totalWeight, errorWeight / totalWeight);
  }

  double getCorrectWeight() const
    {return correctWeight;}

  double getErrorWeight() const
    {return errorWeight;}

  double getMissingWeight() const
    {return missingWeight;}

protected:
  friend class AdaBoostWeakObjectiveClass;

  LuapeSampleVectorPtr predictions;
  DenseDoubleVectorPtr supervisions;
  DenseDoubleVectorPtr weights;

  double correctWeight;
  double errorWeight;
  double missingWeight;
};

typedef ReferenceCountedObjectPtr<AdaBoostWeakObjective> AdaBoostWeakObjectivePtr;

class AdaBoostLearner : public WeightBoostingLearner
{
public:
  AdaBoostLearner(WeakLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : WeightBoostingLearner(weakLearner, maxIterations, treeDepth) {}
  AdaBoostLearner() {}

  virtual WeakLearnerObjectivePtr createWeakObjective(const LuapeInferencePtr& problem) const
    {return new AdaBoostWeakObjective(problem->getTrainingSupervisions(), weights);}

//  virtual bool shouldStop(double accuracy) const
//    {return accuracy == 0.0 || accuracy == 1.0;}
 
  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& successVote, Variable& failureVote, Variable& missingVote) const
  {
    AdaBoostWeakObjectivePtr objective = createWeakObjective(problem).staticCast<AdaBoostWeakObjective>();
    objective->setPredictions(weakPredictions);

    double correctWeight = objective->getCorrectWeight();
    double errorWeight = objective->getErrorWeight();
    //double missingWeight = objective->getMissingWeight();

    double vote;
    if (correctWeight == 0.0)
      vote = -1.0;
    else if (errorWeight == 0.0)
      vote = 1.0;
    else
      vote = 0.5 * log(correctWeight / errorWeight);

    successVote = vote;
    failureVote = -vote;
    missingVote = 0.0;
    return true;
  }

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& loss) const
  {
    DenseDoubleVectorPtr predictions = problem->getTrainingPredictions().staticCast<DenseDoubleVector>();
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions().staticCast<DenseDoubleVector>();
    jassert(predictions->getNumValues() == supervisions->getNumValues());
    jassert(supervisions->getElementsType() == probabilityType);

    size_t n = predictions->getNumValues();
    double invZ = 1.0 / (double)n;
    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);

    double* weightsPtr = res->getValuePointer(0);
    double* supervisionsPtr = supervisions->getValuePointer(0);
    double* predictionsPtr = predictions->getValuePointer(0);

    loss = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double supervision = *supervisionsPtr++;
      double prediction = *predictionsPtr++;
      double weight = invZ * exp(-(2 * supervision - 1.0) * prediction); // supervision are probabilities, scale to range [-1,1]
      *weightsPtr++ = weight;
      loss += weight;
    }
    jassert(isNumberValid(loss));
    res->multiplyByScalar(1.0 / loss);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

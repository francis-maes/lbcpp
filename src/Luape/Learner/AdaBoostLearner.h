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

namespace lbcpp
{

class AdaBoostLearner : public WeightBoostingLearner
{
public:
  AdaBoostLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : WeightBoostingLearner(new BinaryClassificationLearningObjective(), weakLearner, maxIterations, treeDepth) {}
  AdaBoostLearner() {}

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
    {return new LuapeScalarSumNode();}

//  virtual bool shouldStop(double accuracy) const
//    {return accuracy == 0.0 || accuracy == 1.0;}
 
  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const
  {
    BinaryClassificationLearningObjectivePtr objective = this->objective.staticCast<BinaryClassificationLearningObjective>();
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

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& logLoss) const
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

    double sumOfWeights = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double supervision = *supervisionsPtr++;
      double prediction = *predictionsPtr++;
      double weight = invZ * exp(-(2 * supervision - 1.0) * prediction); // supervision are probabilities, scale to range [-1,1]
      *weightsPtr++ = weight;
      sumOfWeights += weight;
    }
    jassert(isNumberValid(sumOfWeights));
    res->multiplyByScalar(1.0 / sumOfWeights);
    logLoss = log10(sumOfWeights);
    return res;
  }

  virtual void updateSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& contribution, const DenseDoubleVectorPtr& weights, double& logLoss) const
  {
    // FIXME
    jassert(false);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

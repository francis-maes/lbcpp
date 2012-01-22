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

  virtual Variable computeVote(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions) const
  {
    BinaryClassificationLearningObjectivePtr objective = this->objective.staticCast<BinaryClassificationLearningObjective>();
    objective->setPredictions(weakPredictions);

    double correctWeight = objective->getCorrectWeight();
    double errorWeight = objective->getErrorWeight();
    //double missingWeight = objective->getMissingWeight();

    double epsilon = 1.0 / (double)problem->getTrainingCache()->getNumSamples();
    return 0.5 * log((correctWeight + epsilon) / (errorWeight + epsilon));
  }

  virtual Variable negateVote(const Variable& vote) const
    {return -vote.getDouble();}

  virtual LuapeFunctionPtr makeVoteFunction(ExecutionContext& context, const LuapeInferencePtr& problem, const Variable& vote) const
    {return scalarVoteLuapeFunction(vote.getDouble());}
  
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

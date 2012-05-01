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
    {return new LuapeScalarSumNode(false, false);}

//  virtual bool shouldStop(double accuracy) const
//    {return accuracy == 0.0 || accuracy == 1.0;}

  virtual Variable computeVote(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions) const
  {
    BinaryClassificationLearningObjectivePtr objective = this->objective.staticCast<BinaryClassificationLearningObjective>();
    objective->setPredictions(weakPredictions);
    objective->ensureIsUpToDate();
    
    double correctWeight = objective->getCorrectWeight();
    double errorWeight = objective->getErrorWeight();
    double missingWeight = objective->getMissingWeight();

    double epsilon = 1.0 / (double)problem->getTrainingCache()->getNumSamples();
    //std::cout << "Correct: " << correctWeight << " Error: " << errorWeight << " Missing: " << missingWeight << " => vote = " << 0.5 * log((correctWeight + epsilon) / (errorWeight + epsilon)) << std::endl;
    
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
    LuapeSampleVectorPtr predictions = problem->getTrainingCache()->getSamples(context, contribution);
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions().staticCast<DenseDoubleVector>();
    size_t n = predictions->size();
    
    jassert(supervisions->getElementsType() == probabilityType);
    jassert(n == supervisions->getNumValues());
    jassert(n == weights->getNumValues());
    
    double sumOfWeights = 0.0;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t index = it.getIndex();
      double supervision = supervisions->getValue(index) * 2 - 1.0; // scale from probabilities to {-1,1}
      double weight = weights->getValue(index) * exp(-supervision * it.getRawDouble());
      //std::cout << index << ": " << supervision << " " << it.getRawDouble() << " " << weight << "; " << std::flush;
      sumOfWeights += weight;
      weights->setValue(index, weight);
    }
    if (sumOfWeights != 0.0)
    {
      logLoss += log10(sumOfWeights);
      weights->multiplyByScalar(1.0 / sumOfWeights);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_

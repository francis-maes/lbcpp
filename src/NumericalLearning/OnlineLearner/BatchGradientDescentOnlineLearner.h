/*-----------------------------------------.---------------------------------.
| Filename: BatchGradientDescentLearnin...h| Batch (or large mini-batch)     |
| Author  : Francis Maes                   |  gradient descent               |
| Started : 26/05/2010 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_BATCH_GRADIENT_H_
# define LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_BATCH_GRADIENT_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class BatchOldGradientDescentOnlineLearner : public OldGradientDescentOnlineLearner
{
public:
  BatchOldGradientDescentOnlineLearner(LearnerUpdateFrequency learningUpdateFrequency,
                                    IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                    LearnerUpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
    : OldGradientDescentOnlineLearner(learningUpdateFrequency, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer)
    {jassert(learningUpdateFrequency != never && learningUpdateFrequency != perStep);}

  BatchOldGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    updateParameters(context, inference, 1.0, input, supervision, prediction, &gradientSum);
    ++epoch;
    if (learningUpdateFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = learningUpdateFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (epoch % miniBatchSize == 0))
        applyGradientSum(context, inference);
    }
    checkRegularizerAfterStep(context, inference);
    OldGradientDescentOnlineLearner::stepFinishedCallback(context, inference, input, supervision, prediction);
  }

  virtual void episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference)
  {
    if (learningUpdateFrequency == perEpisode)
      applyGradientSum(context, inference);
    OldGradientDescentOnlineLearner::episodeFinishedCallback(context, inference);
  }

  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
    {applyGradientSum(context, inference); OldGradientDescentOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);}

protected:
  ObjectPtr gradientSum;

  void applyGradientSum(ExecutionContext& context, InferencePtr inference)
  {
    if (gradientSum)
    {
      //std::cout << "E" << std::flush;
      gradientDescentStep(context, inference, gradientSum);
      gradientSum = ObjectPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_BATCH_GRADIENT_H_

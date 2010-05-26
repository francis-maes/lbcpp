/*-----------------------------------------.---------------------------------.
| Filename: PerStepGradientDescentLearn...h| Stochastic gradient descent     |
| Author  : Francis Maes                   |                                 |
| Started : 25/05/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentLearningCallback.h"

namespace lbcpp
{

class StochasticGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  StochasticGradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, perStep, learningRate, normalizeLearningRate,
                                      never, regularizerUpdateFrequency, regularizer) {}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
    {applyExample(input, supervision, predictedOutput);}
};

class BatchGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  BatchGradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, learningUpdateFrequency, learningRate, normalizeLearningRate,
                                      never, regularizerUpdateFrequency, regularizer)
    {jassert(learningUpdateFrequency != never && learningUpdateFrequency != perStep);}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    FeatureGeneratorPtr exampleGradient = getExampleGradient(input, supervision, predictedOutput);
    if (!gradientSum)
      gradientSum = new DenseVector(exampleGradient->getDictionary());
    exampleGradient->addTo(gradientSum);
    ++epoch;
    if (learningUpdateFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = learningUpdateFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (epoch % miniBatchSize == 0))
        applyGradientSum();
    }
    checkRegularizerAfterStep();
  }

  virtual void episodeFinishedCallback()
  {
    if (learningUpdateFrequency == perEpisode)
      applyGradientSum();
    GradientDescentLearningCallback::episodeFinishedCallback();
  }

  virtual void passFinishedCallback()
  {
    if (learningUpdateFrequency == perPass)
      applyGradientSum();
    GradientDescentLearningCallback::passFinishedCallback();
  }

protected:
  DenseVectorPtr gradientSum;

  void applyGradientSum()
  {
    if (gradientSum)
    {
      gradientSum->addWeightedTo(getParameters(), - computeLearningRate());
      parametersChanged();
      gradientSum = DenseVectorPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

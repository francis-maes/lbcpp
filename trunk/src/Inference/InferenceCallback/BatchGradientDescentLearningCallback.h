/*-----------------------------------------.---------------------------------.
| Filename: BatchGradientDescentLearnin...h| Batch (or large mini-batch)     |
| Author  : Francis Maes                   |  gradient descent               |
| Started : 26/05/2010 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentLearningCallback.h"

namespace lbcpp
{

class BatchGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  BatchGradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, learningUpdateFrequency, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer)
    {jassert(learningUpdateFrequency != never && learningUpdateFrequency != perStep);}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    GradientDescentLearningCallback::stepFinishedCallback(input, supervision, predictedOutput);
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
    {applyGradientSum(); GradientDescentLearningCallback::passFinishedCallback();}

protected:
  DenseVectorPtr gradientSum;

  void applyGradientSum()
  {
    if (gradientSum)
    {
      std::cout << "E" << std::flush;
      gradientDescentStep(gradientSum);
      gradientSum = DenseVectorPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

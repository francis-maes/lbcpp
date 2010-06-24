/*-----------------------------------------.---------------------------------.
| Filename: BatchGradientDescentLearnin...h| Batch (or large mini-batch)     |
| Author  : Francis Maes                   |  gradient descent               |
| Started : 26/05/2010 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class BatchGradientDescentOnlineLearner : public GradientDescentOnlineLearner
{
public:
  BatchGradientDescentOnlineLearner(UpdateFrequency learningUpdateFrequency,
                                    IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                    UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentOnlineLearner(learningUpdateFrequency, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer)
    {jassert(learningUpdateFrequency != never && learningUpdateFrequency != perStep);}

  BatchGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    GradientDescentOnlineLearner::stepFinishedCallback(inference, input, supervision, predictedOutput);
    FeatureGeneratorPtr exampleGradient = getExampleGradient(inference, input, supervision, predictedOutput);
    if (!gradientSum)
      gradientSum = new DenseVector(exampleGradient->getDictionary());
    exampleGradient->addTo(gradientSum);
    ++epoch;
    if (learningUpdateFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = learningUpdateFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (epoch % miniBatchSize == 0))
        applyGradientSum(inference);
    }
    checkRegularizerAfterStep(inference);
  }

  virtual void episodeFinishedCallback(InferencePtr inference)
  {
    if (learningUpdateFrequency == perEpisode)
      applyGradientSum(inference);
    GradientDescentOnlineLearner::episodeFinishedCallback(inference);
  }

  virtual void passFinishedCallback(InferencePtr inference)
    {applyGradientSum(inference); GradientDescentOnlineLearner::passFinishedCallback(inference);}

protected:
  DenseVectorPtr gradientSum;

  void applyGradientSum(InferencePtr inference)
  {
    if (gradientSum)
    {
      //std::cout << "E" << std::flush;
      gradientDescentStep(inference, gradientSum);
      gradientSum = DenseVectorPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

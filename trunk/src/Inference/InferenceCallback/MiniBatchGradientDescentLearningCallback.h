/*-----------------------------------------.---------------------------------.
| Filename: MiniBatchGradientDescentLea...h| Mini-batch                      |
| Author  : Francis Maes                   |  gradient descent               |
| Started : 26/05/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentLearningCallback.h"

namespace lbcpp
{

class MiniBatchGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  MiniBatchGradientDescentLearningCallback(ParameterizedInferencePtr inference, size_t miniBatchSize,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, (UpdateFrequency)(perStepMiniBatch + miniBatchSize), learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer), miniBatchSize(miniBatchSize)
    {jassert(miniBatchSize > 1);}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    GradientDescentLearningCallback::stepFinishedCallback(input, supervision, predictedOutput);
    FeatureGeneratorPtr exampleGradient = getExampleGradient(input, supervision, predictedOutput);
    if (!gradientSum)
      gradientSum = new SparseVector(exampleGradient->getDictionary());
    exampleGradient->addTo(gradientSum);
    ++epoch;
    if ((epoch % miniBatchSize) == 0)
      applyGradientSum();
    checkRegularizerAfterStep();
  }

  virtual void passFinishedCallback()
    {applyGradientSum(); GradientDescentLearningCallback::passFinishedCallback();}

protected:
  SparseVectorPtr gradientSum;
  size_t miniBatchSize;

  void applyGradientSum()
  {
    if (gradientSum)
    {
      //std::cout << "E" << std::flush;
      gradientDescentStep(gradientSum);
      gradientSum = SparseVectorPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

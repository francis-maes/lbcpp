/*-----------------------------------------.---------------------------------.
| Filename: MiniBatchGradientDescentLea...h| Mini-batch                      |
| Author  : Francis Maes                   |  gradient descent               |
| Started : 26/05/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentOnlineLearner.h"
# include <lbcpp/FeatureGenerator/SparseVector.h>

namespace lbcpp
{

class MiniBatchGradientDescentOnlineLearner : public GradientDescentOnlineLearner
{
public:
  MiniBatchGradientDescentOnlineLearner(size_t miniBatchSize,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentOnlineLearner((UpdateFrequency)(perStepMiniBatch + miniBatchSize), learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer), miniBatchSize(miniBatchSize)
    {jassert(miniBatchSize > 1);}

  MiniBatchGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    GradientDescentOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
    FeatureGeneratorPtr exampleGradient = getExampleGradient(inference, input, supervision, prediction);
    if (!gradientSum)
      gradientSum = new SparseVector(exampleGradient->getDictionary());
    exampleGradient->addTo(gradientSum);
    ++epoch;
    if ((epoch % miniBatchSize) == 0)
      applyGradientSum(inference);
    checkRegularizerAfterStep(inference);
  }

  virtual void passFinishedCallback(InferencePtr inference)
    {applyGradientSum(inference); GradientDescentOnlineLearner::passFinishedCallback(inference);}

protected:
  SparseVectorPtr gradientSum;
  size_t miniBatchSize;

  void applyGradientSum(InferencePtr inference)
  {
    if (gradientSum)
    {
      //std::cout << "E" << std::flush;
      gradientDescentStep(inference, gradientSum);
      gradientSum = SparseVectorPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_MINI_BATCH_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

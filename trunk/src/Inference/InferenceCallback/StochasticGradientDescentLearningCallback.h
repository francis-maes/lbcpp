/*-----------------------------------------.---------------------------------.
| Filename: StochasticGradientDescentLe...h| Stochastic gradient descent     |
| Author  : Francis Maes                   |                                 |
| Started : 25/05/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

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
                                      regularizerUpdateFrequency, regularizer) {}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    GradientDescentLearningCallback::stepFinishedCallback(input, supervision, predictedOutput);
    applyExample(input, supervision, predictedOutput);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

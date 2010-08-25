/*-----------------------------------------.---------------------------------.
| Filename: StochasticGradientDescentLe...h| Stochastic gradient descent     |
| Author  : Francis Maes                   |                                 |
| Started : 25/05/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class StochasticGradientDescentOnlineLearner : public GradientDescentOnlineLearner
{
public:
  StochasticGradientDescentOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                          UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
    : GradientDescentOnlineLearner(perStep, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer) {}

  StochasticGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    GradientDescentOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
    applyExample(inference, input, supervision, prediction);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

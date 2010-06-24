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
                                          UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentOnlineLearner(perStep, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer) {}

  StochasticGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    GradientDescentOnlineLearner::stepFinishedCallback(inference, input, supervision, predictedOutput);
    applyExample(inference, input, supervision, predictedOutput);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOCHASTIC_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

/*-----------------------------------------.---------------------------------.
| Filename: StochasticGradientDescentLe...h| Stochastic gradient descent     |
| Author  : Francis Maes                   |                                 |
| Started : 25/05/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_STOCHASTIC_H_
# define LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_STOCHASTIC_H_

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

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    applyExample(inference, input, supervision, prediction);
    GradientDescentOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_STOCHASTIC_H_

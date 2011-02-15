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

class StochasticOldGradientDescentOnlineLearner : public OldGradientDescentOnlineLearner
{
public:
  StochasticOldGradientDescentOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                          LearnerUpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
    : OldGradientDescentOnlineLearner(perStep, learningRate, normalizeLearningRate,
                                      regularizerUpdateFrequency, regularizer) {}

  StochasticOldGradientDescentOnlineLearner() {}

  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    ++epoch;
    updateParameters(context, inference, 1.0, input, supervision, prediction);
    OldGradientDescentOnlineLearner::stepFinishedCallback(context, inference, input, supervision, prediction);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_STOCHASTIC_H_

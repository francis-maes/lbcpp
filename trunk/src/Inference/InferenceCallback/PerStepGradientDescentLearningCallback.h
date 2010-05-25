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

class PerStepGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  PerStepGradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, perStep, learningRate, normalizeLearningRate,
                                      never, regularizerUpdateFrequency, regularizer) {}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
    {applyExample(input, supervision, predictedOutput);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

/*-----------------------------------------.---------------------------------.
| Filename: SingleStepSimulationLearnin...h| A callback that performs        |
| Author  : Francis Maes                   |  learning on a single pass      |
| Started : 09/04/2010 19:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_SINGLE_PASS_SIMULATION_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_SINGLE_PASS_SIMULATION_LEARNING_H_

# include "ExamplesCreatorCallback.h"

namespace lbcpp
{

class SingleStepSimulationLearningCallback : public ExamplesCreatorCallback
{
public:
  SingleStepSimulationLearningCallback(InferenceStepPtr inference)
    : inference(inference) {}

  virtual void startInferencesCallback(size_t count)
    {enableExamplesCreation = false;}

  virtual void finishInferencesCallback()
    {trainAndFlushExamples();}

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == inference)
      enableExamplesCreation = true;
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == inference)
    {
      enableExamplesCreation = false;
      returnCode = InferenceStep::canceledReturnCode;
    }
  }

private:
  InferenceStepPtr inference;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_SINGLE_PASS_SIMULATION_LEARNING_H_

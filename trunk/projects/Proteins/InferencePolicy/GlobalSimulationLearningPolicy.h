/*-----------------------------------------.---------------------------------.
| Filename: GlobalSimulationLearningPolicy.h| A callback that performs global|
| Author  : Francis Maes                   |  simulation to learn a policy   |
| Started : 09/04/2010 15:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_

# include "ExamplesCreatorPolicy.h"

namespace lbcpp
{

class GlobalSimulationLearningCallback : public ExamplesCreatorCallback
{
public:
  virtual void startInferencesCallback(size_t count)
    {std::cout << "Creating training examples with " << count << " episodes..." << std::endl;}

  virtual void finishInferencesCallback()
    {trainAndFlushExamples();}
};

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

#endif // !LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_

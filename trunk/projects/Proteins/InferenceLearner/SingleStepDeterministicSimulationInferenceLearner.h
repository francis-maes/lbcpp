/*-----------------------------------------.---------------------------------.
| Filename: SingleStepDeterministicSim....h| A Learner that performs         |
| Author  : Francis Maes                   | deterministic simulation of a   |
| Started : 16/04/2010 18:19               |  single step                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LEARNER_SINGLE_STEP_DETERMINISTIC_SIMULATION_H_
# define LBCPP_INFERENCE_LEARNER_SINGLE_STEP_DETERMINISTIC_SIMULATION_H_

# include "InferenceLearner.h"
# include "../InferenceContext/ExamplesCreatorCallback.h"
# include "../InferenceStep/SequentialInferenceStep.h"
# include "../InferenceStep/DecoratorInferenceStep.h"

namespace lbcpp
{

class SingleStepSimulationLearningCallback : public ExamplesCreatorCallback
{
public:
  SingleStepSimulationLearningCallback(InferenceStepPtr inference, InferenceLearnerCallbackPtr callback)
    : ExamplesCreatorCallback(callback), inference(inference) {}

  virtual void startInferencesCallback(size_t count)
    {enableExamplesCreation = false;}

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == inference)
      enableExamplesCreation = true;
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == inference)
      enableExamplesCreation = false;
  }

private:
  InferenceStepPtr inference;
};

class StepByStepSimulationInferenceLearner : public InferenceLearner
{
public:
  StepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
    : InferenceLearner(callback) {}

  virtual void train(InferenceStepPtr inf, ObjectContainerPtr trainingData)
  {
    SequentialInferenceStepPtr inference = inf.dynamicCast<SequentialInferenceStep>();
    jassert(inference);
    size_t numSteps = inference->getNumSubSteps();
    for (size_t stepNumber = 0; stepNumber < numSteps; ++stepNumber)
    {
      InferenceStepPtr step = inference->getSubStep(stepNumber);
      InferenceStepPtr decoratedInference = stepNumber < numSteps - 1 ? addBreakToInference(inference, step) : inf;

      callback->preLearningPassCallback(step->getName());
      trainPass(decoratedInference, step, trainingData);
      callback->postLearningPassCallback();
    }
  }
  
private:
  InferenceStepPtr addBreakToInference(InferenceStepPtr inference, InferenceStepPtr lastStepBeforeBreak)
    {return new CallbackBasedDecoratorInferenceStep(inference, new CancelAfterStepCallback(lastStepBeforeBreak));}

  void trainPass(InferenceStepPtr inference, InferenceStepPtr step, ObjectContainerPtr trainingData)
  {
    // create classification examples
    ExamplesCreatorCallbackPtr learningCallback = new SingleStepSimulationLearningCallback(step, callback);
    InferenceContextPtr trainingContext = callback->createContext();
    trainingContext->appendCallback(learningCallback);

    // make examples once and learn
    trainingContext->runWithSupervisedExamples(inference, trainingData);
    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      learningCallback->trainStochasticIteration();
      if (!callback->postLearningIterationCallback(inference, i))
        break;
    }
/*
    // make examples each time and learn 
    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      trainingContext->runWithSupervisedExamples(inference, trainingData);
      learningCallback->trainAndFlushExamples();
      if (!callback->postLearningIterationCallback(inference, i))
        break;
    }*/
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LEARNER_SINGLE_STEP_DETERMINISTIC_SIMULATION_H_

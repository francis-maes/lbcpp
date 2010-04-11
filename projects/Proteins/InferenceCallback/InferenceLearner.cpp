/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.cpp           | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "InferenceLearner.h"
#include "SingleStepSimulationLearningCallback.h"
#include "GlobalSimulationLearningCallback.h"
#include "../InferenceStep/SequentialInferenceStep.h"
using namespace lbcpp;

/*
** InferenceLearner
*/
void InferenceLearner::trainWithCallbacks(InferenceStepPtr inference, ObjectContainerPtr trainingData, InferenceCallbackPtr learningCallback, InferenceCallbackPtr validationCallback)
{
  InferenceCallbackPtr evaluation = callback->createEvaluationCallback();

  InferenceContextPtr trainingContext = callback->createContext();
  trainingContext->appendCallback(evaluation);
  trainingContext->appendCallback(learningCallback);

  InferenceContextPtr validationContext = callback->createContext();
  validationContext->appendCallback(evaluation);
  if (validationCallback)
    validationContext->appendCallback(validationCallback);

  for (size_t i = 0; true; ++i)
  {
    callback->preLearningIterationCallback(i);
    trainingContext->runWithSupervisedExamples(inference, trainingData->randomize());
    if (!callback->postLearningIterationCallback(inference, validationContext, evaluation, i))
      break;
  }
}

/*
** GlobalSimulationInferenceLearner
*/
class GlobalSimulationInferenceLearner : public InferenceLearner
{
public:
  GlobalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
    : InferenceLearner(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData)
    {trainWithCallbacks(inference, trainingData, new GlobalSimulationLearningCallback(), InferenceCallbackPtr());}
};

InferenceLearnerPtr lbcpp::globalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
  {return new GlobalSimulationInferenceLearner(callback);}

/*
** StepByStepSimulationInferenceLearner
*/
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
      callback->preLearningPassCallback(step->getName());
      InferenceCallbackPtr customEvaluationCallback;
      if (stepNumber < numSteps - 1)
        customEvaluationCallback = new CancelAfterStepCallback(step);
      trainWithCallbacks(inference, trainingData, new SingleStepSimulationLearningCallback(step), customEvaluationCallback);
      callback->postLearningPassCallback();
    }
  }
};

InferenceLearnerPtr lbcpp::stepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
  {return new StepByStepSimulationInferenceLearner(callback);}

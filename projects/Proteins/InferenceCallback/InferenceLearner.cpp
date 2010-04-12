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
void InferenceLearner::trainWithCallbacks(InferenceStepPtr inference, ObjectContainerPtr trainingData, InferenceCallbackPtr learningCallback)
{
  InferenceCallbackPtr evaluation = callback->createEvaluationCallback();

  InferenceContextPtr trainingContext = callback->createContext();
  trainingContext->appendCallback(evaluation);
  trainingContext->appendCallback(learningCallback);

  InferenceContextPtr validationContext = callback->createContext();
  validationContext->appendCallback(evaluation);

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
    {trainWithCallbacks(inference, trainingData, new GlobalSimulationLearningCallback());}
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
      trainPass(inference, trainingData, step, customEvaluationCallback);
      callback->postLearningPassCallback();
    }
  }
  
private:
  void trainPass(InferenceStepPtr inference, ObjectContainerPtr trainingData, InferenceStepPtr step, InferenceCallbackPtr customEvaluationCallback)
  {
    ExamplesCreatorCallbackPtr learningCallback = new SingleStepSimulationLearningCallback(step);
  
    InferenceCallbackPtr evaluation = callback->createEvaluationCallback();

    InferenceContextPtr trainingContext = callback->createContext();
    trainingContext->appendCallback(evaluation);
    trainingContext->appendCallback(learningCallback);

    InferenceContextPtr validationContext = callback->createContext();
    validationContext->appendCallback(evaluation);
    if (customEvaluationCallback)
      validationContext->appendCallback(customEvaluationCallback);

    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      if (i == 0) // at the first iteration, we perform episodes to create the training examples
        trainingContext->runWithSupervisedExamples(inference, trainingData);
      learningCallback->trainStochasticIteration(); // then, at each iteration we do a pass of "trainStochastic()"
      if (!callback->postLearningIterationCallback(inference, validationContext, evaluation, i))
        break;
    }
  }
};

InferenceLearnerPtr lbcpp::stepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
  {return new StepByStepSimulationInferenceLearner(callback);}

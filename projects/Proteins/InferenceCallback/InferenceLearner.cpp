/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.cpp           | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "InferenceLearner.h"
#include "ExamplesCreatorCallback.h"
#include "../InferenceStep/SequentialInferenceStep.h"
using namespace lbcpp;

/*
** GlobalSimulationInferenceLearner
*/
class GlobalSimulationInferenceLearner : public InferenceLearner
{
public:
  GlobalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
    : InferenceLearner(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData)
  {
    ExamplesCreatorCallbackPtr learningCallback = new ExamplesCreatorCallback(callback);
    InferenceContextPtr trainingContext = callback->createContext();
    trainingContext->appendCallback(learningCallback);

    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      trainingContext->runWithSupervisedExamples(inference, trainingData->randomize());
      learningCallback->trainAndFlushExamples();
      if (!callback->postLearningIterationCallback(inference, i))
        break;
    }
  }
};

InferenceLearnerPtr lbcpp::globalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
  {return new GlobalSimulationInferenceLearner(callback);}

class CallbackBasedInferenceStepDecorator : public InferenceStep
{
public:
  CallbackBasedInferenceStepDecorator(InferenceStepPtr decorated, InferenceCallbackPtr callback)
    : decorated(decorated), callback(callback) {}

  virtual void accept(InferenceVisitorPtr visitor)
    {decorated->accept(visitor);}

protected:
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    context->appendCallback(callback);
    ObjectPtr res = decorated->run(context, input, supervision, returnCode);
    context->removeCallback(callback);
    return res;
  }

private:
  InferenceStepPtr decorated;
  InferenceCallbackPtr callback;
};

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
      InferenceStepPtr decoratedInference = stepNumber < numSteps - 1 ? addBreakToInference(inference, step) : inference;

      callback->preLearningPassCallback(step->getName());
      trainPass(decoratedInference, step, trainingData);
      callback->postLearningPassCallback();
    }
  }
  
private:
  InferenceStepPtr addBreakToInference(InferenceStepPtr inference, InferenceStepPtr lastStepBeforeBreak)
    {return new CallbackBasedInferenceStepDecorator(inference, new CancelAfterStepCallback(lastStepBeforeBreak));}

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

InferenceLearnerPtr lbcpp::stepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
  {return new StepByStepSimulationInferenceLearner(callback);}

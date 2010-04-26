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
# include "../InferenceContext/CacheInferenceCallback.h"
# include "../InferenceContext/ExamplesCreatorCallback.h"
# include "../InferenceContext/CancelAfterStepCallback.h"
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

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
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

class StepByStepDeterministicSimulationLearner : public InferenceLearner
{
public:
  StepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData, const File& modelDirectory)
    : InferenceLearner(callback), modelDirectory(modelDirectory)
  {
    if (useCacheOnTrainingData)
      cache = new InferenceResultCache();
  }

  virtual void train(InferenceStepPtr inf, ObjectContainerPtr trainingData)
  {
    VectorSequentialInferenceStepPtr inference = inf.dynamicCast<VectorSequentialInferenceStep>();
    jassert(inference);
    size_t numSteps = inference->getNumSubSteps();
    
    /*
    ** Check unicity of InferenceStep names
    */
    std::set<String> names;
    for (size_t stepNumber = 0; stepNumber < numSteps; ++stepNumber)
    {
      String name = inference->getSubStep(stepNumber)->getName();
      if (names.find(name) != names.end())
      {
        Object::error(T("StepByStepDeterministicSimulationLearner::train"), T("Duplicated inference step name: ") + name);
        return;
      }
      names.insert(name);
    }

    /*
    ** Train step by step
    */
    for (size_t stepNumber = 0; stepNumber < numSteps; ++stepNumber)
    {
      InferenceStepPtr step = inference->getSubStep(stepNumber);
      
      File stepFile;
      if (modelDirectory != File::nonexistent)
        stepFile = inference->getSubInferenceFile(stepNumber, modelDirectory);
      if (stepFile.exists() && step->loadFromFile(stepFile))
      {
        std::cout << "Loaded inference step " << stepFile.getFileNameWithoutExtension().quoted() << "." << std::endl;
        continue;
      }

      // decorate inference to add "break"
      InferenceStepPtr decoratedInference = addBreakToInference(inference, step);

      // train current inference step
      callback->preLearningStepCallback(step);
      trainPass(decoratedInference, step, trainingData);
      callback->postLearningStepCallback(step);

      if (modelDirectory != File::nonexistent)
      {
        step->saveToFile(stepFile);
        std::cout << "Saved inference step " << stepFile.getFileNameWithoutExtension().quoted() << "." << std::endl;
      }
    }

    if (modelDirectory != File::nonexistent)
    {
      std::cout << "Save inference " << modelDirectory.getFileNameWithoutExtension().quoted() << std::endl;
      inference->saveToFile(modelDirectory);
    }
  }
  
private:
  InferenceResultCachePtr cache;
  File modelDirectory;

  InferenceStepPtr addBreakToInference(InferenceStepPtr inference, InferenceStepPtr lastStepBeforeBreak)
    {return new CallbackBasedDecoratorInferenceStep(inference->getName() + T(" breaked"), inference, new CancelAfterStepCallback(lastStepBeforeBreak));}

  void trainPass(InferenceStepPtr inference, InferenceStepPtr step, ObjectContainerPtr trainingData)
  {
    // create classification examples
    ExamplesCreatorCallbackPtr learningCallback = new SingleStepSimulationLearningCallback(step, callback);
    InferenceContextPtr trainingContext = callback->createContext();
    trainingContext->appendCallback(learningCallback);
    if (cache)
      trainingContext->appendCallback(new AutoSubStepsCacheInferenceCallback(cache, inference));
    trainingContext->runWithSupervisedExamples(inference, trainingData);

    // learn
    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      learningCallback->trainStochasticIteration();
      if (!callback->postLearningIterationCallback(inference, i))
        break;
    }

    learningCallback->restoreBestParameters();
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LEARNER_SINGLE_STEP_DETERMINISTIC_SIMULATION_H_

  // dead code

/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.cpp           | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

# include <lbcpp/Inference/InferenceLearner.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/DecoratorInference.h>

# include "InferenceCallback/CacheInferenceCallback.h"
# include "InferenceCallback/ExamplesCreatorCallback.h"
# include "InferenceCallback/CancelAfterStepCallback.h"

using namespace lbcpp;


class GlobalSimulationInferenceLearner : public InferenceLearner
{
public:
  GlobalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
    : InferenceLearner(callback) {}

  virtual void train(InferencePtr inference, ObjectContainerPtr trainingData)
  {
    ExamplesCreatorCallbackPtr learningCallback = new ExamplesCreatorCallback(callback, false);
    InferenceContextPtr trainingContext = createLearningContext(inference);
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

InferenceLearnerPtr lbcpp::globalSimulationLearner(InferenceLearnerCallbackPtr callback)
  {return new GlobalSimulationInferenceLearner(callback);}

class SingleStepSimulationLearningCallback : public ExamplesCreatorCallback
{
public:
  SingleStepSimulationLearningCallback(InferencePtr inference, InferenceLearnerCallbackPtr callback)
    : ExamplesCreatorCallback(callback, true), inference(inference) {}

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
  InferencePtr inference;
};

class StepByStepDeterministicSimulationLearner : public InferenceLearner
{
public:
  StepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData, const File& modelDirectory, bool doNotSaveModel)
    : InferenceLearner(callback), modelDirectory(modelDirectory), doNotSaveModel(doNotSaveModel)
  {
    if (useCacheOnTrainingData)
      cache = new InferenceResultCache();
  }

  virtual void train(InferencePtr inf, ObjectContainerPtr trainingData)
  {
    VectorSequentialInferencePtr inference = inf.dynamicCast<VectorSequentialInference>();
    jassert(inference);
    size_t numSteps = inference->getNumSubInferences();
    
    /*
    ** Check unicity of Inference names
    */
    std::set<String> names;
    for (size_t stepNumber = 0; stepNumber < numSteps; ++stepNumber)
    {
      String name = inference->getSubInference(stepNumber)->getName();
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
    for (currentStepNumber = 0; currentStepNumber  < numSteps; ++currentStepNumber )
    {
      InferencePtr step = inference->getSubInference(currentStepNumber);
      
      File stepFile;
      if (modelDirectory != File::nonexistent)
        stepFile = inference->getSubInferenceFile(currentStepNumber, modelDirectory);
      if (stepFile.exists() && step->loadFromFile(stepFile))
      {
        std::cout << "Loaded inference step " << stepFile.getFileNameWithoutExtension().quoted() << "." << std::endl;
        continue;
      }

      // decorate inference to add "break"
      InferencePtr decoratedInference = addBreakToInference(inference, step);

      // train current inference step
      callback->preLearningStepCallback(step);
      trainPass(decoratedInference, step, trainingData);
      callback->postLearningStepCallback(step);

      if (modelDirectory != File::nonexistent && !doNotSaveModel)
      {
        step->saveToFile(stepFile);
        std::cout << "Saved inference step " << stepFile.getFileNameWithoutExtension().quoted() << "." << std::endl;
      }
    }

    if (modelDirectory != File::nonexistent && !doNotSaveModel)
    {
      std::cout << "Save inference " << modelDirectory.getFileNameWithoutExtension().quoted() << std::endl;
      inference->saveToFile(modelDirectory);
    }
  }

protected:
  size_t currentStepNumber;

  virtual InferenceContextPtr createLearningContext(InferencePtr inf)
  {
    if (inf.dynamicCast<DecoratorInference>())
      inf = inf.dynamicCast<DecoratorInference>()->getDecoratedInference();
    VectorSequentialInferencePtr inference = inf.dynamicCast<VectorSequentialInference>();
    jassert(inference);
    jassert(currentStepNumber < inference->getNumSubInferences());
    return InferenceLearner::createLearningContext(inference->getSubInference(currentStepNumber));
  }
  
private:
  InferenceResultCachePtr cache;
  File modelDirectory;
  bool doNotSaveModel;

  InferencePtr addBreakToInference(InferencePtr inference, InferencePtr lastStepBeforeBreak)
    {return callbackBasedDecoratorInference(inference->getName() + T(" breaked"), inference, new CancelAfterStepCallback(lastStepBeforeBreak));}

  void trainPass(InferencePtr inference, InferencePtr step, ObjectContainerPtr trainingData)
  {
    // create classification examples
    ExamplesCreatorCallbackPtr learningCallback = new SingleStepSimulationLearningCallback(step, callback);
    InferenceContextPtr trainingContext = createLearningContext(inference);
    trainingContext->appendCallback(learningCallback);
    if (cache)
      trainingContext->appendCallback(cacheInferenceCallback(cache, inference));
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

InferenceLearnerPtr lbcpp::stepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData, const File& modelDirectory, bool doNotSaveModel)
  {return new StepByStepDeterministicSimulationLearner(callback, useCacheOnTrainingData, modelDirectory, doNotSaveModel);}

class CreateLearningCallbacksVisitor : public DefaultInferenceVisitor
{
public:
  CreateLearningCallbacksVisitor(InferenceContextPtr context, InferenceLearnerCallbackPtr callback)
    : context(context), callback(callback) {}

  virtual void visit(ParameterizedInferencePtr inference)
  {
    InferenceCallbackPtr learningCallback = callback->createLearningCallback(inference, stack.getCurrentInference());
    if (learningCallback)
      context->appendCallback(learningCallback);
  }

private:
  InferenceContextPtr context;
  InferenceLearnerCallbackPtr callback;
};

void InferenceLearner::addLearningCallbacksToContext(InferenceContextPtr context, InferencePtr inference)
  {inference->accept(InferenceVisitorPtr(new CreateLearningCallbacksVisitor(context, callback)));}

InferenceContextPtr InferenceLearner::createLearningContext(InferencePtr inference)
{
  InferenceContextPtr context = callback->createContext();
  addLearningCallbacksToContext(context, inference);
  return context;
}


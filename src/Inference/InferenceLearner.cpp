/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.cpp           | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
#include "InferenceLearner/GlobalSimulationInferenceLearner.h"
#include "InferenceLearner/SingleStepDeterministicSimulationInferenceLearner.h"
using namespace lbcpp;

InferenceLearnerPtr lbcpp::globalSimulationLearner(InferenceLearnerCallbackPtr callback)
  {return new GlobalSimulationInferenceLearner(callback);}

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


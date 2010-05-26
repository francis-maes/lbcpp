/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.h             | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_LEARNER_H_
# define LBCPP_INFERENCE_CALLBACK_LEARNER_H_

# include "predeclarations.h"

namespace lbcpp
{

class InferenceLearnerCallback : public Object
{
public:
  virtual InferenceContextPtr createContext() = 0;

  virtual RegressorPtr createRegressor(InferenceStackPtr stack) = 0;
  virtual ClassifierPtr createClassifier(InferenceStackPtr stack, FeatureDictionaryPtr labels) = 0;

  virtual double getProbabilityToCreateAnExample(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision) = 0;
  virtual InferenceCallbackPtr createLearningCallback(ParameterizedInferencePtr step, InferencePtr parentStep)
    {return InferenceCallbackPtr();}

  //{std::cout << "Using default probability" << std::endl; return 1.0;}

  virtual void preLearningIterationCallback(size_t iterationNumber)
    {}

  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
    {return iterationNumber < 100;}

  virtual void preLearningStepCallback(InferencePtr step)
    {}

  virtual void postLearningStepCallback(InferencePtr step)
    {}
};

class InferenceLearner : public Object
{
public:
  InferenceLearner(InferenceLearnerCallbackPtr callback)
    : callback(callback) {}

  virtual void train(InferencePtr inference, ObjectContainerPtr trainingData) = 0;

protected:
  InferenceLearnerCallbackPtr callback;

  void addLearningCallbacksToContext(InferenceContextPtr context, InferencePtr inference);
  virtual InferenceContextPtr createLearningContext(InferencePtr inference);
};

InferenceLearnerPtr globalSimulationLearner(InferenceLearnerCallbackPtr callback);
InferenceLearnerPtr stepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData = true, const File& modelDirectory = File::nonexistent, bool doNotSaveModel = false);

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_LEARNER_H_

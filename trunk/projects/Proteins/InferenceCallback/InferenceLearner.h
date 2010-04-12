/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.h             | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_LEARNER_H_
# define LBCPP_INFERENCE_CALLBACK_LEARNER_H_

# include "InferenceContext.h"

namespace lbcpp
{

class InferenceLearnerCallback : public Object
{
public:
  virtual void preLearningIterationCallback(size_t iterationNumber)
    {}

  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, InferenceContextPtr validationContext, InferenceCallbackPtr onlineEvaluationCallback, size_t iterationNumber)
    {return iterationNumber < 100;}

  virtual void preLearningPassCallback(const String& passName) {}
  virtual void postLearningPassCallback() {}

  virtual InferenceContextPtr createContext() = 0;
  virtual InferenceCallbackPtr createEvaluationCallback() = 0;
};

typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class InferenceLearner : public Object
{
public:
  InferenceLearner(InferenceLearnerCallbackPtr callback)
    : callback(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData) = 0;

protected:
  InferenceLearnerCallbackPtr callback;

  void trainWithCallbacks(InferenceStepPtr inference, ObjectContainerPtr trainingData, InferenceCallbackPtr learningCallback);
};

typedef ReferenceCountedObjectPtr<InferenceLearner> InferenceLearnerPtr;

InferenceLearnerPtr globalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback);
InferenceLearnerPtr stepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback);

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_LEARNER_H_

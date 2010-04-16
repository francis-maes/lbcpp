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

class InferenceLearnerCallback;
typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class InferenceLearner : public Object
{
public:
  InferenceLearner(InferenceLearnerCallbackPtr callback)
    : callback(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData) = 0;

protected:
  InferenceLearnerCallbackPtr callback;
};

typedef ReferenceCountedObjectPtr<InferenceLearner> InferenceLearnerPtr;

InferenceLearnerPtr globalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback);
InferenceLearnerPtr stepByStepSimulationInferenceLearner(InferenceLearnerCallbackPtr callback);

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_LEARNER_H_

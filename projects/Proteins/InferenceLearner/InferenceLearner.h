/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.h             | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_LEARNER_H_
# define LBCPP_INFERENCE_CALLBACK_LEARNER_H_

# include "../InferencePredeclarations.h"

namespace lbcpp
{

class InferenceLearner : public Object
{
public:
  InferenceLearner(InferenceLearnerCallbackPtr callback)
    : callback(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData) = 0;

protected:
  InferenceLearnerCallbackPtr callback;

  void addLearningCallbacksToContext(InferenceContextPtr context, InferenceStepPtr inference);
  virtual InferenceContextPtr createLearningContext(InferenceStepPtr inference);
};

InferenceLearnerPtr globalSimulationLearner(InferenceLearnerCallbackPtr callback);
InferenceLearnerPtr stepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData = true, const File& modelDirectory = File::nonexistent, bool doNotSaveModel = false);

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_LEARNER_H_

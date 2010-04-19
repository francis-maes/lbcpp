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
};

InferenceLearnerPtr globalSimulationLearner(InferenceLearnerCallbackPtr callback);
InferenceLearnerPtr stepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData = true, size_t firstStepToLearn = 0);

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_LEARNER_H_

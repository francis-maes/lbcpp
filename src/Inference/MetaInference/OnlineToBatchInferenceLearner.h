/*-----------------------------------------.---------------------------------.
| Filename: OnlineToBatchInferenceLearner.h| A batch learner that relies on  |
| Author  : Francis Maes                   | entire-inference simulation     |
| Started : 26/05/2010 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_ONLINE_TO_BATCH_LEARNER_H_
# define LBCPP_INFERENCE_META_ONLINE_TO_BATCH_LEARNER_H_

# include <lbcpp/Inference/SequentialInference.h>
# include "../InferenceCallback/OnlineLearningInferenceCallback.h"
# include "RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class OnlineToBatchInferenceLearner : public InferenceLearner<SequentialInference>
{
public:
  struct State : public SequentialInferenceState
  {
    State(InferencePtr targetInference, const Variable& input, const Variable& supervision)
      : SequentialInferenceState(input, supervision)
    {
      std::set<InferencePtr> inferencesThatHaveALearner;
      targetInference->getInferencesThatHaveAnOnlineLearner(inferencesThatHaveALearner);
      jassert(inferencesThatHaveALearner.size());
      callback = new OnlineLearningInferenceCallback(targetInference, inferencesThatHaveALearner);
    }

    OnlineLearningInferenceCallbackPtr callback;
  };

  typedef ReferenceCountedObjectPtr<State> StatePtr;

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    InferencePtr targetInference = getInference(input);
    ContainerPtr trainingData = getTrainingData(input);

    SequentialInferenceStatePtr res = new State(targetInference, input, supervision);
    updateInference(context, res, returnCode);
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr s, ReturnCode& returnCode)
  {
    StatePtr state = s.staticCast<State>();
    if (state->callback->isLearningStopped())
      return false;

    Variable inferenceAndTrainingData = state->getInput();
    InferencePtr targetInference = getInference(inferenceAndTrainingData);
    ContainerPtr trainingData = getTrainingData(inferenceAndTrainingData);

    InferencePtr learningPassInference = callbackBasedDecoratorInference(T("LearningPass"),
        runOnSupervisedExamplesInference(targetInference, false), state->callback);

    state->setSubInference(learningPassInference, trainingData, Variable());
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_ONLINE_TO_BATCH_LEARNER_H_

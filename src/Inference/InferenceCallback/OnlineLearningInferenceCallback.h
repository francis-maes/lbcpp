/*-----------------------------------------.---------------------------------.
| Filename: OnlineLearningInferenceCallback.h| The callback to call online   |
| Author  : Francis Maes                   |  learners                       |
| Started : 26/05/2010 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_ONLINE_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_ONLINE_LEARNING_H_

# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/Inference/InferenceStack.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include "../MetaInference/RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class OnlineLearningInferenceCallback : public InferenceCallback
{
public:
  OnlineLearningInferenceCallback(InferencePtr targetInference, const std::vector<InferencePtr>& learnedInferences)
    : learnedInferences(learnedInferences), targetInference(targetInference), learningStopped(false)
  {
    for (size_t i = 0; i < learnedInferences.size(); ++i)
      callStartLearningCallback(learnedInferences[i]);
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    const InferencePtr& inference = stack->getCurrentInference();
    if (inference->getOnlineLearner() && supervision.exists())
    {
      // call stepFinishedCallback
      callStepFinishedCallback(inference, input, supervision, output);

      // call subStepFinishedCallback
      for (int i = stack->getDepth() - 1; i >= 0; --i)
      {
        const InferencePtr& parent = stack->getInference(i);
        callSubStepFinishedCallback(parent, inference, input, supervision, output);
        if (parent == targetInference)
          break;
      }
    }
    else if (inference == targetInference)
      finishEpisode();
    else if (isRunOnSupervisedExamplesInference(inference))
      finishPass();
  }

  bool isLearningStopped() const
    {return learningStopped;}

private:
  std::vector<InferencePtr> learnedInferences;
  InferencePtr targetInference;
  bool learningStopped;

  void finishEpisode()
  {
    for (int i = (int)learnedInferences.size() - 1; i >= 0; --i)
      callEpisodeFinishedCallback(learnedInferences[i]);
  }

  void finishPass()
  {
    bool wantsMoreIterations = false;
    for (int i = (int)learnedInferences.size() - 1; i >= 0; --i)
    {
      const InferencePtr& inference = learnedInferences[i];
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      if (!learner->isLearningStopped())
      {
        callPassFinishedCallback(inference);
        wantsMoreIterations |= learner->wantsMoreIterations();
      }
    }
    learningStopped = !wantsMoreIterations;
  }

  void callStartLearningCallback(const InferencePtr& inference)
  {
    InferenceOnlineLearnerPtr learner = inference->getOnlineLearner();
    if (learner)
    {
      learner->startLearningCallback();
      jassert(!learner->isLearningStopped());
    }
  }

  void callStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {
    InferenceOnlineLearnerPtr learner = inference->getOnlineLearner();
    if (learner && !learner->isLearningStopped())
      learner->stepFinishedCallback(inference, input, supervision, output);
  }

  void callSubStepFinishedCallback(const InferencePtr& parentInference, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {
    InferenceOnlineLearnerPtr learner = parentInference->getOnlineLearner();
    if (learner && !learner->isLearningStopped())
      learner->subStepFinishedCallback(inference, input, supervision, output);
  }

  void callEpisodeFinishedCallback(const InferencePtr& inference)
  {
    InferenceOnlineLearnerPtr learner = inference->getOnlineLearner();
    if (learner && !learner->isLearningStopped())
      learner->episodeFinishedCallback(inference);
  }

  void callPassFinishedCallback(const InferencePtr& inference)
  {
    InferenceOnlineLearnerPtr learner = inference->getOnlineLearner();
    if (learner && !learner->isLearningStopped())
      learner->passFinishedCallback(inference);
  }
};

typedef ReferenceCountedObjectPtr<OnlineLearningInferenceCallback> OnlineLearningInferenceCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_ONLINE_LEARNING_H_

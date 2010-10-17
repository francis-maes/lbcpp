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
  OnlineLearningInferenceCallback(InferencePtr targetInference, const std::set<InferencePtr>& learnedInferences)
    : learnedInferences(learnedInferences), targetInference(targetInference), learningStopped(false)
  {
    for (std::set<InferencePtr>::const_iterator it = learnedInferences.begin(); it != learnedInferences.end(); ++it)
    {
      const InferenceOnlineLearnerPtr& learner = (*it)->getOnlineLearner();
      jassert(learner);
      learner->startLearningCallback();
      jassert(!learner->isLearningStopped());
    }
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    const InferencePtr& inference = stack->getCurrentInference();
    if (inference->getOnlineLearner() && supervision.exists())
    {
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      jassert(learnedInferences.find(inference) != learnedInferences.end());
      if (!learner->isLearningStopped())
        learner->stepFinishedCallback(inference, input, supervision, output);
    }
    else if (inference == targetInference)
      finishEpisode();
    else if (isRunOnSupervisedExamplesInference(inference))
      finishPass();
  }

  bool isLearningStopped() const
    {return learningStopped;}

private:
  std::set<InferencePtr> learnedInferences;
  InferencePtr targetInference;
  bool learningStopped;

  void finishEpisode()
  {
    for (std::set<InferencePtr>::const_iterator it = learnedInferences.begin(); it != learnedInferences.end(); ++it)
    {
      const InferenceOnlineLearnerPtr& learner = (*it)->getOnlineLearner();
      if (!learner->isLearningStopped())
        learner->episodeFinishedCallback(*it);
    }
  }

  void finishPass()
  {
    bool wantsMoreIterations = false;
    for (std::set<InferencePtr>::const_iterator it = learnedInferences.begin(); it != learnedInferences.end(); ++it)
    {
      const InferenceOnlineLearnerPtr& learner = (*it)->getOnlineLearner();
      if (!learner->isLearningStopped())
      {
        learner->passFinishedCallback(*it);
        wantsMoreIterations |= learner->wantsMoreIterations();
      }
    }
    learningStopped = !wantsMoreIterations;
  }
};

typedef ReferenceCountedObjectPtr<OnlineLearningInferenceCallback> OnlineLearningInferenceCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_ONLINE_LEARNING_H_

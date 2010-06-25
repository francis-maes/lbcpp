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

namespace lbcpp
{

class OnlineLearningInferenceCallback : public InferenceCallback
{
public:
  OnlineLearningInferenceCallback(InferencePtr batchLearner)
    : batchLearner(batchLearner), learningStopped(false) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    InferencePtr inference = stack->getCurrentInference();
    if (inference->getOnlineLearner() && supervision)
    {
      InferenceOnlineLearnerPtr learner = inference->getOnlineLearner();
      if (!learner->isLearningStopped())
      {
        learners[inference] = learner;
        learner->stepFinishedCallback(inference, input, supervision, output);
      }
    }
    else if (stack->getGrandParentInference() == batchLearner)
      finishEpisode();
    else if (stack->getParentInference() == batchLearner)
      finishPass();
  }

  bool isLearningStopped() const
    {return learningStopped;}

private:
  typedef std::map<InferencePtr, InferenceOnlineLearnerPtr> LearnersMap;
  LearnersMap learners;
  InferencePtr batchLearner;
  bool learningStopped;

  void finishEpisode()
  {
    for (LearnersMap::const_iterator it = learners.begin(); it != learners.end(); ++it)
      it->second->episodeFinishedCallback(it->first);
  }

  void finishPass()
  {
    learningStopped = true;
    for (LearnersMap::const_iterator it = learners.begin(); it != learners.end(); ++it)
    {
      it->second->passFinishedCallback(it->first);
      learningStopped &= it->second->isLearningStopped();
    }
    learners.clear();
  }
};

typedef ReferenceCountedObjectPtr<OnlineLearningInferenceCallback> OnlineLearningInferenceCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_ONLINE_LEARNING_H_

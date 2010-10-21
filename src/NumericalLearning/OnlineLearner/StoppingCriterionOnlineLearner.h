/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterionOnlineLearner.h| Adds a stopping criterion to an|
| Author  : Francis Maes                   |  online learner                 |
| Started : 26/05/2010 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/Inference.h>

namespace lbcpp
{

class StoppingCriterionOnlineLearner : public UpdatableOnlineLearner
{
public:
  StoppingCriterionOnlineLearner(InferenceOnlineLearnerPtr learner, UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops)
    : UpdatableOnlineLearner(criterionTestFrequency), learner(learner), 
        criterion(criterion), restoreBestParametersWhenLearningStops(restoreBestParametersWhenLearningStops),
        learningStopped(false), bestScore(-DBL_MAX)
     {criterion->reset();}

  StoppingCriterionOnlineLearner() : learningStopped(false), bestScore(-DBL_MAX) {}

  virtual void startLearningCallback()
  {
    UpdatableOnlineLearner::startLearningCallback();
    learningStopped = false;
    bestParameters = ObjectPtr();
    bestScore = -DBL_MAX;
    learner->startLearningCallback();
  }

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    learner->stepFinishedCallback(inference, input, supervision, prediction);
    UpdatableOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
  }

  virtual void episodeFinishedCallback(const InferencePtr& inference)
  {
    jassert(!learningStopped);
    learner->episodeFinishedCallback(inference);
    UpdatableOnlineLearner::episodeFinishedCallback(inference);
  }

  virtual void passFinishedCallback(const InferencePtr& inference)
  {
    jassert(!learningStopped);
    learner->passFinishedCallback(inference);
    UpdatableOnlineLearner::passFinishedCallback(inference);
  }

  virtual double getCurrentLossEstimate() const
    {return learner->getCurrentLossEstimate();}

  virtual bool isLearningStopped() const
    {return learningStopped;}

  virtual void clone(ObjectPtr target) const
  {
    UpdatableOnlineLearner::clone(target);
    target.staticCast<StoppingCriterionOnlineLearner>()->criterion = criterion->cloneAndCast<StoppingCriterion>();
  }

private:
  friend class StoppingCriterionOnlineLearnerClass;

  InferenceOnlineLearnerPtr learner;
  StoppingCriterionPtr criterion;
  bool restoreBestParametersWhenLearningStops;

  bool learningStopped;
  Variable bestParameters;
  double bestScore;

  virtual void update(const InferencePtr& inference)
  {
    double score = -getCurrentLossEstimate();
    Variable parameters = inference->getParameters();
    if (parameters.exists() && restoreBestParametersWhenLearningStops && score > bestScore)
    {
      bestParameters = parameters;
      bestScore = score;
      //MessageCallback::info(T("StoppingCriterionOnlineLearner::update"), T("New best score: ") + String((double)bestScore));
    }
    if (criterion->shouldStop(score))
    {
      //MessageCallback::info(T("StoppingCriterionOnlineLearner::update"), T("Stopped, best score = ") + String((double)bestScore));
      learningStopped = true;
      if (bestParameters.exists() && bestScore > score)
      {
        MessageCallback::info(T("StoppingCriterionOnlineLearner::update"), T("Restoring parameters that led to score ") + String((double)bestScore));
        inference->setParameters(bestParameters);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_

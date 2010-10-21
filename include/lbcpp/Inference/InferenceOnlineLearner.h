/*-----------------------------------------.---------------------------------.
| Filename: InferenceOnlineLearner.h       | Inference Online Learners       |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_H_

# include "Inference.h"
# include "../Function/ScalarObjectFunction.h"
# include "../Function/IterationFunction.h"
# include "../Data/RandomVariable.h"

namespace lbcpp
{

class InferenceOnlineLearner : public Object
{
public:
  virtual void startLearningCallback() = 0;
  virtual void subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction) {}
  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction) = 0;
  virtual void episodeFinishedCallback(const InferencePtr& inference) = 0;
  virtual void passFinishedCallback(const InferencePtr& inference) = 0;

  virtual double getCurrentLossEstimate() const;

  virtual bool wantsMoreIterations() const
    {return !isLearningStopped();}

  virtual bool isLearningStopped() const
    {return false;}

  enum UpdateFrequency
  {
    never,
    perStep,
    perEpisode,
    perPass,
    perStepMiniBatch,
    perStepMiniBatch2 = perStepMiniBatch + 2,
    perStepMiniBatch5 = perStepMiniBatch + 5,
    perStepMiniBatch10 = perStepMiniBatch + 10,
    perStepMiniBatch20 = perStepMiniBatch + 20,
    perStepMiniBatch50 = perStepMiniBatch + 50,
    perStepMiniBatch100 = perStepMiniBatch + 100,
    perStepMiniBatch200 = perStepMiniBatch + 200,
    perStepMiniBatch500 = perStepMiniBatch + 500,
    perStepMiniBatch1000 = perStepMiniBatch + 1000,
  };

  InferenceOnlineLearnerPtr addStoppingCriterion(UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops = true) const;

  const InferenceOnlineLearnerPtr& getNextLearner() const
    {return nextLearner;}

  void setNextLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->previousLearner = this; nextLearner = learner;}

  void setPreviousLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->nextLearner = this; previousLearner = learner;}

protected:
  friend class InferenceOnlineLearnerClass;

  InferenceOnlineLearnerPtr nextLearner;
  InferenceOnlineLearnerPtr previousLearner;
};

extern ClassPtr inferenceOnlineLearnerClass;

class UpdatableOnlineLearner : public InferenceOnlineLearner
{
public:
  UpdatableOnlineLearner(UpdateFrequency updateFrequency = never);

  virtual void update(const InferencePtr& inference) = 0;

  virtual void startLearningCallback()
    {epoch = 0;}
  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(const InferencePtr& inference);
  virtual void passFinishedCallback(const InferencePtr& inference);

protected:
  friend class UpdatableOnlineLearnerClass;

  size_t epoch;
  UpdateFrequency updateFrequency;
};

typedef ReferenceCountedObjectPtr<UpdatableOnlineLearner> UpdatableOnlineLearnerPtr;

extern InferenceOnlineLearnerPtr stoppingCriterionOnlineLearner(InferenceOnlineLearnerPtr learner,
                    InferenceOnlineLearner::UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_ONLINE_LEARNER_H_

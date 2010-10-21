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
  virtual void startLearningCallback()
    {if (nextLearner) nextLearner->startLearningCallback();}

  virtual void subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
    {if (nextLearner) nextLearner->subStepFinishedCallback(inference, input, supervision, prediction);}

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
    {if (nextLearner) nextLearner->stepFinishedCallback(inference, input, supervision, prediction);}

  virtual void episodeFinishedCallback(const InferencePtr& inference)
    {if (nextLearner) nextLearner->episodeFinishedCallback(inference);}

  virtual void passFinishedCallback(const InferencePtr& inference)
    {if (nextLearner) nextLearner->passFinishedCallback(inference);}

  virtual double getCurrentLossEstimate() const;

  virtual bool wantsMoreIterations() const
    {return !isLearningStopped();}

  virtual bool isLearningStopped() const
    {return nextLearner && nextLearner->isLearningStopped();}

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

  const InferenceOnlineLearnerPtr& getNextLearner() const
    {return nextLearner;}

  void setNextLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->previousLearner = this; nextLearner = learner;}

  void setPreviousLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->nextLearner = this; previousLearner = learner;}

  virtual void clone(const ObjectPtr& target) const;

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
  {
    epoch = 0;
    InferenceOnlineLearner::startLearningCallback();
  }

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(const InferencePtr& inference);
  virtual void passFinishedCallback(const InferencePtr& inference);

protected:
  friend class UpdatableOnlineLearnerClass;

  size_t epoch;
  UpdateFrequency updateFrequency;

  void updateAfterStep(const InferencePtr& inference);
};

typedef ReferenceCountedObjectPtr<UpdatableOnlineLearner> UpdatableOnlineLearnerPtr;

extern UpdatableOnlineLearnerPtr stoppingCriterionOnlineLearner(InferenceOnlineLearner::UpdateFrequency criterionTestFrequency,
                                                                StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops);

extern UpdatableOnlineLearnerPtr randomizerOnlineLearner(InferenceOnlineLearner::UpdateFrequency randomizationFrequency);
}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_ONLINE_LEARNER_H_

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
  virtual void startLearningCallback(InferenceContextWeakPtr context);
  virtual void subStepFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void stepFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference);
  virtual void passFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference);

  virtual void getScores(std::vector< std::pair<String, double> >& res) const;
  virtual double getDefaultScore() const;

  virtual bool wantsMoreIterations() const;
  virtual bool isLearningStopped() const;

  const InferenceOnlineLearnerPtr& getNextLearner() const
    {return nextLearner;}

  const InferenceOnlineLearnerPtr& getPreviousLearner() const
    {return previousLearner;}

  InferenceOnlineLearnerPtr getLastLearner() const;

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

enum LearnerUpdateFrequency
{
  never = 0,
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

class UpdatableOnlineLearner : public InferenceOnlineLearner
{
public:
  UpdatableOnlineLearner(LearnerUpdateFrequency updateFrequency = never);

  virtual void update(InferenceContextWeakPtr context, const InferencePtr& inference) = 0;

  virtual void startLearningCallback(InferenceContextWeakPtr context)
    {epoch = 0; InferenceOnlineLearner::startLearningCallback(context);}

  virtual void stepFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference);
  virtual void passFinishedCallback(InferenceContextWeakPtr context, const InferencePtr& inference);

protected:
  friend class UpdatableOnlineLearnerClass;

  size_t epoch;
  LearnerUpdateFrequency updateFrequency;

  void updateAfterStep(InferenceContextWeakPtr context, const InferencePtr& inference);
};

typedef ReferenceCountedObjectPtr<UpdatableOnlineLearner> UpdatableOnlineLearnerPtr;

extern UpdatableOnlineLearnerPtr randomizerOnlineLearner(LearnerUpdateFrequency randomizationFrequency = perPass);
extern UpdatableOnlineLearnerPtr stoppingCriterionOnlineLearner(StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops, LearnerUpdateFrequency criterionTestFrequency = perPass);

extern UpdatableOnlineLearnerPtr computeScoreOnlineLearner(FunctionPtr scoreFunction, LearnerUpdateFrequency computeFrequency = perPass);
extern UpdatableOnlineLearnerPtr computeEvaluatorOnlineLearner(EvaluatorPtr evaluator, ContainerPtr examples, LearnerUpdateFrequency computeFrequency = perPass);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_ONLINE_LEARNER_H_

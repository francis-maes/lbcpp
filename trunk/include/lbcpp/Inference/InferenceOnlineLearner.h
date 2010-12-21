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
  virtual void startLearningCallback(ExecutionContext& context);
  virtual void subStepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference);
  // batchLearnerInput: if learning is performed within the context of a batch learner, we have batch learning parameters here
  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput);

  virtual void getScores(std::vector< std::pair<String, double> >& res) const;
  virtual double getDefaultScore() const;

  virtual bool wantsMoreIterations() const;
  virtual bool isLearningStopped() const;

  const InferenceOnlineLearnerPtr& getNextLearner() const
    {return nextLearner;}

  const InferenceOnlineLearnerPtr& getPreviousLearner() const
    {return previousLearner;}

  InferenceOnlineLearnerPtr getLastLearner() const;

  InferenceOnlineLearnerPtr setNextLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->previousLearner = this; nextLearner = learner; return learner;}

  void setPreviousLearner(const InferenceOnlineLearnerPtr& learner)
    {if (learner) learner->nextLearner = this; previousLearner = learner;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

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

  virtual void update(ExecutionContext& context, const InferencePtr& inference) = 0;

  virtual void startLearningCallback(ExecutionContext& context)
    {epoch = 0; InferenceOnlineLearner::startLearningCallback(context);}

  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference);
  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput);

protected:
  friend class UpdatableOnlineLearnerClass;

  size_t epoch;
  LearnerUpdateFrequency updateFrequency;

  void updateAfterStep(ExecutionContext& context, const InferencePtr& inference);
};

typedef ReferenceCountedObjectPtr<UpdatableOnlineLearner> UpdatableOnlineLearnerPtr;

extern UpdatableOnlineLearnerPtr randomizerOnlineLearner(LearnerUpdateFrequency randomizationFrequency = perPass);
extern UpdatableOnlineLearnerPtr stoppingCriterionOnlineLearner(StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops, LearnerUpdateFrequency criterionTestFrequency = perPass);

extern UpdatableOnlineLearnerPtr computeScoreOnlineLearner(FunctionPtr scoreFunction, LearnerUpdateFrequency computeFrequency = perPass);
extern InferenceOnlineLearnerPtr computeEvaluatorOnlineLearner(EvaluatorPtr evaluator, bool computeOnValidationData);

extern UpdatableOnlineLearnerPtr saveScoresToGnuPlotFileOnlineLearner(const File& outputFile, LearnerUpdateFrequency updateFrequency = perPass);

class InferenceOnlineLearnerParameters : public Object
{
public:
  virtual InferenceOnlineLearnerPtr createLearner() const = 0;
};

extern ClassPtr inferenceOnlineLearnerParametersClass;
typedef ReferenceCountedObjectPtr<InferenceOnlineLearnerParameters> InferenceOnlineLearnerParametersPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_ONLINE_LEARNER_H_

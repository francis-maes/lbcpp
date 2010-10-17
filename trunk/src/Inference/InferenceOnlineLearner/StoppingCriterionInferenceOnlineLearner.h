/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterionLearningIn...h| Adds a stopping criterion to a  |
| Author  : Francis Maes                   |  learner                        |
| Started : 26/05/2010 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_

# include "GradientDescentOnlineLearner.h"
# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class StoppingCriterionInferenceOnlineLearner : public InferenceOnlineLearner
{
public:
  StoppingCriterionInferenceOnlineLearner(InferenceOnlineLearnerPtr learner, UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops)
    : learner(learner), criterionTestFrequency(criterionTestFrequency),
        criterion(criterion), restoreBestParametersWhenLearningStops(restoreBestParametersWhenLearningStops),
        learningStopped(false), bestScore(-DBL_MAX), epoch(0)
     {criterion->reset();}

  StoppingCriterionInferenceOnlineLearner() : criterionTestFrequency(never), learningStopped(false), bestScore(-DBL_MAX), epoch(0) {}

  virtual void startLearningCallback()
  {
    learningStopped = false;
    bestParameters = ObjectPtr();
    bestScore = -DBL_MAX;
    epoch = 0;
    learner->startLearningCallback();
  }

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    if (!learningStopped)
    {
      learner->stepFinishedCallback(inference, input, supervision, prediction);

      ++epoch;
      if (criterionTestFrequency == perStep)
        testStoppingCriterion(inference);
      else if (criterionTestFrequency >= perStepMiniBatch)
      {
        size_t miniBatchSize = criterionTestFrequency - perStepMiniBatch;
        if ((miniBatchSize <= 1) || (epoch % miniBatchSize) == 0)
          testStoppingCriterion(inference);
      }
    }
  }

  virtual void episodeFinishedCallback(const InferencePtr& inference)
  {
    if (!learningStopped)
    {
      learner->episodeFinishedCallback(inference);
      if (criterionTestFrequency == perEpisode)
        testStoppingCriterion(inference);
    }
  }

  virtual void passFinishedCallback(const InferencePtr& inference)
  {
    if (!learningStopped) 
    {
      learner->passFinishedCallback(inference);
      if (criterionTestFrequency == perPass)
        testStoppingCriterion(inference);
    }
  }

  virtual double getCurrentLossEstimate() const
    {return learner->getCurrentLossEstimate();}

  virtual bool isLearningStopped() const
    {return learningStopped;}

  virtual ObjectPtr clone() const
  {
    ReferenceCountedObjectPtr<StoppingCriterionInferenceOnlineLearner> res 
      = new StoppingCriterionInferenceOnlineLearner(learner->cloneAndCast<InferenceOnlineLearner>(), criterionTestFrequency,
                                                    criterion->cloneAndCast<StoppingCriterion>(), restoreBestParametersWhenLearningStops);
    res->learningStopped = learningStopped;
    res->bestParameters = bestParameters ? bestParameters->clone() : ObjectPtr();
    res->bestScore = bestScore;
    res->epoch = epoch;
    return res;
  }

private:
  InferenceOnlineLearnerPtr learner;
  UpdateFrequency criterionTestFrequency;
  StoppingCriterionPtr criterion;
  bool restoreBestParametersWhenLearningStops;

  bool learningStopped;
  ObjectPtr bestParameters;
  double bestScore;
  size_t epoch;

  NumericalInferencePtr getNumericalInference(InferencePtr inference) const
    {return inference.staticCast<NumericalInference>();}

  void testStoppingCriterion(InferencePtr inf)
  {
    NumericalInferencePtr inference = getNumericalInference(inf);

    double score = -getCurrentLossEstimate();
    ObjectPtr parameters = inference->getParametersCopy();
    if (parameters && restoreBestParametersWhenLearningStops && score > bestScore)
    {
      bestParameters = parameters;
      bestScore = score;
    }
    if (criterion->shouldStop(score))
    {
      learningStopped = true;
      if (bestParameters)
        getNumericalInference(inference)->setParameters(bestParameters);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_

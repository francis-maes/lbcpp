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
# include <lbcpp/Utilities/StoppingCriterion.h>

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

  virtual void stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    if (!learningStopped)
    {
      learner->stepFinishedCallback(inference, input, supervision, predictedOutput);

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

  virtual void episodeFinishedCallback(InferencePtr inference)
  {
    if (!learningStopped)
    {
      learner->episodeFinishedCallback(inference);
      if (criterionTestFrequency == perEpisode)
        testStoppingCriterion(inference);
    }
  }

  virtual void passFinishedCallback(InferencePtr inference)
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
    res->bestParameters = bestParameters ? bestParameters->cloneAndCast<DenseVector>() : DenseVectorPtr();
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
  DenseVectorPtr bestParameters;
  double bestScore;
  size_t epoch;

  void testStoppingCriterion(InferencePtr inference)
  {
    bool currentParametersAreBest = false;
    double score = -getCurrentLossEstimate();
    if (restoreBestParametersWhenLearningStops && score > bestScore)
    {
      bestParameters = getParameters(inference)->clone();
      bestScore = score;
      currentParametersAreBest = true;
    }
    if (criterion->shouldOptimizerStop(score))
    {
      learningStopped = true;
      if (bestParameters && !currentParametersAreBest)
        getParameterizedInference(inference)->setParameters(bestParameters);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_

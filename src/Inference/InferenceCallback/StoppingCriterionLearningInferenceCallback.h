/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterionLearningIn...h| Adds a stopping criterion to a  |
| Author  : Francis Maes                   |  learner                        |
| Started : 26/05/2010 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_

# include "GradientDescentLearningCallback.h"
# include <lbcpp/Utilities/StoppingCriterion.h>

namespace lbcpp
{

class StoppingCriterionLearningInferenceCallback : public LearningInferenceCallback
{
public:
  StoppingCriterionLearningInferenceCallback(LearningInferenceCallbackPtr learner, UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops)
    : LearningInferenceCallback(learner->getInference()), learner(learner), criterionTestFrequency(criterionTestFrequency),
        criterion(criterion), restoreBestParametersWhenLearningStops(restoreBestParametersWhenLearningStops),
        learningStopped(false), bestScore(-DBL_MAX), epoch(0)
     {criterion->reset();}
  
  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    if (!learningStopped)
    {
      learner->stepFinishedCallback(input, supervision, predictedOutput);

      ++epoch;
      if (criterionTestFrequency == perStep)
        testStoppingCriterion();
      else if (criterionTestFrequency >= perStepMiniBatch)
      {
        size_t miniBatchSize = criterionTestFrequency - perStepMiniBatch;
        if ((miniBatchSize <= 1) || (epoch % miniBatchSize) == 0)
          testStoppingCriterion();
      }
    }
  }

  virtual void episodeFinishedCallback()
  {
    if (!learningStopped)
    {
      learner->episodeFinishedCallback();
      if (criterionTestFrequency == perEpisode)
        testStoppingCriterion();
    }
  }

  virtual void passFinishedCallback()
  {
    if (!learningStopped) 
    {
      learner->passFinishedCallback();
      if (criterionTestFrequency == perPass)
        testStoppingCriterion();
    }
  }

  virtual double getCurrentLossEstimate() const
    {return learner->getCurrentLossEstimate();}

  virtual bool isLearningStopped() const
    {return learningStopped;}

private:
  LearningInferenceCallbackPtr learner;
  UpdateFrequency criterionTestFrequency;
  StoppingCriterionPtr criterion;
  bool restoreBestParametersWhenLearningStops;

  bool learningStopped;
  DenseVectorPtr bestParameters;
  double bestScore;
  size_t epoch;

  void testStoppingCriterion()
  {
    bool currentParametersAreBest = false;
    double score = -getCurrentLossEstimate();
    if (restoreBestParametersWhenLearningStops && score > bestScore)
    {
      bestParameters = getParameters()->clone();
      bestScore = score;
      currentParametersAreBest = true;
    }
    if (criterion->shouldOptimizerStop(score))
    {
      learningStopped = true;
      if (bestParameters && !currentParametersAreBest)
        getInference()->setParameters(bestParameters);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOPPING_CRITERION_LEARNING_H_

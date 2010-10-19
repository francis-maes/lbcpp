/*-----------------------------------------.---------------------------------.
| Filename: InferenceOnlineLearner.cpp     | Inference Online Learners       |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceOnlineLearner.h>
using namespace lbcpp;

/*
** InferenceOnlineLearner
*/
InferenceOnlineLearnerPtr InferenceOnlineLearner::addStoppingCriterion(UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops) const
{
  return stoppingCriterionOnlineLearner(refCountedPointerFromThis(this),
    criterionTestFrequency, criterion, restoreBestParametersWhenLearningStops);
}

/*
** UpdatableOnlineLearner
*/
UpdatableOnlineLearner::UpdatableOnlineLearner(UpdateFrequency updateFrequency)
  : epoch(0), updateFrequency(updateFrequency) {}

void UpdatableOnlineLearner::stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  ++epoch;
  if (updateFrequency == perStep)
    update(inference);
  if (updateFrequency >= perStepMiniBatch)
  {
    int miniBatchSize = updateFrequency - perStepMiniBatch;
    if (miniBatchSize <= 1 || (epoch % miniBatchSize) == 0)
      update(inference);
  }
}

void UpdatableOnlineLearner::episodeFinishedCallback(const InferencePtr& inference)
{
  if (updateFrequency == perEpisode)
    update(inference);
}

void UpdatableOnlineLearner::passFinishedCallback(const InferencePtr& inference)
{
  if (updateFrequency == perPass)
    update(inference);
}


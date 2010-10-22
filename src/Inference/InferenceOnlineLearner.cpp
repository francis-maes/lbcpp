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
void InferenceOnlineLearner::clone(const ObjectPtr& t) const
{
  Object::clone(t);
  const InferenceOnlineLearnerPtr& target = t.staticCast<InferenceOnlineLearner>();
  if (nextLearner)
    target->setNextLearner(nextLearner->cloneAndCast<InferenceOnlineLearner>());
}

double InferenceOnlineLearner::getCurrentLossEstimate() const
{
  if (previousLearner)
    return previousLearner->getCurrentLossEstimate();
  else
  {
    jassert(false);
    return 0.0;
  }
}

InferenceOnlineLearnerPtr InferenceOnlineLearner::getLastLearner() const
{
  InferenceOnlineLearnerPtr last = refCountedPointerFromThis(this);
  while (last->getNextLearner())
    last = last->getNextLearner();
  return last;
}

/*
** UpdatableOnlineLearner
*/
UpdatableOnlineLearner::UpdatableOnlineLearner(UpdateFrequency updateFrequency)
  : epoch(0), updateFrequency(updateFrequency) {}

void UpdatableOnlineLearner::updateAfterStep(const InferencePtr& inference)
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

void UpdatableOnlineLearner::stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  updateAfterStep(inference);
  InferenceOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
}

void UpdatableOnlineLearner::episodeFinishedCallback(const InferencePtr& inference)
{
  if (updateFrequency == perEpisode)
    update(inference);
  InferenceOnlineLearner::episodeFinishedCallback(inference);
}

void UpdatableOnlineLearner::passFinishedCallback(const InferencePtr& inference)
{
  if (updateFrequency == perPass)
    update(inference);
  InferenceOnlineLearner::passFinishedCallback(inference);
}


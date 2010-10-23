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
void InferenceOnlineLearner::startLearningCallback()
{
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);
    nextLearner->startLearningCallback();
  }
}

void InferenceOnlineLearner::subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {if (nextLearner) nextLearner->subStepFinishedCallback(inference, input, supervision, prediction);}

void InferenceOnlineLearner::stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {if (nextLearner) nextLearner->stepFinishedCallback(inference, input, supervision, prediction);}

void InferenceOnlineLearner::episodeFinishedCallback(const InferencePtr& inference)
  {if (nextLearner) nextLearner->episodeFinishedCallback(inference);}

void InferenceOnlineLearner::passFinishedCallback(const InferencePtr& inference)
{
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);
    nextLearner->passFinishedCallback(inference);
  }
}

bool InferenceOnlineLearner::wantsMoreIterations() const
  {return !isLearningStopped();}

bool InferenceOnlineLearner::isLearningStopped() const
  {return nextLearner && nextLearner->isLearningStopped();}

void InferenceOnlineLearner::clone(const ObjectPtr& t) const
{
  Object::clone(t);
  const InferenceOnlineLearnerPtr& target = t.staticCast<InferenceOnlineLearner>();
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);

    target->setNextLearner(nextLearner->cloneAndCast<InferenceOnlineLearner>());
    jassert(target->getNextLearner()->getPreviousLearner() == target);
  }
}

double InferenceOnlineLearner::getCurrentLossEstimate() const
{
  if (previousLearner)
  {
    jassert(previousLearner->getNextLearner().get() == this);
    return previousLearner->getCurrentLossEstimate();
  }
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


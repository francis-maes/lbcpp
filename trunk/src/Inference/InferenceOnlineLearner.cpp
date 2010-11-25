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
void InferenceOnlineLearner::startLearningCallback(InferenceContext& context)
{
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);
    nextLearner->startLearningCallback(context);
  }
}

void InferenceOnlineLearner::subStepFinishedCallback(InferenceContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {if (nextLearner) nextLearner->subStepFinishedCallback(context, inference, input, supervision, prediction);}

void InferenceOnlineLearner::stepFinishedCallback(InferenceContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {if (nextLearner) nextLearner->stepFinishedCallback(context, inference, input, supervision, prediction);}

void InferenceOnlineLearner::episodeFinishedCallback(InferenceContext& context, const InferencePtr& inference)
  {if (nextLearner) nextLearner->episodeFinishedCallback(context, inference);}

void InferenceOnlineLearner::passFinishedCallback(InferenceContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
{
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);
    nextLearner->passFinishedCallback(context, inference, batchLearnerInput);
  }
}

bool InferenceOnlineLearner::wantsMoreIterations() const
  {return !isLearningStopped();}

bool InferenceOnlineLearner::isLearningStopped() const
  {return nextLearner && nextLearner->isLearningStopped();}

void InferenceOnlineLearner::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Object::clone(context, t);
  const InferenceOnlineLearnerPtr& target = t.staticCast<InferenceOnlineLearner>();
  if (nextLearner)
  {
    jassert(nextLearner->getPreviousLearner().get() == this);

    target->setNextLearner(nextLearner->cloneAndCast<InferenceOnlineLearner>(context));
    jassert(target->getNextLearner()->getPreviousLearner() == target);
  }
}

void InferenceOnlineLearner::getScores(std::vector< std::pair<String, double> >& res) const
{
  if (previousLearner)
    previousLearner->getScores(res);
}

double InferenceOnlineLearner::getDefaultScore() const
{
  if (previousLearner)
    return previousLearner->getDefaultScore();
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
UpdatableOnlineLearner::UpdatableOnlineLearner(LearnerUpdateFrequency updateFrequency)
  : epoch(0), updateFrequency(updateFrequency) {}

void UpdatableOnlineLearner::updateAfterStep(InferenceContext& context, const InferencePtr& inference)
{
  ++epoch;
  if (updateFrequency == perStep)
    update(context, inference);
  else if (updateFrequency >= perStepMiniBatch)
  {
    int miniBatchSize = updateFrequency - perStepMiniBatch;
    if (miniBatchSize <= 1 || (epoch % miniBatchSize) == 0)
      update(context, inference);
  }
}

void UpdatableOnlineLearner::stepFinishedCallback(InferenceContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  updateAfterStep(context, inference);
  InferenceOnlineLearner::stepFinishedCallback(context, inference, input, supervision, prediction);
}

void UpdatableOnlineLearner::episodeFinishedCallback(InferenceContext& context, const InferencePtr& inference)
{
  if (updateFrequency == perEpisode)
    update(context, inference);
  InferenceOnlineLearner::episodeFinishedCallback(context, inference);
}

void UpdatableOnlineLearner::passFinishedCallback(InferenceContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
{
  if (updateFrequency == perPass)
    update(context, inference);
  InferenceOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);
}


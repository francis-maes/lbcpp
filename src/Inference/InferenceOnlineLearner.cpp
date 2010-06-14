/*-----------------------------------------.---------------------------------.
| Filename: InferenceOnlineLearner.cpp     | Inference Online Learners       |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "InferenceOnlineLearner/StochasticGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/MiniBatchGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/BatchGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/RandomizerInferenceOnlineLearner.h"
#include "InferenceOnlineLearner/StoppingCriterionInferenceOnlineLearner.h"
using namespace lbcpp;

DenseVectorPtr InferenceOnlineLearner::getParameters(InferencePtr inference) const
  {return getParameterizedInference(inference)->getParameters();}

static bool isRandomizationRequired(InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency, InferenceOnlineLearner::UpdateFrequency randomizationFrequency)
{
  jassert(learningUpdateFrequency != InferenceOnlineLearner::never);

  if (randomizationFrequency == InferenceOnlineLearner::never ||
      randomizationFrequency == InferenceOnlineLearner::perStep)
    return false;
  if (learningUpdateFrequency == InferenceOnlineLearner::perStep ||
      learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch)
    return true;
  if (learningUpdateFrequency == InferenceOnlineLearner::perEpisode)
  {
    jassert(randomizationFrequency != InferenceOnlineLearner::perPass); // this combination is not supported
    return false;
  }
  if (learningUpdateFrequency == InferenceOnlineLearner::perPass)
    return false;
  jassert(false);
  return false;
}

InferenceOnlineLearnerPtr lbcpp::gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::UpdateFrequency randomizationFrequency,
        InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate,
        InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer,
        InferenceOnlineLearner::UpdateFrequency criterionTestFrequency, StoppingCriterionPtr stoppingCriterion, bool restoreBestParametersWhenLearningStops)
{
  jassert(learningUpdateFrequency != InferenceOnlineLearner::never);
  InferenceOnlineLearnerPtr res;

  size_t miniBatchSize = 0;
  if (learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch)
  {
    miniBatchSize = learningUpdateFrequency - InferenceOnlineLearner::perStepMiniBatch;
    if (miniBatchSize <= 1)
      learningUpdateFrequency = InferenceOnlineLearner::perStep;
  }

  // base learner
  if (learningUpdateFrequency == InferenceOnlineLearner::perStep)
    res = new StochasticGradientDescentOnlineLearner(learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else if (learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch && miniBatchSize < 100)
    res = new MiniBatchGradientDescentOnlineLearner(miniBatchSize, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else
    res = new BatchGradientDescentOnlineLearner(learningUpdateFrequency,
                                              learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);

  // randomization
  if (isRandomizationRequired(learningUpdateFrequency, randomizationFrequency))
    res = InferenceOnlineLearnerPtr(new RandomizerInferenceOnlineLearner(randomizationFrequency, res));

  // stopping criterion and best parameters restore
  jassert(!restoreBestParametersWhenLearningStops || stoppingCriterion);
  if (stoppingCriterion)
  {
    jassert(criterionTestFrequency != InferenceOnlineLearner::never);
    res = res->addStoppingCriterion(criterionTestFrequency, stoppingCriterion, restoreBestParametersWhenLearningStops);
  }
  else
    jassert(!restoreBestParametersWhenLearningStops);
  return res;
}

InferenceOnlineLearnerPtr InferenceOnlineLearner::addStoppingCriterion(UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops) const
{
  InferenceOnlineLearnerPtr pthis(const_cast<InferenceOnlineLearner* >(this));
  return new StoppingCriterionInferenceOnlineLearner(pthis, criterionTestFrequency, criterion, restoreBestParametersWhenLearningStops);
}

void declareInferenceOnlineLearnerClasses()
{
  LBCPP_DECLARE_CLASS(StochasticGradientDescentOnlineLearner);
  LBCPP_DECLARE_CLASS(MiniBatchGradientDescentOnlineLearner);
  LBCPP_DECLARE_CLASS(BatchGradientDescentOnlineLearner);
  LBCPP_DECLARE_CLASS(RandomizerInferenceOnlineLearner);
  LBCPP_DECLARE_CLASS(StoppingCriterionInferenceOnlineLearner);
}

#include "InferenceBatchLearner/SimulationInferenceBatchLearner.h"
#include "InferenceBatchLearner/SequentialInferenceBatchLearner.h"

InferenceBatchLearnerPtr lbcpp::simulationInferenceBatchLearner()
  {return new SimulationInferenceBatchLearner();}

InferenceBatchLearnerPtr lbcpp::sharedSequentialInferenceBatchLearner(InferenceBatchLearnerPtr subLearner)
  {return new SharedSequentialInferenceBatchLearner(subLearner);}

InferenceBatchLearnerPtr lbcpp::vectorSequentialInferenceBatchLearner(const std::vector<InferenceBatchLearnerPtr>& subLearners)
  {return new VectorSequentialInferenceBatchLearner(subLearners);}
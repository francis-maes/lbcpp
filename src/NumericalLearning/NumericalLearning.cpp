/*-----------------------------------------.---------------------------------.
| Filename: NumericalLearning.cpp          | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 30/09/2010 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarObjectFunction.h>
#include <lbcpp/Perception/PerceptionMaths.h>
#include <lbcpp/NumericalLearning/NumericalLearning.h>
using namespace lbcpp;

NumericalInference::NumericalInference(const String& name, PerceptionPtr perception)
  : ParameterizedInference(name), perception(perception) {}

void NumericalInference::addWeightedToParameters(const ObjectPtr& value, double weight)
{
  if (weight)
  {
    parametersLock.enterWrite();
    lbcpp::addWeighted(parameters, value, weight);
    parametersLock.exitWrite();
    parametersChangedCallback();
  }
}

void NumericalInference::applyRegularizerToParameters(ScalarObjectFunctionPtr regularizer, double weight)
{
  if (weight)
  {
    parametersLock.enterWrite();
    bool changed = true;
    if (parameters)
      regularizer->compute(parameters, NULL, &parameters, weight);
    else
      changed = false;
    parametersLock.exitWrite();
    if (changed)
      parametersChangedCallback();
  }
}

#include "OnlineLearner/StochasticGradientDescentOnlineLearner.h"
#include "OnlineLearner/BatchGradientDescentOnlineLearner.h"
#include "OnlineLearner/RandomizerInferenceOnlineLearner.h"

namespace lbcpp
{
  extern InferenceOnlineLearnerPtr randomizerInferenceOnlineLearner(
    InferenceOnlineLearner::UpdateFrequency randomizationFrequency, InferenceOnlineLearnerPtr targetLearningCallback);
 
  extern GradientDescentOnlineLearnerPtr stochasticGradientDescentOnlineLearner(
    IterationFunctionPtr learningRate, bool normalizeLearningRate,
    InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency,
    ScalarObjectFunctionPtr regularizer);

  extern GradientDescentOnlineLearnerPtr batchGradientDescentOnlineLearner(
    InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate,
    InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer);
  
};

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
        InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer,
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
    res = stochasticGradientDescentOnlineLearner(learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  //else if (learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch && miniBatchSize < 100)
  //  res = miniBatchGradientDescentOnlineLearner(miniBatchSize, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else
    res = batchGradientDescentOnlineLearner(learningUpdateFrequency,
                                              learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);

  // randomization
  if (isRandomizationRequired(learningUpdateFrequency, randomizationFrequency))
    res = InferenceOnlineLearnerPtr(randomizerInferenceOnlineLearner(randomizationFrequency, res));

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

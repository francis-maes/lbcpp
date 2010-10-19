/*-----------------------------------------.---------------------------------.
| Filename: NumericalLearning.cpp          | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 30/09/2010 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarObjectFunction.h>
#include <lbcpp/NumericalLearning/NumericalLearning.h>
using namespace lbcpp;

/*
** Math
*/
#include "Math/DoubleUnaryOperation.h"
#include "Math/DoubleConstUnaryOperation.h"
#include "Math/DoubleDotProductOperation.h"
#include "Math/DoubleAssignmentOperation.h"

/*
** Perception
*/
namespace lbcpp
{
  extern FunctionPtr multiplyDoubleFunction();
};

PerceptionPtr lbcpp::defaultPositiveIntegerFeatures(size_t numIntervals, double maxPowerOfTen)
  {return softDiscretizedLogNumberFeatures(positiveIntegerType, 0.0, maxPowerOfTen, numIntervals, true);}

PerceptionPtr lbcpp::defaultIntegerFeatures(size_t numIntervals, double maxPowerOfTen)
  {return signedNumberFeatures(softDiscretizedLogNumberFeatures(integerType, 0.0, maxPowerOfTen, numIntervals, true));}

PerceptionPtr lbcpp::defaultProbabilityFeatures(size_t numIntervals)
  {return softDiscretizedNumberFeatures(probabilityType, 0.0, 1.0, numIntervals, false, false);}

PerceptionPtr lbcpp::defaultPositiveDoubleFeatures(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return softDiscretizedLogNumberFeatures(doubleType, minPowerOfTen, maxPowerOfTen, numIntervals, true);}

PerceptionPtr lbcpp::defaultDoubleFeatures(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return signedNumberFeatures(defaultPositiveDoubleFeatures(numIntervals, minPowerOfTen, maxPowerOfTen));}

PerceptionPtr lbcpp::conjunctionFeatures(PerceptionPtr perception1, PerceptionPtr perception2)
  {jassert(perception1 && perception2); return productPerception(multiplyDoubleFunction(), perception1, perception2, true, true);}

PerceptionPtr lbcpp::selectAndMakeConjunctionFeatures(PerceptionPtr decorated, const std::vector< std::vector<size_t> >& selectedConjunctions)
  {return selectAndMakeProductsPerception(decorated, multiplyDoubleFunction(), selectedConjunctions);}

PerceptionPtr lbcpp::perceptionToFeatures(PerceptionPtr perception)
{
  PerceptionRewriterPtr rewriter = new PerceptionRewriter(false);

  rewriter->addRule(booleanType, booleanFeatures());
  rewriter->addRule(enumValueFeaturesPerceptionRewriteRule());

  rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
  rewriter->addRule(probabilityType, defaultProbabilityFeatures());
  rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
  rewriter->addRule(integerType, defaultIntegerFeatures());

  rewriter->addRule(doubleType, identityPerception());
  return rewriter->rewrite(perception);
}

/*
** Inference
*/
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

/*
** OnlineLearner
*/
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


/*-----------------------------------------.---------------------------------.
| Filename: NumericalLearning.cpp          | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 30/09/2010 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarObjectFunction.h>
#include <lbcpp/NumericalLearning/NumericalLearning.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
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
** NumericalInferenceParameters
*/
NumericalInferenceParameters::NumericalInferenceParameters(const PerceptionPtr& perception, TypePtr weightsType)
  : Object(numericalInferenceParametersClass(weightsType)), perception(perception) {}

void NumericalInferenceParameters::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  Object::clone(context, target);
  target.staticCast<NumericalInferenceParameters>()->weights = weights ? weights->deepClone(context) : ObjectPtr();
}

/*
** NumericalInference
*/
NumericalInference::NumericalInference(const String& name, PerceptionPtr perception)
  : Inference(name)
{
}

const NumericalInferenceParametersPtr& NumericalInference::getParameters() const
{
  ScopedReadLock _(parametersLock);
  return parameters.getObject().staticCast<NumericalInferenceParameters>();
}

const PerceptionPtr& NumericalInference::getPerception() const
{
  ScopedReadLock _(parametersLock);
  return getParameters()->getPerception();
}

const ObjectPtr& NumericalInference::getWeights() const
{
  ScopedReadLock _(parametersLock);
  return parameters.getObject().staticCast<NumericalInferenceParameters>()->getWeights();
}

ObjectPtr NumericalInference::getWeightsCopy(ExecutionContext& context) const
{
  ScopedReadLock _(parametersLock);
  ObjectPtr weights = getParameters()->getWeights();
  return weights ? weights->deepClone(context) : ObjectPtr();
}

void NumericalInference::setWeights(const ObjectPtr& newWeights)
{
  ScopedWriteLock _(parametersLock);
  getParameters()->getWeights() = newWeights;
}

void NumericalInference::addWeightedToParameters(ExecutionContext& context, const ObjectPtr& value, double weight)
{
  if (weight)
  {
    parametersLock.enterWrite();
    lbcpp::addWeighted(context, getParameters()->getWeights(), value, weight);
    parametersLock.exitWrite();
    parametersChangedCallback();
  }
}

void NumericalInference::addWeightedToParameters(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input, double weight)
{
  if (weight)
  {
    parametersLock.enterWrite();
    lbcpp::addWeighted(context, getParameters()->getWeights(), perception, input, weight);
    parametersLock.exitWrite();
    parametersChangedCallback();
  }
}

void NumericalInference::applyRegularizerToParameters(ExecutionContext& context, ScalarObjectFunctionPtr regularizer, double weight)
{
  if (weight)
  {
    parametersLock.enterWrite();
    bool changed = true;
    ObjectPtr weights = getParameters()->getWeights();
    if (weights)
      regularizer->compute(context, weights, NULL, &weights, weight);
    else
      changed = false;
    parametersLock.exitWrite();
    if (changed)
      parametersChangedCallback();
  }
}

void NumericalInference::updateParametersType(ExecutionContext& context)
{
  NumericalInferenceParametersPtr previousParameters = getParameters();
  const PerceptionPtr& perception = previousParameters->getPerception();
  const ObjectPtr& weights = previousParameters->getWeights();

  ScopedWriteLock _(parametersLock);
  TypePtr weightsType = getWeightsType(perception->getOutputType());
  parameters = Variable(new NumericalInferenceParameters(perception, weightsType));
  if (weights)
    getParameters()->getWeights() = weights->cloneToNewType(context, weightsType);
}

/*
** OnlineLearner
*/
#include "OnlineLearner/StochasticGradientDescentOnlineLearner.h"
#include "OnlineLearner/BatchGradientDescentOnlineLearner.h"

namespace lbcpp
{
  extern GradientDescentOnlineLearnerPtr stochasticGradientDescentOnlineLearner(
    IterationFunctionPtr learningRate, bool normalizeLearningRate,
    LearnerUpdateFrequency regularizerUpdateFrequency,
    ScalarObjectFunctionPtr regularizer);

  extern GradientDescentOnlineLearnerPtr batchGradientDescentOnlineLearner(
    LearnerUpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate,
    LearnerUpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer);
  
};

InferenceOnlineLearnerPtr lbcpp::gradientDescentOnlineLearner(
        LearnerUpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate,
        LearnerUpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
{
  jassert(learningUpdateFrequency != never);
  InferenceOnlineLearnerPtr res;

  size_t miniBatchSize = 0;
  if (learningUpdateFrequency >= perStepMiniBatch)
  {
    miniBatchSize = learningUpdateFrequency - perStepMiniBatch;
    if (miniBatchSize <= 1)
      learningUpdateFrequency = perStep;
  }

  // base learner
  if (learningUpdateFrequency == perStep)
    return stochasticGradientDescentOnlineLearner(learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  //else if (learningUpdateFrequency >= perStepMiniBatch && miniBatchSize < 100)
  //  res = miniBatchGradientDescentOnlineLearner(miniBatchSize, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else
    return batchGradientDescentOnlineLearner(learningUpdateFrequency,
                                              learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
}


/*
** NumericalSupervisedInference
*/
namespace lbcpp { extern ClassPtr stoppingCriterionOnlineLearnerClass; };

void NumericalSupervisedInference::setStochasticLearner(const InferenceOnlineLearnerPtr& onlineLearner, bool precomputePerceptions, bool randomizeExamples)
{
  jassert(onlineLearner);
  InferencePtr batchLearner = stochasticInferenceLearner(randomizeExamples);//, evaluator, validationDataPercentage);
  if (precomputePerceptions)
    batchLearner = precomputePerceptionsNumericalInferenceLearner(batchLearner);
  decorated->setBatchLearner(batchLearner);
  decorated->addOnlineLearner(onlineLearner);
}

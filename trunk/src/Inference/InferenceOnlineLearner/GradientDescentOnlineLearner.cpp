/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCa...cpp| Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GradientDescentOnlineLearner.h"
#include <lbcpp/Function/ScalarFunction.h>
#include <lbcpp/Perception/PerceptionMaths.h>
using namespace lbcpp;

GradientDescentOnlineLearner::GradientDescentOnlineLearner(
                      UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                      UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
    : epoch(0),
      learningUpdateFrequency(learningUpdateFrequency), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer),
      lossValue(T("Loss")), lastApplyRegularizerEpoch(0)
{
}

void GradientDescentOnlineLearner::stepFinishedCallback(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  updateNumberOfActiveFeatures(getPerception(inference), input);
}
  
void GradientDescentOnlineLearner::episodeFinishedCallback(InferencePtr inference)
{
  if (regularizerUpdateFrequency == perEpisode)
    applyRegularizer(inference);
}

void GradientDescentOnlineLearner::passFinishedCallback(InferencePtr inference)
{
  if (regularizerUpdateFrequency == perPass)
    applyRegularizer(inference);
  
  ObjectPtr parameters = getNumericalInference(inference)->getParametersCopy();
  size_t l0norm = lbcpp::l0norm(parameters);
  double l2norm = lbcpp::l2norm(parameters);
  MessageCallback::info(inference->getName() + T(" Epoch ") + String((int)epoch) + T(", ") + String((int)l0norm) + T(" parameters, L2 = ") + String(l2norm, 3));
  if (lossValue.getCount())
  {
    double mean = lossValue.getMean();
    MessageCallback::info(lossValue.toString() + T("\n"));
    lossValue.clear();
    lossValue.push(mean); // hack: we push the previous mean loss as a first sample, in order to have a correct estimate before the first example arrives
  }
}

void GradientDescentOnlineLearner::updateParameters(InferencePtr inference, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, ObjectPtr* target)
{
  double exampleLossValue;
  getNumericalInference(inference)->computeAndAddGradient(- weight * computeLearningRate(), input, supervision, prediction, exampleLossValue, target);

  ScopedLock _(lossValueLock);
  lossValue.push(exampleLossValue);
}

bool GradientDescentOnlineLearner::shouldApplyRegularizerAfterStep(size_t epoch) const
{
  if (regularizerUpdateFrequency == perStep)
    return true;
  if (regularizerUpdateFrequency >= perStepMiniBatch)
  {
    int miniBatchSize = regularizerUpdateFrequency - perStepMiniBatch;
    return miniBatchSize <= 1 || (epoch % miniBatchSize) == 0;
  }
  return false;
}

void GradientDescentOnlineLearner::checkRegularizerAfterStep(InferencePtr inference)
{
  if (shouldApplyRegularizerAfterStep(epoch))
    applyRegularizer(inference);
}

void GradientDescentOnlineLearner::gradientDescentStep(InferencePtr inf, ObjectPtr gradient, double weight)
{
  NumericalInferencePtr inference = getNumericalInference(inf);
  inference->addWeightedToParameters(gradient, -computeLearningRate() * weight);
}

void GradientDescentOnlineLearner::applyExample(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  ++epoch;
  updateParameters(inference, 1.0, input, supervision, prediction);
  checkRegularizerAfterStep(inference);
}

void GradientDescentOnlineLearner::applyRegularizer(InferencePtr inference)
{
  if (regularizer)
  {
    double weight = (double)(epoch - lastApplyRegularizerEpoch);
    getNumericalInference(inference)->applyRegularizerToParameters(regularizer, -computeLearningRate() * weight);
    lastApplyRegularizerEpoch = epoch;
  }
}

double GradientDescentOnlineLearner::computeLearningRate() const
{
  double res = 1.0;
  if (learningRate)
    res *= learningRate->compute(epoch);
  if (normalizeLearningRate && numberOfActiveFeatures.getMean())
    res /= numberOfActiveFeatures.getMean();
  return res;
}

void GradientDescentOnlineLearner::updateNumberOfActiveFeatures(PerceptionPtr perception, const Variable& input)
{
  size_t numSamples = (size_t)numberOfActiveFeatures.getCount();
  // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
  if (numSamples < 10 ||                        // every time until having 10 samples
    (numSamples < 100 && (epoch % 10 == 0)) ||  // every 10 epochs until having 100 samples
    (epoch % 100 == 0))                         // every 100 epochs after that
    numberOfActiveFeatures.push(lbcpp::l1norm(perception, input));
}

void GradientDescentOnlineLearner::clone(ObjectPtr target) const
{
  GradientDescentOnlineLearnerPtr res = (GradientDescentOnlineLearnerPtr)target;
  res->numberOfActiveFeatures = numberOfActiveFeatures;
  res->epoch = epoch;
  res->learningRate = learningRate;
  res->learningUpdateFrequency = learningUpdateFrequency;
  res->normalizeLearningRate = normalizeLearningRate;
  res->regularizerUpdateFrequency = regularizerUpdateFrequency;
  res->regularizer = regularizer;
  res->lossValue = lossValue;
  res->lastApplyRegularizerEpoch = lastApplyRegularizerEpoch;
  Object::clone(res);
}

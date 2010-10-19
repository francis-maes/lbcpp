/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCa...cpp| Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GradientDescentOnlineLearner.h"
#include <lbcpp/Function/ScalarFunction.h>
using namespace lbcpp;

GradientDescentOnlineLearner::GradientDescentOnlineLearner(
                      UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                      UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer)
    : numberOfActiveFeatures(T("NumActiveFeatures"), 10), epoch(0),
      learningUpdateFrequency(learningUpdateFrequency), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer),
      lossValue(T("Loss")), lastApplyRegularizerEpoch(0)
{
}

void GradientDescentOnlineLearner::startLearningCallback()
{
  numberOfActiveFeatures.clear();
  epoch = 0;
  ScopedLock _(lossValueLock);
  lossValue.clear();
  lastApplyRegularizerEpoch = 0;
}

void GradientDescentOnlineLearner::stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  updateNumberOfActiveFeatures(getPerception(inference), input);
}
  
void GradientDescentOnlineLearner::episodeFinishedCallback(const InferencePtr& inference)
{
  if (regularizerUpdateFrequency == perEpisode)
    applyRegularizer(inference);
}

void GradientDescentOnlineLearner::passFinishedCallback(const InferencePtr& inference)
{
  if (regularizerUpdateFrequency == perPass)
    applyRegularizer(inference);
  
  ObjectPtr parameters = getNumericalInference(inference)->getParameters();
  size_t l0norm = lbcpp::l0norm(parameters);
  double l2norm = lbcpp::l2norm(parameters);
  MessageCallback::info(inference->getName() + T(" Epoch ") + String((int)epoch) + T(", ") + String((int)l0norm) + T(" parameters, L2 = ") + String(l2norm, 3));
  if (lossValue.getCount())
  {
    double mean = lossValue.getMean();
    MessageCallback::info(lossValue.toString() + T(" meanFeaturesL1 = ") + String(numberOfActiveFeatures.getMean()) + T("\n"));
    lossValue.clear();
    lossValue.push(mean); // hack: we push the previous mean loss as a first sample, in order to have a correct estimate before the first example arrives
  }
}

void GradientDescentOnlineLearner::updateParameters(const InferencePtr& inference, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, ObjectPtr* target)
{
  double exampleLossValue;
  const NumericalInferencePtr& numericalInference = getNumericalInference(inference);
  Variable pred = prediction.exists() ? prediction : numericalInference->predict(input);
  numericalInference->computeAndAddGradient(- weight * computeLearningRate(), input, supervision, pred, exampleLossValue, target);

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

void GradientDescentOnlineLearner::checkRegularizerAfterStep(const InferencePtr& inference)
{
  if (shouldApplyRegularizerAfterStep(epoch))
    applyRegularizer(inference);
}

void GradientDescentOnlineLearner::gradientDescentStep(const InferencePtr& inf, const ObjectPtr& gradient, double weight)
{
  NumericalInferencePtr inference = getNumericalInference(inf);
  inference->addWeightedToParameters(gradient, -computeLearningRate() * weight);
}

void GradientDescentOnlineLearner::applyExample(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  ++epoch;
  updateParameters(inference, 1.0, input, supervision, prediction);
  checkRegularizerAfterStep(inference);
}

void GradientDescentOnlineLearner::applyRegularizer(const InferencePtr& inference)
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

void GradientDescentOnlineLearner::updateNumberOfActiveFeatures(const PerceptionPtr& perception, const Variable& input)
{
  if (normalizeLearningRate)
  {
    // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
    if (!numberOfActiveFeatures.isMemoryFull() || (epoch % 20 == 0))
    {
      double norm = lbcpp::l1norm(perception, input);
      if (norm)
        numberOfActiveFeatures.push(norm);
    }
  }
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

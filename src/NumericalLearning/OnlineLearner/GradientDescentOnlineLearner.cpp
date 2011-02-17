/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCa...cpp| Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GradientDescentOnlineLearner.h"
#include <lbcpp/Function/ScalarFunction.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/ParallelInference.h>
using namespace lbcpp;

OldGradientDescentOnlineLearner::OldGradientDescentOnlineLearner(
                      LearnerUpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                      LearnerUpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : numberOfActiveFeatures(T("NumActiveFeatures"), 10), epoch(0),
      learningUpdateFrequency(learningUpdateFrequency), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer),
      lossValue(T("Loss")), lastEmpiricalRisk(0.0), lastApplyRegularizerEpoch(0)
{
}

void OldGradientDescentOnlineLearner::startLearningCallback(ExecutionContext& context)
{
  numberOfActiveFeatures.clear();
  epoch = 0;
  ScopedLock _(lossValueLock);
  lossValue.clear();
  lastApplyRegularizerEpoch = 0;
  lastEmpiricalRisk = 0.0;
  InferenceOnlineLearner::startLearningCallback(context);
}

void OldGradientDescentOnlineLearner::stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  checkRegularizerAfterStep(context, inference);

  const PerceptionPtr& perception = getPerception(inference);
  if (input.isObject() && input.dynamicCast<Container>())
  {
    // composite inputs (e.g. ranking)
    const ContainerPtr& inputs = input.getObjectAndCast<Container>(context);
    size_t n = inputs->getNumElements();
    for (size_t i = 0; i < n; ++i)
      updateNumberOfActiveFeatures(context, perception, inputs->getElement(i));
  }
  else
  {
    // simple input
    updateNumberOfActiveFeatures(context, perception, input);
  }
  InferenceOnlineLearner::stepFinishedCallback(context, inference, input, supervision, prediction);
}
  
void OldGradientDescentOnlineLearner::episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference)
{
  if (regularizerUpdateFrequency == perEpisode)
    applyRegularizer(context, inference);
  InferenceOnlineLearner::episodeFinishedCallback(context, inference);
}

void OldGradientDescentOnlineLearner::passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
{
  if (regularizerUpdateFrequency == perPass)
    applyRegularizer(context, inference);
  
  ObjectPtr parameters = getNumericalInference(inference)->getWeightsCopy(context);
  size_t l0norm = lbcpp::l0norm(context, parameters);
  double l2norm = lbcpp::l2norm(context, parameters);
  context.resultCallback(T("Epoch"), (int)epoch);
  context.resultCallback(T("NumParameters"), (int)l0norm);
  context.resultCallback(T("Parameters L2"), l2norm);
  //context.informationCallback(inference->getName() + T(" Epoch ") + String((int)epoch) + T(", ") + String((int)l0norm) + T(" parameters, L2 = ") + String(l2norm, 3));
  //Variable(parameters).printRecursively(std::cout, -1, false, false);
  if (lossValue.getCount())
  {
    double mean = lossValue.getMean();
    context.resultCallback(T("Mean Active Features"), numberOfActiveFeatures.getMean());
    //context.informationCallback(lossValue.toString() + T(" meanFeaturesL1 = ") + String(numberOfActiveFeatures.getMean()) + T("\n"));
    lastEmpiricalRisk = mean;
    lossValue.clear();
    //lossValue.push(mean); // hack: we push the previous mean loss as a first sample, in order to have a correct estimate before the first example arrives
  }
  InferenceOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);
}

void OldGradientDescentOnlineLearner::updateParameters(ExecutionContext& context, const InferencePtr& inference, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, ObjectPtr* target)
{
  double exampleLossValue;
  const NumericalInferencePtr& numericalInference = getNumericalInference(inference);
  Variable pred;
  if (prediction.exists())
    pred = prediction;
  else
    pred = inference->computeFunction(context, input);
  numericalInference->computeAndAddGradient(context, - weight * computeLearningRate(), input, supervision, pred, exampleLossValue, target);

  ScopedLock _(lossValueLock);
  lossValue.push(exampleLossValue);
}

bool OldGradientDescentOnlineLearner::shouldApplyRegularizerAfterStep(size_t epoch) const
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

void OldGradientDescentOnlineLearner::checkRegularizerAfterStep(ExecutionContext& context, const InferencePtr& inference)
{
  if (shouldApplyRegularizerAfterStep(epoch))
    applyRegularizer(context, inference);
}

void OldGradientDescentOnlineLearner::gradientDescentStep(ExecutionContext& context, const InferencePtr& inf, const ObjectPtr& gradient, double weight)
{
  NumericalInferencePtr inference = getNumericalInference(inf);
  inference->addWeightedToParameters(context, gradient, -computeLearningRate() * weight);
}

void OldGradientDescentOnlineLearner::applyRegularizer(ExecutionContext& context, const InferencePtr& inference)
{
  if (regularizer)
  {
    double weight = (double)(epoch - lastApplyRegularizerEpoch);
    getNumericalInference(inference)->applyRegularizerToParameters(context, regularizer, -computeLearningRate() * weight);
    lastApplyRegularizerEpoch = epoch;
  }
}

double OldGradientDescentOnlineLearner::computeLearningRate() const
{
  double res = 1.0;
  if (learningRate)
    res *= learningRate->computeIterationFunction(epoch);
  if (normalizeLearningRate && numberOfActiveFeatures.getMean())
    res /= numberOfActiveFeatures.getMean();
  return res;
}

void OldGradientDescentOnlineLearner::updateNumberOfActiveFeatures(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input)
{
  if (normalizeLearningRate && input.exists())
  {
    // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
    if (!numberOfActiveFeatures.isMemoryFull() || (epoch % 20 == 0))
    {
      double norm = input.getType() == perception->getOutputType() ? lbcpp::l1norm(context, input.getObject()) : lbcpp::l1norm(context, perception, input);
      if (norm)
        numberOfActiveFeatures.push(norm);
    }
  }
}

void OldGradientDescentOnlineLearner::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  InferenceOnlineLearner::clone(context, target);

  const OldGradientDescentOnlineLearnerPtr& res = target.staticCast<OldGradientDescentOnlineLearner>();
  res->numberOfActiveFeatures = numberOfActiveFeatures;
  res->epoch = epoch;
  res->learningRate = learningRate;
  res->learningUpdateFrequency = learningUpdateFrequency;
  res->normalizeLearningRate = normalizeLearningRate;
  res->regularizerUpdateFrequency = regularizerUpdateFrequency;
  res->regularizer = regularizer;
  res->lossValue = lossValue;
  res->lastApplyRegularizerEpoch = lastApplyRegularizerEpoch;
}

NumericalInferencePtr OldGradientDescentOnlineLearner::getNumericalInference(const InferencePtr& inference) const
{
  if (inference.isInstanceOf<NumericalInference>())
    return inference.staticCast<NumericalInference>();
  else if (inference.isInstanceOf<StaticDecoratorInference>())
    return getNumericalInference(inference.staticCast<StaticDecoratorInference>()->getSubInference());
  else if (inference.isInstanceOf<SharedParallelInference>())
    return getNumericalInference(inference.staticCast<SharedParallelInference>()->getSubInference());
  else
  {
    jassert(false);
    return *(const NumericalInferencePtr* )0;
  }
}

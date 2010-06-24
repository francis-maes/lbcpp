/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCa...cpp| Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GradientDescentOnlineLearner.h"
#include <lbcpp/FeatureGenerator/DenseVector.h>
using namespace lbcpp;

GradientDescentOnlineLearner::GradientDescentOnlineLearner(
                      UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                      UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : epoch(0),
      learningUpdateFrequency(learningUpdateFrequency), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer),
      lossValue(T("Loss")), lastApplyRegularizerEpoch(0)
{
}

void GradientDescentOnlineLearner::stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
  jassert(features);
  updateNumberOfActiveFeatures(features);
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
  
  DenseVectorPtr parameters = getParameters(inference);
  size_t l0norm = parameters ? parameters->l0norm() : 0;
  double l2norm = parameters ? parameters->l2norm() : 0.0;
  std::cout << inference->getName() << " Epoch " << epoch << ", " << l0norm << " parameters, L2 = " << String(l2norm, 3) << std::endl;
  if (lossValue.getCount())
  {
    double mean = lossValue.getMean();
    std::cout << lossValue.toString() << std::endl;
    lossValue.clear();
    lossValue.push(mean); // hack: we push the previous mean loss as a first sample, in order to have a correct estimate before the first example arrives
  }
  std::cout << std::endl;
}

FeatureGeneratorPtr GradientDescentOnlineLearner::getExampleGradient(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  double exampleLossValue;
  FeatureGeneratorPtr gradient = getParameterizedInference(inference)->getExampleGradient(input, supervision, predictedOutput, exampleLossValue);
  lossValue.push(exampleLossValue);
  return gradient;
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

void GradientDescentOnlineLearner::gradientDescentStep(InferencePtr inf, FeatureGeneratorPtr gradient, double weight)
{
  ParameterizedInferencePtr inference = getParameterizedInference(inf);
  DenseVectorPtr parameters = inference->getParameters();
  if (!parameters)
  {
    parameters = new DenseVector(gradient->getDictionary());
    inference->setParameters(parameters);
  }
  else
    parameters->ensureDictionary(gradient->getDictionary());

  gradient->addWeightedTo(parameters, -computeLearningRate() * weight);
  //std::cout << "gradient: " << gradient->l2norm() << " learning rate: " << computeLearningRate() * weight << " parameters: " << getParameters()->l2norm() << std::endl;
  inference->validateParametersChange();
}

void GradientDescentOnlineLearner::applyExample(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  //std::cout << "e" << std::flush;
  ++epoch;
  gradientDescentStep(inference, getExampleGradient(inference, input, supervision, predictedOutput));
  checkRegularizerAfterStep(inference);
}

void GradientDescentOnlineLearner::applyRegularizer(InferencePtr inference)
{
  if (regularizer)
  {
    DenseVectorPtr parameters = getParameters(inference);
    if (parameters)
      //std::cout << "R" << std::flush;
      gradientDescentStep(inference, regularizer->computeGradient(getParameters(inference)), (double)(epoch - lastApplyRegularizerEpoch));

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

void GradientDescentOnlineLearner::updateNumberOfActiveFeatures(FeatureGeneratorPtr features)
{
  size_t numSamples = (size_t)numberOfActiveFeatures.getCount();
  // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
  if (numSamples < 10 ||                        // every time until having 10 samples
    (numSamples < 100 && (epoch % 10 == 0)) ||  // every 10 epochs until having 100 samples
    (epoch % 100 == 0))                         // every 100 epochs after that
  numberOfActiveFeatures.push((double)(features->l1norm()));
}

ObjectPtr GradientDescentOnlineLearner::clone() const
{
  GradientDescentOnlineLearnerPtr res = createAndCast<GradientDescentOnlineLearner>(getClassName());
  res->numberOfActiveFeatures = numberOfActiveFeatures;
  res->epoch = epoch;
  res->learningRate = learningRate;
  res->learningUpdateFrequency = learningUpdateFrequency;
  res->normalizeLearningRate = normalizeLearningRate;
  res->regularizerUpdateFrequency = regularizerUpdateFrequency;
  res->regularizer = regularizer;
  res->lossValue = lossValue;
  res->lastApplyRegularizerEpoch = lastApplyRegularizerEpoch;
  return res;
}

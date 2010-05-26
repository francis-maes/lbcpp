/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCa...cpp| Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GradientDescentLearningCallback.h"
using namespace lbcpp;

/*
** IterativeLearningInferenceCallback
*/
IterativeLearningInferenceCallback::IterativeLearningInferenceCallback(InferencePtr inference,
                                                                       IterationFunctionPtr learningRate, bool normalizeLearningRate)
  : LearningInferenceCallback(inference), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0)
{
}

double IterativeLearningInferenceCallback::computeLearningRate() const
{
  double res = 1.0;
  if (learningRate)
    res *= learningRate->compute(epoch);
  if (normalizeLearningRate && numberOfActiveFeatures.getMean())
    res /= numberOfActiveFeatures.getMean();
  return res;
}

void IterativeLearningInferenceCallback::updateNumberOfActiveFeatures(FeatureGeneratorPtr features)
{
  size_t numSamples = (size_t)numberOfActiveFeatures.getCount();
  // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
  if (numSamples < 10 ||                        // every time until having 10 samples
    (numSamples < 100 && (epoch % 10 == 0)) ||  // every 10 epochs until having 100 samples
    (epoch % 100 == 0))                         // every 100 epochs after that
  numberOfActiveFeatures.push((double)(features->l1norm()));
}

/*
** GradientDescentLearningCallback
*/
GradientDescentLearningCallback::GradientDescentLearningCallback(ParameterizedInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : IterativeLearningInferenceCallback(inference, learningRate, normalizeLearningRate),
      learningUpdateFrequency(learningUpdateFrequency),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer),
      lossValue(T("Loss")), lastApplyRegularizerEpoch(0)
{
}

void GradientDescentLearningCallback::stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
  jassert(features);
  updateNumberOfActiveFeatures(features);
}
  
void GradientDescentLearningCallback::episodeFinishedCallback()
{
  if (regularizerUpdateFrequency == perEpisode)
    applyRegularizer();
}

void GradientDescentLearningCallback::passFinishedCallback()
{
  if (regularizerUpdateFrequency == perPass)
    applyRegularizer();
  
  std::cout << inference->getName() << " Epoch " << epoch << ", " << getParameters()->l0norm() << " parameters, L2 = " << String(getParameters()->l2norm(), 3) << std::endl;
  if (lossValue.getCount())
  {
    double mean = lossValue.getMean();
    std::cout << lossValue.toString() << std::endl;
    lossValue.clear();
    lossValue.push(mean); // hack: we push the previous mean loss as a first sample, in order to have a correct estimate before the first example arrives
  }
  std::cout << std::endl;
}

FeatureGeneratorPtr GradientDescentLearningCallback::getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  double exampleLossValue;
  FeatureGeneratorPtr gradient = getInference()->getExampleGradient(input, supervision, predictedOutput, exampleLossValue);
  lossValue.push(exampleLossValue);
  return gradient;
}

bool GradientDescentLearningCallback::shouldApplyRegularizerAfterStep(size_t epoch) const
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

void GradientDescentLearningCallback::checkRegularizerAfterStep()
{
  if (shouldApplyRegularizerAfterStep(epoch))
    applyRegularizer();
}

void GradientDescentLearningCallback::gradientDescentStep(FeatureGeneratorPtr gradient, double weight)
{
  gradient->addWeightedTo(getParameters(), -computeLearningRate() * weight);
  //std::cout << "gradient: " << gradient->l2norm() << " learning rate: " << computeLearningRate() * weight << " parameters: " << getParameters()->l2norm() << std::endl;
  getInference()->validateParametersChange();
}

void GradientDescentLearningCallback::applyExample(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  //std::cout << "e" << std::flush;
  ++epoch;
  gradientDescentStep(getExampleGradient(input, supervision, predictedOutput));
  checkRegularizerAfterStep();
}

void GradientDescentLearningCallback::applyRegularizer()
{
  if (regularizer)
  {
    //std::cout << "R" << std::flush;
    gradientDescentStep(regularizer->computeGradient(getParameters()), (double)(epoch - lastApplyRegularizerEpoch));
    lastApplyRegularizerEpoch = epoch;
  }
}

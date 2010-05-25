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
GradientDescentLearningCallback::GradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency randomizationFrequency,
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : IterativeLearningInferenceCallback(inference, learningRate, normalizeLearningRate),
      learningUpdateFrequency(learningUpdateFrequency), randomizationFrequency(randomizationFrequency),
      regularizerUpdateFrequency(regularizerUpdateFrequency), regularizer(regularizer)
{
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

void GradientDescentLearningCallback::applyExample(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
{
  ++epoch;
  FeatureGeneratorPtr gradient = getExampleGradient(input, supervision, predictedOutput);
  gradient->addWeightedTo(getParameters(), - computeLearningRate());
  if (shouldApplyRegularizerAfterStep(epoch))
    applyRegularizer();
  parametersChanged();
}

void GradientDescentLearningCallback::applyRegularizer()
{
  if (regularizer)
  {
    DenseVectorPtr parameters = getParameters();
    regularizer->computeGradient(parameters)->addWeightedTo(parameters, - computeLearningRate());
    parametersChanged();  
  }
}

void GradientDescentLearningCallback::parametersChanged()
  {getInference()->validateParametersChange();}

void GradientDescentLearningCallback::finishInferencesCallback()
{
  IterativeLearningInferenceCallback::finishInferencesCallback();
  std::cout << inference->getName() << " Epoch " << epoch << ", " << getParameters()->l0norm() << " parameters, L2 = " << String(getParameters()->l2norm(), 3) << std::endl;
  if (lossValue.getCount())
  {
    std::cout << lossValue.toString() << std::endl;
    lossValue.clear();
  }
  std::cout << std::endl;
}

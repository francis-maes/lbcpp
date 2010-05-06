/*-----------------------------------------.---------------------------------.
| Filename: StochasticScalarLinearInfer...h| Stochastic gradient descent     |
| Author  : Francis Maes                   |  learner for linear Inference   |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_STOCHASTIC_SCALAR_LINEAR_INFERENCE_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_STOCHASTIC_SCALAR_LINEAR_INFERENCE_LEARNING_CALLBACK_H_

# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/Inference/InferenceBaseClasses.h>

namespace lbcpp
{

class StochasticScalarLinearInferenceLearningCallback : public ScalarInferenceLearningCallback
{
public:
  StochasticScalarLinearInferenceLearningCallback(LearnableAtomicInferencePtr step, IterationFunctionPtr learningRate, ScalarVectorFunctionPtr regularizer = ScalarVectorFunctionPtr(), bool normalizeLearningRate = true)
    : ScalarInferenceLearningCallback(step), learningRate(learningRate), regularizer(regularizer), normalizeLearningRate(normalizeLearningRate) {}

  virtual size_t postInferenceCallback(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr exampleLoss)
  {
    updateInputSize(features);

    //std::cout << "Loss: " << exampleLoss->toString() << " Derivative: " << exampleLoss->computeDerivative(prediction) << " PRediction = " << prediction << std::endl;

    const size_t regularizerFrequency = 100;

    DenseVectorPtr parameters = step->getParameters();

    double alpha = computeAlpha();
    features->addWeightedTo(parameters, - alpha * exampleLoss->computeDerivative(prediction));
    if (regularizer && (epoch % regularizerFrequency) == 0)
    {
      FeatureGeneratorPtr regularizerGradient = regularizer->computeGradient(parameters);
      regularizerGradient->addWeightedTo(parameters, - alpha * (double)regularizerFrequency);
    }
    step->validateParametersChange();
    return 1;
  }

  virtual void finishInferencesCallback()
  {
    std::cout << "Epoch " << epoch << ", " << step->getParameters()->l0norm() << " parameters, L2 = " << step->getParameters()->l2norm() << std::endl;
  }

protected:
  IterationFunctionPtr learningRate;
  ScalarVectorFunctionPtr regularizer;
  bool normalizeLearningRate;
  ScalarVariableMean inputSize;

  double computeAlpha() const
  {
    double res = 1.0;
    if (learningRate)
      res *= learningRate->compute(epoch);
    if (normalizeLearningRate && inputSize.getMean())
      res /= inputSize.getMean();
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_STOCHASTIC_SCALAR_LINEAR_INFERENCE_LEARNING_CALLBACK_H_

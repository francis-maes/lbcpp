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

class ScalarInferenceLearningCallback : public InferenceCallback
{
public:
  ScalarInferenceLearningCallback(LearnableAtomicInferencePtr step)
    : step(step), epoch(1) {}

  // epoch starts at 1
  virtual bool learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr loss) = 0;

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (step == stack->getCurrentInference() && supervision && output)
    {
      FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      ScalarPtr prediction = output.dynamicCast<Scalar>();
      jassert(features && loss && prediction);
      if (learningEpoch(epoch, features, prediction->getValue(), loss))
        ++epoch;
    }
  }

protected:
  LearnableAtomicInferencePtr step;
  size_t epoch;
};

class StochasticScalarLinearInferenceLearningCallback : public ScalarInferenceLearningCallback
{
public:
  StochasticScalarLinearInferenceLearningCallback(LearnableAtomicInferencePtr step, IterationFunctionPtr learningRate, ScalarFunctionPtr regularizer = ScalarFunctionPtr(), bool normalizeLearningRate = true)
    : ScalarInferenceLearningCallback(step), learningRate(learningRate), regularizer(regularizer), normalizeLearningRate(normalizeLearningRate) {}

  virtual bool learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr exampleLoss)
  {
    // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
    if (inputSize.getCount() < 10 ||                         // every time until having 10 samples
        (inputSize.getCount() < 100 && (epoch % 10 == 0)) || // every 10 epochs until having 100 samples
        (epoch % 100 == 0))                                  // every 100 epochs after that
    {
      inputSize.push((double)(features->l1norm()));
      //std::cout << "Alpha: " << weight * computeAlpha() << " inputSize: " << inputSize.toString() << std::endl;
    }

    if (exampleLoss->compute(1.0) > exampleLoss->compute(-1.0) && RandomGenerator::getInstance().sampleBool(0.95))
      return true; // reject 95% of negative examples

    //std::cout << "Loss: " << exampleLoss->toString() << " Derivative: " << exampleLoss->computeDerivative(prediction) << " PRediction = " << prediction << std::endl;
    double k = computeAlpha() * exampleLoss->computeDerivative(prediction);

    features->addWeightedTo(step->getParameters(), - k);
    step->validateParametersChange();

    //if (epoch % 1000 == 0)
    //  std::cout << step->getParameters()->toString() << std::endl;

/*    size_t regularizerFrequency = 100;
    if ((epoch % regularizerFrequency) == 0)
    {
      // todo: apply regularizer
    }*/
    return true;
  }
  virtual void finishInferencesCallback()
  {
    std::cout << "Epoch " << epoch << ", " << step->getParameters()->l0norm() << " parameters, L2 = " << step->getParameters()->l2norm() << std::endl;
  }

protected:
  IterationFunctionPtr learningRate;
  ScalarFunctionPtr regularizer;
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

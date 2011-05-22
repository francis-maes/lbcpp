/*-----------------------------------------.---------------------------------.
| Filename: ConditionalGaussianSampler.h   | Conditional Gaussian Sampler    |
| Author  : Francis Maes                   |                                 |
| Started : 22/05/2011 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_CONTINUOUS_CONDITIONAL_GAUSSIAN_H_
# define LBCPP_SAMPLER_CONTINUOUS_CONDITIONAL_GAUSSIAN_H_

# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

// - log conditional density of x, given mean and stddev_parameter
//   with stddev = log(1 + exp(-stddev_parameter))
class ConditionalGaussianLossFunction : public ScalarVectorFunction
{
public:
  ConditionalGaussianLossFunction()
    : logBinomialLoss(logBinomialDiscriminativeLossFunction()) {}

  virtual bool isDerivable() const
    {return true;}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : (TypePtr)denseDoubleVectorClass();}

  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input,
      const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    // get mean
    double mean = input ? input->getValue(0) : 0.0;

    // get stddev parameter and compute stddev
    double stdDev, stdDevDerivative;
    Variable vTrue(true);
    logBinomialLoss->computeScalarFunction(input ? input->getValue(1) : 0.0, &vTrue, &stdDev, &stdDevDerivative);
    jassert(stdDev > 0);

    double invStdDev = 1.0 / stdDev;
    double x = otherInputs->getDouble();
    double srPi = std::sqrt(2 * M_PI);
 
    //std::cerr << "mean = " << mean << " stddev = " << stdDev << " x = " << x << std::endl;

    if (output)
    {
      double negativeLogDensity = log(srPi * stdDev) + 0.5 * pow2((x - mean) * invStdDev);
      jassert(isNumberValid(negativeLogDensity));
      //std::cerr << "logP = " << logP << std::endl;
      *output += negativeLogDensity;
    }

    if (gradientTarget)
    {
      double derivativeWRTMean = -(x - mean) * pow2(invStdDev);
      double derivativeWRTStddev = (invStdDev - pow2(x - mean) * pow3(invStdDev)) * stdDevDerivative;
      //std::cerr << "derivativeWRTMean = " << derivativeWRTMean << " derivativeWRTStddev = " << derivativeWRTStddev << std::endl;

      // limit l2-norm to be 1 at maximum
      /*double norm = derivativeWRTMean * derivativeWRTMean + derivativeWRTStddev * derivativeWRTStddev;
      if (norm > 1)
      {
        std::cerr << "Norm is too high: " << sqrt(norm) << std::endl;
        gradientWeight /= sqrt(norm);
      }*/

      (*gradientTarget)->incrementValue(0, derivativeWRTMean * gradientWeight);
      (*gradientTarget)->incrementValue(1, derivativeWRTStddev * gradientWeight);
    }
  }

protected:
  static inline double pow2(double x)
    {return x * x;}
  static inline double pow3(double x)
    {return x * x * x;}

  DiscriminativeLossFunctionPtr logBinomialLoss;
};

extern ScalarVectorFunctionPtr conditionalGaussianLossFunction();
extern ClassPtr gaussianSamplerClass;

// f(x) = N(<theta_1,x>, log(1 + exp(-<theta_2,x>)))
class ConditionalGaussianSampler : public ScalarContinuousSampler
{
public:
  ConditionalGaussianSampler()
  {
    ExecutionContext& context = defaultExecutionContext();

    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(0.01));
    parameters->setLossFunction(conditionalGaussianLossFunction());
    EnumerationPtr parametersEnumeration = variablesEnumerationEnumeration(gaussianSamplerClass); // an enum containing "mean" and "stddev"

    multiLinearFunction = multiLinearLearnableFunction(parametersEnumeration);
    multiLinearFunction->setOnlineLearner(parameters->createOnlineLearner(context));
    multiLinearFunction->setBatchLearner(parameters->createBatchLearner(context));
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr meanAndStddev = multiLinearFunction->compute(context, inputs[0], Variable::missingValue(doubleType)).getObjectAndCast<DenseDoubleVector>();
    if (!meanAndStddev)
      return Variable::missingValue(doubleType);

    double mean = meanAndStddev->getValue(0);
    double stddev = log(1 + exp(-meanAndStddev->getValue(1)));
    return random->sampleDoubleFromGaussian(mean, stddev);
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& validationWeights)
  {
    ContainerPtr trainingData = Container::makePairsContainer(trainingInputs, trainingSamples);
    ContainerPtr validationData;
    if (validationSamples)
      validationData = Container::makePairsContainer(validationInputs, validationSamples);
    multiLinearFunction->train(context, trainingData, validationData, T("Training conditional gaussian"));
  }

protected:
  friend class ConditionalGaussianSamplerClass;

  NumericalLearnableFunctionPtr multiLinearFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_CONTINUOUS_CONDITIONAL_GAUSSIAN_H_

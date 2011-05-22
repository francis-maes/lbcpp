/*-----------------------------------------.---------------------------------.
| Filename: RosettaSandBox.h               | Rosetta Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2011 19:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SAND_BOX
# define LBCPP_PROTEINS_ROSETTA_SAND_BOX

# include "../Data/Protein.h"
# include "../Data/AminoAcid.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Evaluator/QScoreEvaluator.h"
# include "RosettaUtils.h"
# include "ProteinMover.h"
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyMover.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/ResiduePairSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "Sampler.h"
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Core/Vector.h>
//using namespace std;

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

typedef ReferenceCountedObjectPtr<ConditionalGaussianSampler> ConditionalGaussianSamplerPtr;

class MaximumEntropySampler : public DiscreteSampler
{
public:
  MaximumEntropySampler(TypePtr outputType) : outputType(outputType)
  {
    StochasticGDParametersPtr learnerParameters = new StochasticGDParameters(constantIterationFunction(1.0), maxIterationsStoppingCriterion(100));
    learnerParameters->setLossFunction(logBinomialMultiClassLossFunction());
    learnerParameters->setEvaluateAtEachIteration(false);

    predictor = linearMultiClassClassifier(learnerParameters);
    predictor->setEvaluator(EvaluatorPtr()); // todo: log-likelyhood evaluator
  }

  MaximumEntropySampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr probabilities = predictor->compute(context, inputs[0], Variable::missingValue(outputType)).getObjectAndCast<DenseDoubleVector>();
    if (!probabilities)
      return Variable();
    return Variable(random->sampleWithNormalizedProbabilities(probabilities->getValues()), probabilities->getElementsEnumeration());
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    ContainerPtr trainingData = Container::makePairsContainer(trainingInputs, trainingSamples);
    ContainerPtr validationData;
    if (validationSamples)
      validationData = Container::makePairsContainer(validationInputs, validationSamples);
    predictor->train(context, trainingData, validationData, T("Training maxent"));
  }

protected:
  friend class MaximumEntropySamplerClass;

  TypePtr outputType; // tmp, todo: merge with Functions
  FunctionPtr predictor;
};

class RosettaSandBox : public WorkUnit
{
public:
  RosettaSandBox() : numExamples(100) {}

  void generateConditionalGaussianDataSet(VectorPtr& inputs, VectorPtr& samples)
  {
    size_t n = numExamples;

    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    RandomGeneratorPtr random = new RandomGenerator();
    inputs = vector(inputClass, n);
    samples = new DenseDoubleVector(n, 0.0);

    // in interval x \in [0, 1], mean goes from 0.5 to -0.5 and stddev goes from 0.0 to 1.0
    double meanOrigin = 0.5;
    double meanCoefficient = -1;
    double stddevOrigin = 0.0;
    double stddevCoefficient = 1.0;

    for (size_t i = 0; i < n; ++i)
    {
# if 0
      double x = random->sampleDouble(0, 1);
      double mean = meanOrigin + x * meanCoefficient;
      double stddev = stddevOrigin + x * stddevCoefficient;
# else
      double x = 0;
      double mean = 0;
      double stddev = 0;
      if ((i % 2) == 0)
      {
        x = random->sampleDoubleFromGaussian(20, 1);
        mean = 13;
        stddev = 1;
      }
      else
      {
        x = random->sampleDoubleFromGaussian(50, 2);
        mean = 33;
        stddev = 0.5;
      }
# endif

      double y = random->sampleDoubleFromGaussian(mean, stddev);
      DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
      input->setValue(0, 1.0);
      input->setValue(1, x);
      inputs->setElement(i, input);
      samples->setElement(i, y);
    }
  }

  void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples)
  {
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    inputs = vector(inputClass);
    samples = vector(proteinMoverClass);

    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    for (size_t i = 0; i < numExamples / 2; ++i)
    {
      inputs->append(input);
      samples->append(phiPsiMover(0, 32, -123));

      inputs->append(input);
      samples->append(phiPsiMover(1, 34, -120));

      inputs->append(input);
      samples->append(phiPsiMover(2, 38, -121));

      inputs->append(input);
      samples->append(phiPsiMover(3, 30, -122));
    }

    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    for (size_t i = 0; i < numExamples / 2; ++i)
    {
      inputs->append(input);
      samples->append(shearMover(3, 0.9, 4.5));

      inputs->append(input);
      samples->append(shearMover(4, 0.7, 4.3));

      inputs->append(input);
      samples->append(shearMover(3, 0.8, 3.4));

      // general
      inputs->append(input);
      samples->append(rigidBodyMover(3, 5, 2.8, -3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(3, 5, 2.5, -2.4));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.8, 3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(0, 4, 1.2, 2.4));

      inputs->append(input);
      samples->append(rigidBodyMover(2, 4, 0.3, 3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      inputs->append(input);
      samples->append(rigidBodyMover(0, 3, 1.01, 4));
    }
  }

  void displayDataSet(ExecutionContext& context, const String& name, const ContainerPtr& inputs, const ContainerPtr& samples)
  {
    size_t n = samples->getNumElements();

    TypePtr elementsType = inputs->getElementsType();
    bool isDoubleVector = elementsType->inheritsFrom(denseDoubleVectorClass());
    EnumerationPtr inputsEnumeration;
    if (isDoubleVector)
      inputsEnumeration = DoubleVector::getElementsEnumeration(elementsType);

    context.enterScope(name);
    for (size_t i = 0; i < n; ++i)
    {
      context.enterScope(T("Sample ") + String((int)i + 1));
      
      if (isDoubleVector)
      {
        DenseDoubleVectorPtr input = inputs->getElement(i).getObjectAndCast<DenseDoubleVector>();
        if (inputsEnumeration == falseOrTrueEnumeration)
          context.resultCallback(T("input"), input->getValue(1));
        else
          for (size_t j = 0; j < inputsEnumeration->getNumElements(); ++j)
            context.resultCallback(inputsEnumeration->getElementName(j), input->getValue(j));
      }
      else
        context.resultCallback(T("input"), inputs->getElement(i));

      context.resultCallback(T("sample"), samples->getElement(i));
      context.leaveScope(true);
    }
    context.leaveScope(n);
  }

  virtual Variable run(ExecutionContext& context)
  {
    VectorPtr inputs;
    VectorPtr samples;
    RandomGeneratorPtr random = new RandomGenerator();

    generateConditionalGaussianDataSet(inputs, samples);
    displayDataSet(context, T("Conditional gaussian dataset"), inputs, samples);

    SamplerPtr sampler = new ConditionalGaussianSampler();
    sampler->learn(context, inputs, samples);

    context.enterScope(T("Testing conditional gaussian"));
    for (size_t i = 0; i < 1000; ++i)
    {
      context.enterScope(T("Sample ") + String((int)i + 1));
# if 0
      double x = random->sampleDouble(0.0, 1.0);
#else
      double x = random->sampleDouble(0.0, 50.0);
# endif
      context.resultCallback(T("x"), x);
      DenseDoubleVectorPtr features = new DenseDoubleVector(falseOrTrueEnumeration, doubleType);
      features->setValue(0, 1.0);
      features->setValue(1, x);
      Variable featuresVariable(features);

      double y = sampler->sample(context, random, &featuresVariable).getDouble();
      context.resultCallback(T("y"), y);
      context.leaveScope(true);
    }
    context.leaveScope(true);

#if 0
    generateMoversDataSet(inputs, samples);
    SamplerPtr moverClassSampler = new MaximumEntropySampler(proteinMoverEnumerationEnumeration);
    SamplerPtr MEsampler = proteinMoverSampler(moverClassSampler, 1000);

    MEsampler->learn(context, inputs, samples);
  
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    random = new RandomGenerator();

    context.enterScope(T("Samples 1"));
    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    Variable inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

    context.enterScope(T("Samples 2"));
    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();
#endif // 0
#if 0

    // phipsi
    learning.push_back(phiPsiMover(1, 34, -123));
    learning.push_back(phiPsiMover(0, 30, -122));
    learning.push_back(phiPsiMover(2, 27, -121));
    learning.push_back(phiPsiMover(3, 33, -121));
    // shear
    learning.push_back(shearMover(3, 0.9, 4.5));
    learning.push_back(shearMover(4, 0.7, 4.3));
    learning.push_back(shearMover(3, 0.8, 3.4));
    // general
    learning.push_back(rigidBodyMover(3, 5, 2.8, -3.4));
    learning.push_back(rigidBodyMover(3, 5, 2.5, -2.4));
    learning.push_back(rigidBodyMover(1, 3, 0.8, 3.4));
    learning.push_back(rigidBodyMover(0, 4, 1.2, 2.4));
    learning.push_back(rigidBodyMover(2, 4, 0.3, 3.4));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(0, 3, 1.01, 4));
    // spin
    learning.push_back(rigidBodyMover(0, 3, 0.0, 11.3));
    learning.push_back(rigidBodyMover(1, 3, 0.0, 12.4));
    learning.push_back(rigidBodyMover(3, 5, 0.0, 9.3));
    learning.push_back(rigidBodyMover(2, 5, 0.0, 10.2));
    // trans
    learning.push_back(rigidBodyMover(4, 1, 10.2, 0.0));
    learning.push_back(rigidBodyMover(4, 1, 9.2, 0.0));
    learning.push_back(rigidBodyMover(4, 0, 12.1, 0.0));
    learning.push_back(rigidBodyMover(1, 3, -0.3, 0.0));
    learning.push_back(rigidBodyMover(0, 2, -2.1, 0.0));
    learning.push_back(rigidBodyMover(0, 3, -1.3, 0.0));

    SamplerPtr sampler = new ProteinMoverSampler(5);
    sampler->learn(context, learning);
    context.resultCallback(T("sampler"), sampler);
#endif 


    return Variable();

  }

private:
  friend class RosettaSandBoxClass;

  size_t numExamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

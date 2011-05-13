/*-----------------------------------------.---------------------------------.
| Filename: ParzenContinuousSampler.h      | ParzenContinuousSampler         |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  09:11:49           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PARZEN_CONTINUOUS_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_PARZEN_CONTINUOUS_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class ParzenContinuousSampler;
typedef ReferenceCountedObjectPtr<ParzenContinuousSampler> ParzenContinuousSamplerPtr;

class ParzenContinuousSampler : public ContinuousSampler
{
public:
  ParzenContinuousSampler()
    : mean(0.0), stddev(1.0), precision(0.0001), excursion(6), kernelWidth(0.25),
      learned(false), fixedAbscissa(false)
  {
  }

  ParzenContinuousSampler(double precision, double excursion = 6, double kernelWidth = 0.25,
      double initialMean = 0, double initialStddev = 1)
    : mean(initialMean), stddev(initialStddev), precision(precision), excursion(excursion),
      kernelWidth(kernelWidth), learned(false), fixedAbscissa(false)
  {
  }

  ParzenContinuousSampler(double precision, double minAbscissa, double maxAbscissa,
      double kernelWidth, double initialMean, double initialStddev)
    : mean(initialMean), stddev(initialStddev), precision(precision), excursion(1),
      kernelWidth(kernelWidth), learned(false), fixedAbscissa(true)
  {
    double delta = std::abs((maxAbscissa - minAbscissa) * precision);
    abscissa = createAbscissa(minAbscissa, maxAbscissa, delta);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    if (!learned)
      if (!fixedAbscissa)
        return Variable(random->sampleDoubleFromGaussian(mean, stddev));
      else
        return Variable(juce::jlimit(abscissa->getValue(0), abscissa->getValue(
            abscissa->getNumElements() - 1), random->sampleDoubleFromGaussian(mean, stddev)));
    else
    {
      size_t minIndex = 0;
      size_t maxIndex = integral->getNumElements() - 1;
      double randomSeed = random->sampleDouble();
      double newValue;
      while (std::abs((int)(minIndex - maxIndex)) > 1)
      {
        size_t newIndex = (size_t)std::floor((minIndex + maxIndex) / 2.0);
        newValue = integral->getValue(newIndex) - randomSeed;
        if (newValue >= 0)
          maxIndex = newIndex;
        else
          minIndex = newIndex;
      }

      double finalValue = ((abscissa->getValue(maxIndex) - abscissa->getValue(minIndex))
          / (integral->getValue(maxIndex) - integral->getValue(minIndex))) * (randomSeed
          - integral->getValue(maxIndex)) + abscissa->getValue(maxIndex);
      return Variable(finalValue);
    }
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */

  static void getMeanAndVariance(const std::vector<Variable>& dataset, double& mean, double& variance)
  {
    ScalarVariableMeanAndVariance v;
    for (size_t i = 0; i < dataset.size(); i++)
      v.push(dataset[i].getDouble());
    mean = v.getMean();
    variance = v.getVariance();
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 2)
      return;

    learned = true;
    double temporaryVariance;
    getMeanAndVariance(dataset, mean, temporaryVariance);
    if (temporaryVariance == 0)
      temporaryVariance = juce::jmax(1.0, std::abs(mean * 0.3));
    stddev = std::sqrt(temporaryVariance);

    double delta = 0;
    if (!fixedAbscissa)
      delta = std::abs(2 * excursion * stddev * precision);
    else
      delta = (abscissa->getValue(abscissa->getNumElements() - 1) - abscissa->getValue(0))
          * precision;

    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    if (!fixedAbscissa)
    {
      double minAbscissa = mean - excursion * stddev;
      double maxAbscissa = mean + excursion * stddev;
      abscissa = createAbscissa(minAbscissa, maxAbscissa, delta);
    }
    double stdCorrectionFactor = dataset.size() >= 20 ? kernelWidth : kernelWidth * (20.0
        / (double)dataset.size());
    DenseDoubleVectorPtr frequencies = createFrequencies(dataset, abscissa, stddev * stdCorrectionFactor);
    integral = createIntegral(frequencies, delta);
  }

  /**
   * Creates the integration of the probability density function of the data provided.
   */
  DenseDoubleVectorPtr createIntegral(DenseDoubleVectorPtr& frequencies, double delta)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr integral = new DenseDoubleVector(actionClass,
        frequencies->getNumElements(), 0.0);

    double accumulator = 0;
    for (size_t j = 0; j < frequencies->getNumElements(); j++)
    {
      accumulator += frequencies->getValue(j);
      if (j == 0)
        integral->setValue(j, 0);
      else if (j == frequencies->getNumElements() - 1)
        integral->setValue(j, 1.0);
      else
        integral->setValue(j, accumulator * delta);

      if (integral->getValue(j) > 1)
        integral->setValue(j, 1.0);
    }

    return integral;
  }

  /**
   * Creates the frequencies of the data provided, i.e. its probability density function.
   */
  DenseDoubleVectorPtr createFrequencies(
      const std::vector<Variable>& dataset, DenseDoubleVectorPtr& abscissa,
      double gaussianDeviation)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr frequencies = new DenseDoubleVector(actionClass,
        abscissa->getNumElements(), 0.0);

    for (size_t i = 0; i < dataset.size(); i++)
    {
      for (size_t j = 0; j < frequencies->getNumElements(); j++)
      {
        double oldValue = frequencies->getValue(j);
        double inc = std::exp(-std::pow(abscissa->getValue(j) - dataset[i].getDouble(), 2)
            / (2 * std::pow(gaussianDeviation, 2)));
        frequencies->setValue(j, oldValue + inc);
      }
    }

    double normalize = 1.0 / ((double)dataset.size() * std::sqrt(2 * M_PI * std::pow(
        gaussianDeviation, 2)));
    for (size_t i = 0; i < frequencies->getNumElements(); i++)
    {
      double oldValue = frequencies->getValue(i) * normalize;
      frequencies->setValue(i, oldValue);
    }
    return frequencies;
  }

  /**
   * Creates a linearly spaced vector containing the abscissa where the probability density
   * function and the integration will be evaluated.
   */
  DenseDoubleVectorPtr createAbscissa(double minAbscissa, double maxAbscissa, double delta)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr abscissa = new DenseDoubleVector(actionClass, 0, 0.0);
    double tempAbscissa = minAbscissa;
    while (tempAbscissa <= maxAbscissa)
    {
      abscissa->append(Variable(tempAbscissa));
      tempAbscissa += delta;
    }
    return abscissa;
  }

protected:
  friend class ParzenContinuousSamplerClass;
  bool learned;
  DenseDoubleVectorPtr integral;
  DenseDoubleVectorPtr abscissa;
  double mean;
  double stddev;
  double precision;
  double excursion;
  double kernelWidth;
  bool fixedAbscissa;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_PARZEN_CONTINUOUS_SAMPLER_H_

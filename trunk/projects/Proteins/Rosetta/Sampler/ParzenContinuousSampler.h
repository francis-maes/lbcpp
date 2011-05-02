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

class ParzenContinuousSampler: public ContinuousSampler
{
public:
  ParzenContinuousSampler() :
    ContinuousSampler(), precision(0.0001), excursion(6), kernelWidth(0.25), learned(false)
  {
  }

  ParzenContinuousSampler(double precision, double excursion = 6, double kernelWidth = 0.25) :
    ContinuousSampler(), precision(precision), excursion(excursion), kernelWidth(kernelWidth),
        learned(false)
  {
  }

  ParzenContinuousSampler(const ParzenContinuousSampler& sampler) :
    ContinuousSampler(sampler.mean, sampler.std), learned(sampler.learned), precision(
        sampler.precision), excursion(sampler.excursion), kernelWidth(sampler.kernelWidth)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    integral = new DenseDoubleVector(actionClass, sampler.integral->getValues());
    abscissa = new DenseDoubleVector(actionClass, sampler.abscissa->getValues());
  }

  ~ParzenContinuousSampler()
  {
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    if (!learned)
      return Variable(random->sampleDoubleFromGaussian());
    else
    {
      size_t minIndex = 0;
      size_t maxIndex = integral->getNumElements() - 1;
      double randomSeed = random->sampleDouble();
      double newValue;
      while (std::abs((int)(minIndex - maxIndex)) > 1)
      {
        size_t newIndex = (size_t)std::floor((minIndex + maxIndex) / 2);
        newValue = integral->getValue(newIndex) - randomSeed;
        if (newValue >= 0)
          maxIndex = newIndex;
        else
          minIndex = newIndex;
      }
      double minValue = integral->getValue(minIndex) - randomSeed;
      double maxValue = integral->getValue(maxIndex) - randomSeed;
      if (std::abs(minValue) > std::abs(maxValue))
        return Variable(abscissa->getValue(maxIndex));
      else
        return Variable(abscissa->getValue(minIndex));
    }
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() == 0)
      return;

    learned = true;
    mean = getMean(dataset);
    std = std::sqrt(getVariance(dataset, mean));

    double delta = std::abs(2 * excursion * std * precision);
    double minAbscissa = mean - excursion * std;
    double maxAbscissa = mean + excursion * std;

    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    abscissa = createAbscissa(minAbscissa, maxAbscissa, delta);
    DenseDoubleVectorPtr frequencies = createFrequencies(dataset, abscissa, std * kernelWidth);
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
    for (int j = 0; j < frequencies->getNumElements(); j++)
    {
      accumulator += frequencies->getValue(j);
      integral->setValue(j, accumulator * delta);
    }
    return integral;
  }

  /**
   * Creates the frequencies of the data provided, i.e. its probability density function.
   */
  DenseDoubleVectorPtr createFrequencies(
      const std::vector<std::pair<Variable, Variable> >& dataset, DenseDoubleVectorPtr& abscissa,
      double gaussianDeviation)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr frequencies = new DenseDoubleVector(actionClass,
        abscissa->getNumElements(), 0.0);

    for (int i = 0; i < dataset.size(); i++)
    {
      for (int j = 0; j < frequencies->getNumElements(); j++)
      {
        double oldValue = frequencies->getValue(j);
        double inc = std::exp(-std::pow(abscissa->getValue(j) - dataset[i].first.getDouble(), 2)
            / (2 * std::pow(gaussianDeviation, 2)));
        frequencies->setValue(j, oldValue + inc);
      }
    }

    double normalize = 1.0 / ((double)dataset.size() * std::sqrt(2 * M_PI * std::pow(
        gaussianDeviation, 2)));
    for (int i = 0; i < frequencies->getNumElements(); i++)
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
    while (tempAbscissa < maxAbscissa)
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
  double precision;
  double excursion;
  double kernelWidth;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_PARZEN_CONTINUOUS_SAMPLER_H_

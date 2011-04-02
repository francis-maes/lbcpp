/*-----------------------------------------.---------------------------------.
| Filename: ContinuousProbabilityDist...cpp| Continuous Probability          |
| Author  : Julien Becker                  | Distributions                   |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Distribution/ContinuousDistribution.h>
#include "Builder/GaussianDistributionBuilder.h"


using namespace lbcpp;

/*
** UniformDistribution
*/
double UniformDistribution::computeEntropy() const
  {return log(maximum - minimum);}

double UniformDistribution::computeProbability(const Variable& value) const
{
  jassert(minimum < maximum);
  double d = value.getDouble();
  if (d < minimum || d > maximum)
    return 0.0;
  else
    return 1.0 / (maximum - minimum);
}

Variable UniformDistribution::sample(RandomGeneratorPtr random) const
  {return random->sampleDouble(minimum, maximum);}

void UniformDistribution::sampleUniformly(size_t numSamples, std::vector<double>& res) const
{
  jassert(minimum < maximum);
  jassert(numSamples);
  res.resize(numSamples);
  double k = (maximum - minimum) / (numSamples - 1.0);
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = minimum + i * k;
}

/*
** GaussianDistribution
*/
double GaussianDistribution::computeEntropy() const
  {return 0.5 * log(2 * M_PI * exp(1.0) * getVariance());}

double GaussianDistribution::computeProbability(const Variable& value) const
{
  jassert(value.isDouble());
  double mean = getMean();
  double variance = getVariance();
  double squaredNumerator = value.getDouble() - mean;
  squaredNumerator *= squaredNumerator;
  return 1 / sqrt(variance * 2 * M_PI) * exp(-squaredNumerator/(2*variance));
}

Variable GaussianDistribution::sample(RandomGeneratorPtr random) const
  {return Variable(random->sampleDoubleFromGaussian(getMean(), getStandardDeviation()), doubleType);}

void GaussianDistribution::sampleUniformly(size_t numSamples, std::vector<double>& res) const
{
  jassert(false); // not implemented !
}

DistributionBuilderPtr GaussianDistribution::createBuilder() const
  {return new GaussianDistributionBuilder();}

/*
 ** IntegerGaussianDistribution
 */
DistributionBuilderPtr IntegerGaussianDistribution::createBuilder() const
  {return new IntegerGaussianDistributionBuilder();}

/*
 ** PositiveIntegerGaussianDistribution
 */
DistributionBuilderPtr PositiveIntegerGaussianDistribution::createBuilder() const
  {return new PositiveIntegerGaussianDistributionBuilder();}

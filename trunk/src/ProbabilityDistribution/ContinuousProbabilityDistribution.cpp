/*-----------------------------------------.---------------------------------.
| Filename: ContinuousProbabilityDist...cpp| Continuous Probability          |
| Author  : Julien Becker                  | Distributions                   |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Variable.h>
#include <lbcpp/ProbabilityDistribution/ContinuousProbabilityDistribution.h>
using namespace lbcpp;

/*
** UniformProbabilityDistribution
*/
double UniformProbabilityDistribution::computeEntropy() const
  {return log(maximum - minimum);}

double UniformProbabilityDistribution::compute(ExecutionContext& context, const Variable& value) const
{
  jassert(minimum < maximum);
  double d = value.getDouble();
  if (d < minimum || d > maximum)
    return 0.0;
  else
    return 1.0 / (maximum - minimum);
}

Variable UniformProbabilityDistribution::sample(RandomGeneratorPtr random) const
  {return random->sampleDouble(minimum, maximum);}

void UniformProbabilityDistribution::sampleUniformly(size_t numSamples, std::vector<double>& res) const
{
  jassert(minimum < maximum);
  jassert(numSamples);
  res.resize(numSamples);
  double k = (maximum - minimum) / (numSamples - 1.0);
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = minimum + i * k;
}

/*
** GaussianProbabilityDistribution
*/
double GaussianProbabilityDistribution::computeEntropy() const
  {return 0.5 * log(2 * M_PI * exp(1.0) * getVariance());}

double GaussianProbabilityDistribution::compute(ExecutionContext& context, const Variable& value) const
{
  jassert(value.isDouble());
  double mean = getMean();
  double variance = getVariance(); // FIXME: Variance or Standard Deviation ?????
  double squaredNumerator = value.getDouble() - mean;
  squaredNumerator *= squaredNumerator;
  double squaredDenominator = 2 * variance;
  squaredDenominator *= squaredDenominator;
  return 1 / sqrt(variance * 2 * M_PI) * exp(-squaredNumerator/squaredDenominator);
}

Variable GaussianProbabilityDistribution::sample(RandomGeneratorPtr random) const
  {return Variable(random->sampleDoubleFromGaussian(getMean(), getVariance()), doubleType);} // FIXME: variance or stddev ?

void GaussianProbabilityDistribution::sampleUniformly(size_t numSamples, std::vector<double>& res) const
{
  jassert(false); // not implemented !
}


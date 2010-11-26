/*-----------------------------------------.---------------------------------.
| Filename: ContinuousProbabilityDist...cpp| Continuous Probability          |
| Author  : Julien Becker                  | Distributions                   |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ProbabilityDistribution/ContinuousProbabilityDistribution.h>
#include <lbcpp/Data/Variable.h>

using namespace lbcpp;

/*
 ** GaussianProbabilityDistribution
 */
double GaussianProbabilityDistribution::computeEntropy() const
  {return 0.5 * log(2 * M_PI * exp(1) * values->getVariance());}

double GaussianProbabilityDistribution::compute(ExecutionContext& context, const Variable& value) const
{
  jassert(value.isDouble());
  double mean = values->getMean();
  double variance = values->getVariance(); // FIXME: Variance or Standard Deviation ?????
  double squaredNumerator = value.getDouble() - mean;
  squaredNumerator *= squaredNumerator;
  double squaredDenominator = 2 * variance;
  squaredDenominator *= squaredDenominator;
  return 1 / sqrt(variance * 2 * M_PI) * exp(-squaredNumerator/squaredDenominator);
}

Variable GaussianProbabilityDistribution::sample(RandomGeneratorPtr random) const
  {return Variable(random->sampleDoubleFromGaussian(values->getMean(), values->getStandardDeviation()), doubleType);}

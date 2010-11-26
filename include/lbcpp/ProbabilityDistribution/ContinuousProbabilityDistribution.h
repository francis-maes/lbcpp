/*-----------------------------------------.---------------------------------.
| Filename: ContinuousProbabilityDistri...h| Continuous Probability          |
| Author  : Julien Becker                  | Distributions                   |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/ProbabilityDistribution/ProbabilityDistribution.h>

namespace lbcpp
{

class ContinuousProbabilityDistribution : public ProbabilityDistribution
{
public:
  juce_UseDebuggingNewOperator
};

class GaussianProbabilityDistribution : public ContinuousProbabilityDistribution
{
public:
  GaussianProbabilityDistribution(double mean = 0.0, double variance = 0.0)
    : mean(mean), variance(variance) {}

  virtual double computeEntropy() const;

  virtual double compute(ExecutionContext& context, const Variable& value) const;

  virtual Variable sample(RandomGeneratorPtr random) const;

  double getMean() const
    {return mean;}

  double getVariance() const
    {return variance;}

  juce_UseDebuggingNewOperator

protected:
  friend class GaussianProbabilityDistributionClass;

  double mean;
  double variance;
};

typedef ReferenceCountedObjectPtr<GaussianProbabilityDistribution> GaussianProbabilityDistributionPtr;
  
}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_

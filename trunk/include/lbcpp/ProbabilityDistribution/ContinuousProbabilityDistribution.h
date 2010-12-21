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
  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const = 0;

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<ContinuousProbabilityDistribution> ContinuousProbabilityDistributionPtr;

class UniformProbabilityDistribution : public ContinuousProbabilityDistribution
{
public:
  UniformProbabilityDistribution(double minimum, double maximum)
    : minimum(minimum), maximum(maximum) {}
  UniformProbabilityDistribution() : minimum(0.0), maximum(0.0) {}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const;

  double getMinimum() const
    {return minimum;}

  double getMaximum() const
    {return maximum;}

  juce_UseDebuggingNewOperator

protected:
  friend class UniformProbabilityDistributionClass;

  double minimum;
  double maximum;
};

class GaussianProbabilityDistribution : public ContinuousProbabilityDistribution
{
public:
  GaussianProbabilityDistribution(double mean = 0.0, double variance = 0.0)
    : mean(mean), variance(variance) {}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const;

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

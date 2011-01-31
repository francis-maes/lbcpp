/*-----------------------------------------.---------------------------------.
| Filename: ContinuousProbabilityDistri...h| Continuous Probability          |
| Author  : Julien Becker                  | Distributions                   |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_

# include <lbcpp/Distribution/Distribution.h>

namespace lbcpp
{

class ContinuousDistribution : public Distribution
{
public:
  virtual TypePtr getElementsType() const
    {return doubleType;}

  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const = 0;

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<ContinuousDistribution> ContinuousDistributionPtr;

class UniformDistribution : public ContinuousDistribution
{
public:
  UniformDistribution(double minimum, double maximum)
    : minimum(minimum), maximum(maximum) {}
  UniformDistribution() : minimum(0.0), maximum(0.0) {}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {return sample(random);}

  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const;

  double getMinimum() const
    {return minimum;}

  double getMaximum() const
    {return maximum;}

  juce_UseDebuggingNewOperator

protected:
  friend class UniformDistributionClass;

  double minimum;
  double maximum;
};

typedef ReferenceCountedObjectPtr<UniformDistribution> UniformDistributionPtr;

class GaussianDistribution : public ContinuousDistribution
{
public:
  GaussianDistribution(double mean = 0.0, double variance = 0.0)
    : mean(mean), variance(variance) {}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {return mean;}
  
  virtual void sampleUniformly(size_t numSamples, std::vector<double>& res) const;

  double getMean() const
    {return mean;}

  double getVariance() const
    {return variance;}

  juce_UseDebuggingNewOperator

protected:
  friend class GaussianDistributionClass;

  double mean;
  double variance;
};

typedef ReferenceCountedObjectPtr<GaussianDistribution> GaussianDistributionPtr;
  
}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_

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
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{  
class ContinuousProbabilityDistribution : public ProbabilityDistribution
{};

class GaussianProbabilityDistribution : public ContinuousProbabilityDistribution
{
public:
  GaussianProbabilityDistribution() : values(new ScalarVariableMeanAndVariance) {}
  
  virtual double computeEntropy() const;
  
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  
  virtual Variable sample(RandomGeneratorPtr random) const;
  
  void push(double value)
    {values->push(value);}
  
  double getMean() const
    {return values->getMean();}
  
  double getVariance() const
    {return values->getVariance();}
  
protected:
  friend class GaussianProbabilityDistributionClass;

  ScalarVariableMeanAndVariancePtr values;
};

typedef ReferenceCountedObjectPtr<GaussianProbabilityDistribution> GaussianProbabilityDistributionPtr;
  
}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_PROBABILITY_DISTRIBUTION_H_

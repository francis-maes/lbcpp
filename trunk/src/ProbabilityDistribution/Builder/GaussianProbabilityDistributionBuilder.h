/*-----------------------------------------.---------------------------------.
| Filename: GaussianProbabilit...Builder.h | Gaussian Probability            |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 14:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include <lbcpp/ProbabilityDistribution/ContinuousProbabilityDistribution.h>
# include <lbcpp/ProbabilityDistribution/ProbabilityDistributionBuilder.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/Type.h>

namespace lbcpp
{
  
class GaussianProbabilityDistributionBuilder : public ProbabilityDistributionBuilder
{
public:
  GaussianProbabilityDistributionBuilder()
    : means(new ScalarVariableMean), variances(new ScalarVariableMean) {}

  virtual TypePtr getInputType() const
    {return doubleType;}
  
  virtual void clear()
  {
    means = new ScalarVariableMean();
    variances = new ScalarVariableMean();
  }
  
  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), doubleType))
      return;
    meanAndVariances->push(element.getDouble());
  }
  
  virtual void addDistribution(const ProbabilityDistributionPtr& value, double weight)
  {
    GaussianProbabilityDistributionPtr valueDistribution = value.staticCast<GaussianProbabilityDistribution>();
    jassert(valueDistribution);
    means->push(valueDistribution->getMean());
    variances->push(valueDistribution->getVariance());
  }
  
  virtual ProbabilityDistributionPtr build() const
  {
    double mean = means->getMean();
    double variance = variances->getMean();

    if (meanAndVariances->getCount() != 0.0) // consider added elements as a unique distribution 
    {
      double count = means->getCount();
      mean = (count * mean + meanAndVariances->getMean()) / (count + 1);
      variance = (count * variance + meanAndVariances->getVariance()) / (count + 1);
    }
    return new GaussianProbabilityDistribution(mean, variance);
  }
  
protected:
  friend class GaussianProbabilityDistributionBuilderClass;
  // from Gaussian distributions
  ScalarVariableMeanPtr means;
  ScalarVariableMeanPtr variances;
  // from elements
  ScalarVariableMeanAndVariancePtr meanAndVariances;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

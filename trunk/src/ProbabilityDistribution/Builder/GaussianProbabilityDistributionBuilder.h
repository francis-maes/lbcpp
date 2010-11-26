/*-----------------------------------------.---------------------------------.
| Filename: GaussianProbabilit...Builder.h | Gaussian Probability            |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 14:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

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
  
  virtual void clear();
  
  virtual void addElement(const Variable& element, double weight);
  
  virtual void addDistribution(const ProbabilityDistributionPtr& value, double weight);
  
  virtual ProbabilityDistributionPtr build() const;
  
protected:
  friend class GaussianProbabilityDistributionBuilderClass;
  
  ScalarVariableMeanPtr means;
  ScalarVariableMeanPtr variances;
  ScalarVariableMeanAndVariancePtr meanAndVariances;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

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
  virtual TypePtr getInputType() const
    {return doubleType;}
  
  virtual void clear()
  {
    if (means)
      means->clear();
    if (variances)
      variances->clear();
    if (meanAndVariances)
      meanAndVariances->clear();
  }
  
  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), doubleType))
      return;
    ensureScalarMeanAndVarianceIsInitialized();
    meanAndVariances->push(element.getDouble());
  }
  
  virtual void addDistribution(const ProbabilityDistributionPtr& value, double weight)
  {
    GaussianProbabilityDistributionPtr valueDistribution = value.staticCast<GaussianProbabilityDistribution>();
    jassert(valueDistribution);
    ensureScalarMeanAreInitialized();
    means->push(valueDistribution->getMean());
    variances->push(valueDistribution->getVariance());
  }
  
  virtual ProbabilityDistributionPtr build() const
  {
    jassert((means && variances) != meanAndVariances);

    if (means)
      return new GaussianProbabilityDistribution(means->getMean(), variances->getMean());
    if (meanAndVariances)
      return new GaussianProbabilityDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());

    jassertfalse;
    return GaussianProbabilityDistributionPtr();
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ProbabilityDistributionBuilder::clone(context, target);
    ReferenceCountedObjectPtr<GaussianProbabilityDistributionBuilder> targ = target.staticCast<GaussianProbabilityDistributionBuilder>();
    targ->means = ScalarVariableMeanPtr();
    targ->variances = ScalarVariableMeanPtr();
    targ->meanAndVariances = ScalarVariableMeanAndVariancePtr();
  }
  
protected:
  friend class GaussianProbabilityDistributionBuilderClass;
  // from Gaussian distributions
  ScalarVariableMeanPtr means;
  ScalarVariableMeanPtr variances;
  // from elements
  ScalarVariableMeanAndVariancePtr meanAndVariances;
  
private:
  void ensureScalarMeanAreInitialized()
  {
    jassert((means && !variances) || (!means && variances));
    if (!means)
    {
      means = new ScalarVariableMean();
      variances = new ScalarVariableMean();
    }
  }
  
  void ensureScalarMeanAndVarianceIsInitialized()
  {
    if (!meanAndVariances)
      meanAndVariances = new ScalarVariableMeanAndVariance();
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

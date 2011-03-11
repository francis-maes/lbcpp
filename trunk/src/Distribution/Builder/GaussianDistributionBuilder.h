/*-----------------------------------------.---------------------------------.
| Filename: GaussianProbabilit...Builder.h | Gaussian Probability            |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 14:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

//# include <lbcpp/Distribution/ContinuousDistribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/Type.h>

namespace lbcpp
{
  
class GaussianDistributionBuilder : public DistributionBuilder
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
  
  virtual void addDistribution(const DistributionPtr& value, double weight);
  /*{
    GaussianDistributionPtr valueDistribution = value.staticCast<GaussianDistribution>();
    jassert(valueDistribution);
    ensureScalarMeanAreInitialized();
    means->push(valueDistribution->getMean());
    variances->push(valueDistribution->getVariance());
  }*/
  
  virtual DistributionPtr build(ExecutionContext& context) const;
  /*{
    jassert((means && variances) != meanAndVariances);

    if (means)
      return new GaussianDistribution(means->getMean(), variances->getMean());
    if (meanAndVariances)
      return new GaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());

    jassertfalse;
    return GaussianDistributionPtr();
  }*/
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    DistributionBuilder::clone(context, target);
    ReferenceCountedObjectPtr<GaussianDistributionBuilder> targ = target.staticCast<GaussianDistributionBuilder>();
    targ->means = ScalarVariableMeanPtr();
    targ->variances = ScalarVariableMeanPtr();
    targ->meanAndVariances = ScalarVariableMeanAndVariancePtr();
  }
  
protected:
  friend class GaussianDistributionBuilderClass;
  // from Gaussian distributions
  ScalarVariableMeanPtr means;
  ScalarVariableMeanPtr variances;
  // from elements
  ScalarVariableMeanAndVariancePtr meanAndVariances;

  void ensureScalarMeanAreInitialized()
  {
    jassert((!means && !variances) || (means && variances));
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
  
class IntegerGaussianDistributionBuilder : public GaussianDistributionBuilder
{
  virtual TypePtr getInputType() const
    {return integerType;}
  
  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), getInputType()))
      return;
    ensureScalarMeanAndVarianceIsInitialized();
    meanAndVariances->push(element.getInteger());
  }
  
  virtual DistributionPtr build(ExecutionContext& context) const;
  /*{
    jassert((means && variances) != meanAndVariances);
    
    if (means)
      return new IntegerGaussianDistribution(means->getMean(), variances->getMean());
    if (meanAndVariances)
      return new IntegerGaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());
    
    jassertfalse;
    return IntegerGaussianDistributionPtr();
  }*/
  
};
  
class PositiveIntegerGaussianDistributionBuilder : public IntegerGaussianDistributionBuilder
{
  virtual TypePtr getInputType() const
    {return positiveIntegerType;}
  
  virtual DistributionPtr build(ExecutionContext& context) const;
  /*{
    jassert((means && variances) != meanAndVariances);
    
    if (means)
      return new PositiveIntegerGaussianDistribution(means->getMean(), variances->getMean());
    if (meanAndVariances)
      return new PositiveIntegerGaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());
    
    jassertfalse;
    return PositiveIntegerGaussianDistributionPtr();
  }*/
    
};
  

  
}; /* namespace lbcpp */

#endif // !LBCPP_GAUSSIAN_PROBABILITY_DISTRIBUTION_BUILDER_H_

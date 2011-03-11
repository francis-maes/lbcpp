/*-----------------------------------------.---------------------------------.
| Filename: Gaussian...Builder.cpp         | Gaussian Probability            |
| Author  : Arnaud Schoofs                 | Distributions Builder           |
| Started : 11/03/2011 21:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#include "GaussianDistributionBuilder.h"
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

using namespace lbcpp;

/**
 * GaussianDistributionBuilder
 */
void GaussianDistributionBuilder::addDistribution(const DistributionPtr& value, double weight)
{
  GaussianDistributionPtr valueDistribution = value.staticCast<GaussianDistribution>();
  jassert(valueDistribution);
  ensureScalarMeanAreInitialized();
  means->push(valueDistribution->getMean());
  variances->push(valueDistribution->getVariance());
}

DistributionPtr GaussianDistributionBuilder::build(ExecutionContext& context) const
{
  jassert((means && variances) != meanAndVariances);
  
  if (means)
    return new GaussianDistribution(means->getMean(), variances->getMean());
  if (meanAndVariances)
    return new GaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());
  
  jassertfalse;
  return GaussianDistributionPtr();
}

/**
 * IntegerGaussianDistributionBuilder
 */
DistributionPtr IntegerGaussianDistributionBuilder::build(ExecutionContext& context) const
{
 jassert((means && variances) != meanAndVariances);
 
 if (means)
 return new IntegerGaussianDistribution(means->getMean(), variances->getMean());
 if (meanAndVariances)
 return new IntegerGaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());
 
 jassertfalse;
 return IntegerGaussianDistributionPtr();
}

/**
 * PositiveIntegerGaussianDistributionBuilder
 */
DistributionPtr PositiveIntegerGaussianDistributionBuilder::build(ExecutionContext& context) const
{
 jassert((means && variances) != meanAndVariances);
 
 if (means)
 return new PositiveIntegerGaussianDistribution(means->getMean(), variances->getMean());
 if (meanAndVariances)
 return new PositiveIntegerGaussianDistribution(meanAndVariances->getMean(), meanAndVariances->getVariance());
 
 jassertfalse;
 return PositiveIntegerGaussianDistributionPtr();
}

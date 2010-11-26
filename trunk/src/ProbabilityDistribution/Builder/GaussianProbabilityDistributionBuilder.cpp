/*-----------------------------------------.---------------------------------.
| Filename: GaussianProbabil...Builder.cpp | Gaussian Probability            |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 14:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GaussianProbabilityDistributionBuilder.h"
#include <lbcpp/ProbabilityDistribution/ContinuousProbabilityDistribution.h>
#include <lbcpp/Core/Type.h>
#include <lbcpp/Core/Variable.h>

using namespace lbcpp;

void GaussianProbabilityDistributionBuilder::clear()
{
  means = new ScalarVariableMean();
  variances = new ScalarVariableMean();
}

void GaussianProbabilityDistributionBuilder::addElement(const Variable& element, double weight)
{
  if (!checkInheritance(element.getType(), doubleType))
    return;
  meanAndVariances->push(element.getDouble());
}

void GaussianProbabilityDistributionBuilder::addDistribution(const ProbabilityDistributionPtr& value, double weight)
{
  GaussianProbabilityDistributionPtr valueDistribution = value.staticCast<GaussianProbabilityDistribution>();
  jassert(valueDistribution);
  means->push(valueDistribution->getMean());
  variances->push(valueDistribution->getVariance());
}

ProbabilityDistributionPtr GaussianProbabilityDistributionBuilder::build() const
{
  if (meanAndVariances->getCount() != 0.0)
  {
    means->push(meanAndVariances->getMean());
    variances->push(meanAndVariances->getVariance());
  }
  return new GaussianProbabilityDistribution(means->getMean(), variances->getMean());
}

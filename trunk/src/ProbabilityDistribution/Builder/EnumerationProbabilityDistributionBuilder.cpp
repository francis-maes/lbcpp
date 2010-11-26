/*-----------------------------------------.---------------------------------.
| Filename: EnumeationProbab...Builder.cpp | Enumeration Probability         |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 14:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "EnumerationProbabilityDistributionBuilder.h"

using namespace lbcpp;

void EnumerationProbabilityDistributionBuilder::addElement(const Variable& element, double weight)
{
  if (!checkInheritance(element.getType(), elementType))
    return;
  distribution->increment(element, weight);
}

void EnumerationProbabilityDistributionBuilder::addDistribution(const ProbabilityDistributionPtr& value, double weight)
{
  EnumerationProbabilityDistributionPtr valueDistribution = value.staticCast<EnumerationProbabilityDistribution>();
  jassert(valueDistribution);
  jassert(valueDistribution->getEnumeration() == distribution->getEnumeration());
  size_t n = distribution->getEnumeration()->getNumElements();
  for (size_t i = 0; i <= n; ++i)
    distribution->setProbability(i, distribution->getProbability(i) + valueDistribution->getProbability(i));
  distribution->increment(Variable::missingValue(distribution->getEnumeration()), 0); // increment counter
}

ProbabilityDistributionPtr EnumerationProbabilityDistributionBuilder::build() const
{
  distribution->normalize();
  return distribution;
}

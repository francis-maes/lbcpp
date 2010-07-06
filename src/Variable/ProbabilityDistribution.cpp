/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.cpp    | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ProbabilityDistribution.h>
using namespace lbcpp;

/*
** DiscreteProbabilityDistribution
*/
DiscreteProbabilityDistribution::DiscreteProbabilityDistribution(EnumerationPtr enumeration)
  : enumeration(enumeration), values(enumeration->getNumElements(), 0.0), sum(0.0) {}

double DiscreteProbabilityDistribution::compute(const Variable& value) const
{
  if (!checkInheritance(value, enumeration))
    return 0.0;
  int index = value.getInteger();
  jassert(index >= 0 && index < (int)values.size());
  return sum ? values[index] / sum : 0.0;
}

void DiscreteProbabilityDistribution::setVariable(size_t index, const Variable& value)
{
  jassert(value.isDouble());
  sum -= values[index];
  values[index] = value.getDouble();
  sum += values[index];
}

void DiscreteProbabilityDistribution::increment(const Variable& value)
{
  if (checkInheritance(value, enumeration))
  {
    ++(values[value.getInteger()]);
    ++sum;
  }
}

/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.h      | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

# include "../Data/RandomGenerator.h"

namespace lbcpp
{

class ProbabilityDistribution : public Object
{
public:
  ProbabilityDistribution(ClassPtr thisClass) : Object(thisClass) {}
  ProbabilityDistribution() : Object() {}
  
  virtual double computeEntropy() const = 0;
  virtual double compute(ExecutionContext& context, const Variable& value) const = 0;
  virtual Variable sample(RandomGeneratorPtr random) const = 0;
};

typedef ReferenceCountedObjectPtr<ProbabilityDistribution> ProbabilityDistributionPtr;

extern ClassPtr probabilityDistributionClass;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistrib...Builder.h | Probability Distributions       |
| Author  : Julien Becker                  | Builder                         |
| Started : 26/07/2010 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include "Distribution.h"

namespace lbcpp
{
  
class DistributionBuilder : public Object
{
public:
  virtual TypePtr getInputType() const = 0;
  virtual void clear() = 0;
  virtual void addElement(const Variable& element, double weight = 1.0) = 0;
  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0) = 0;
  virtual DistributionPtr build(ExecutionContext& context) const = 0;
};

typedef ReferenceCountedObjectPtr<DistributionBuilder> DistributionBuilderPtr;

extern ClassPtr probabilityDistributionBuilderClass;
extern ClassPtr bernoulliDistributionBuilderClass;

extern DistributionBuilderPtr gaussianDistributionBuilder();
extern DistributionBuilderPtr enumerationDistributionBuilder(EnumerationPtr enumeration);
extern DistributionBuilderPtr bernoulliDistributionBuilder();

}; /* namespace lbcpp */

#endif // !LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_

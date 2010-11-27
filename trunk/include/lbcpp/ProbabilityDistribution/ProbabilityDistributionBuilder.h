/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistrib...Builder.h | Probability Distributions       |
| Author  : Julien Becker                  | Builder                         |
| Started : 26/07/2010 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include "ProbabilityDistribution.h"

namespace lbcpp
{
  
class ProbabilityDistributionBuilder : public Object
{
public:
  virtual TypePtr getInputType() const = 0;
  virtual void clear() = 0;
  virtual void addElement(const Variable& element, double weight = 1.0) = 0;
  virtual void addDistribution(const ProbabilityDistributionPtr& distribution, double weight = 1.0) = 0;
  virtual ProbabilityDistributionPtr build() const = 0;
};

typedef ReferenceCountedObjectPtr<ProbabilityDistributionBuilder> ProbabilityDistributionBuilderPtr;

extern ClassPtr probabilityDistributionBuilderClass;

extern ProbabilityDistributionBuilderPtr gaussianProbabilityDistributionBuilder();
extern ProbabilityDistributionBuilderPtr enumerationProbabilityDistributionBuilder(TypePtr elementType);

}; /* namespace lbcpp */

#endif // !LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_H_

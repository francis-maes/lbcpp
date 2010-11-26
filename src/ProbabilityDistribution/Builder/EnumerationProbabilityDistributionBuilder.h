/*-----------------------------------------.---------------------------------.
| Filename: EnumeationProbabil...Builder.h | Enumeration Probability         |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include <lbcpp/ProbabilityDistribution/ProbabilityDistributionBuilder.h>
# include <lbcpp/ProbabilityDistribution/DiscreteProbabilityDistribution.h>

namespace lbcpp
{
  
class EnumerationProbabilityDistributionBuilder : public ProbabilityDistributionBuilder
{
public:
  EnumerationProbabilityDistributionBuilder(TypePtr elementType)
    : elementType(elementType), distribution(new EnumerationProbabilityDistribution(elementType))
    {jassert(elementType->inheritsFrom(enumValueType));}

  EnumerationProbabilityDistributionBuilder() {}
  
  virtual TypePtr getInputType() const
    {return elementType;}

  virtual void clear()
    {distribution = new EnumerationProbabilityDistribution(elementType);}

  virtual void addElement(const Variable& element, double weight);

  virtual void addDistribution(const ProbabilityDistributionPtr& value, double weight);

  virtual ProbabilityDistributionPtr build() const;
  
protected:
  friend class EnumerationProbabilityDistributionBuilderClass;
  
  TypePtr elementType;
  EnumerationProbabilityDistributionPtr distribution;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_

/*-----------------------------------------.---------------------------------.
| Filename: EnumeationProbabil...Builder.h | Enumeration Probability         |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 26/07/2010 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{
  
class EnumerationDistributionBuilder : public DistributionBuilder
{
public:
  EnumerationDistributionBuilder(EnumerationPtr enumeration)
    : enumeration(enumeration)
  , elementValues(enumeration->getNumElements() + 1, 0.0)
  , distributionValues(enumeration->getNumElements() + 1, 0.0)
  , cacheDistribution(new EnumerationDistribution(enumeration))
    {}

  EnumerationDistributionBuilder() {}
  
  virtual TypePtr getInputType() const
    {return enumeration;}

  virtual void clear()
  {
    elementValues.clear();
    elementValues.resize(enumeration->getNumElements() + 1, 0.0);
    distributionValues.clear();
    distributionValues.resize(enumeration->getNumElements() + 1, 0.0);
  }

  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), enumeration))
      return;
    elementValues[element.getInteger()] += weight;
  }

  virtual void addDistribution(const DistributionPtr& value, double weight)
  {
    EnumerationDistributionPtr enumDistribution = value.staticCast<EnumerationDistribution>();
    jassert(enumDistribution && enumDistribution->getEnumeration() == enumeration);

    for (size_t i = 0; i < enumeration->getNumElements() + 1; ++i)
      distributionValues[i] += enumDistribution->getProbability(i) * weight;
  }

  virtual DistributionPtr build(ExecutionContext& context) const
  {
    std::vector<double> values(enumeration->getNumElements() + 1, 0.0);
    // consider added elements as a unique distribution
    normalize(elementValues, values);
    for (size_t i = 0; i < values.size(); ++i)
      values[i] += distributionValues[i];
    normalize(values, values);
    jassert(cacheDistribution);
    EnumerationDistributionPtr res = cacheDistribution->cloneAndCast<EnumerationDistribution>(context);
    res->values = values;
    jassert(res);
    return res;
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    DistributionBuilder::clone(context, target);
    target.staticCast<EnumerationDistributionBuilder>()->cacheDistribution = cacheDistribution;
  }
  
protected:
  friend class EnumerationDistributionBuilderClass;

  EnumerationPtr enumeration;
  std::vector<double> elementValues;
  std::vector<double> distributionValues;

  void normalize(const std::vector<double>& values, std::vector<double>& results) const
  {
    jassert(values.size() == results.size());
    double sum = 0.0;
    for (size_t i = 0; i < values.size(); ++i)
      sum += values[i];
    if (sum == 0.0)
      return;
    for (size_t i = 0; i < values.size(); ++i)
      results[i] = values[i] / sum;
  }
  
private:
  EnumerationDistributionPtr cacheDistribution;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_

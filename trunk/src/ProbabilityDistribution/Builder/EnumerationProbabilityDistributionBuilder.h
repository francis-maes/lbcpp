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
  EnumerationProbabilityDistributionBuilder(EnumerationPtr enumeration)
    : enumeration(enumeration), elementValues(enumeration->getNumElements() + 1, 0.0), distributionValues(enumeration->getNumElements() + 1, 0.0)
    {}

  EnumerationProbabilityDistributionBuilder() {}
  
  virtual TypePtr getInputType() const
    {return enumeration;}

  virtual void clear()
  {
    elementValues.clear();
    distributionValues.clear();
  }

  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), enumeration))
      return;
    elementValues[element.getInteger()] += weight;
  }

  virtual void addDistribution(const ProbabilityDistributionPtr& value, double weight)
  {
    EnumerationProbabilityDistributionPtr enumDistribution = value.staticCast<EnumerationProbabilityDistribution>();
    jassert(enumDistribution && enumDistribution->getEnumeration() == enumeration);

    for (size_t i = 0; i < distributionValues.size(); ++i)
      distributionValues[i] += enumDistribution->getProbability(i) * weight;
  }

  virtual ProbabilityDistributionPtr build() const
  {
    std::vector<double> res(enumeration->getNumElements() + 1, 0.0);
    // consider added elements as a unique distribution
    normalize(elementValues, res);
    for (size_t i = 0; i < res.size(); ++i)
      res[i] += distributionValues[i];

    normalize(res, res);
    return enumerationProbabilityDistribution(enumeration, res);
  }
  
protected:
  friend class EnumerationProbabilityDistributionBuilderClass;

  EnumerationPtr enumeration;
  std::vector<double> elementValues;
  std::vector<double> distributionValues;

  void normalize(const std::vector<double>& values, std::vector<double>& results) const
  {
    jassert(values.size() == results.size());
    double sum = 0.0;
    for (size_t i = 0; i < values.size(); ++i)
      sum += values[i];
    for (size_t i = 0; i < values.size(); ++i)
      results[i] = values[i] / sum;
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_ENUMERATION_PROBABILITY_DISTRIBUTION_BUILDER_H_

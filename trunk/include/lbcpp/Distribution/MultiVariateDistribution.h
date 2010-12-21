/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis...h | MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_
# define LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_

# include "Distribution.h"
# include "../Core/Vector.h"
# include "../Core/Variable.h"

namespace lbcpp
{

class MultiVariateDistribution : public Distribution
{
public:
};

typedef ReferenceCountedObjectPtr<MultiVariateDistribution> MultiVariateDistributionPtr;

class IndependentMultiVariateDistribution : public MultiVariateDistribution
{
public:
  IndependentMultiVariateDistribution(ClassPtr variableClass);
  IndependentMultiVariateDistribution() {}

  ClassPtr getVariableClass() const
    {return getClass()->getTemplateArgument(0).dynamicCast<Class>();}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;

  const DistributionPtr& getSubDistribution(size_t index) const
    {jassert(index < distributions.size()); return distributions[index];}

protected:
  friend class IndependentMultiVariateDistributionClass;

  std::vector<DistributionPtr> distributions;
};

extern ClassPtr independentMultiVariateDistributionClass(TypePtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_

/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis...h | MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_
# define LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_

# include "ProbabilityDistribution.h"
# include "../Core/Vector.h"
# include "../Core/Variable.h"

namespace lbcpp
{

class MultiVariateProbabilityDistribution : public ProbabilityDistribution
{
public:
};

typedef ReferenceCountedObjectPtr<MultiVariateProbabilityDistribution> MultiVariateProbabilityDistributionPtr;

class IndependentMultiVariateProbabilityDistribution : public MultiVariateProbabilityDistribution
{
public:
  IndependentMultiVariateProbabilityDistribution(ClassPtr variableClass);
  IndependentMultiVariateProbabilityDistribution() {}

  ClassPtr getVariableClass() const
    {return getClass()->getTemplateArgument(0).dynamicCast<Class>();}

  virtual double computeEntropy() const;
  virtual double compute(ExecutionContext& context, const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;

  const ProbabilityDistributionPtr& getSubDistribution(size_t index) const
    {jassert(index < distributions.size()); return distributions[index];}

protected:
  friend class IndependentMultiVariateProbabilityDistributionClass;

  std::vector<ProbabilityDistributionPtr> distributions;
};

extern ClassPtr independentMultiVariateProbabilityDistributionClass(TypePtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_

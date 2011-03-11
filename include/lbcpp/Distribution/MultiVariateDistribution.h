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
  MultiVariateDistribution(ClassPtr thisClass)
    : Distribution(thisClass) {}
  MultiVariateDistribution() {}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<MultiVariateDistribution> MultiVariateDistributionPtr;

class IndependentMultiVariateDistribution : public MultiVariateDistribution
{
public:
  IndependentMultiVariateDistribution(ClassPtr elementsType);
  IndependentMultiVariateDistribution() {}

  virtual TypePtr getElementsType() const
    {return getClass()->getTemplateArgument(0);}

  virtual double computeEntropy() const;
  virtual double computeProbability(const Variable& value) const;
  virtual Variable sample(RandomGeneratorPtr random) const;
  virtual Variable sampleBest(RandomGeneratorPtr random) const;

  const DistributionPtr& getSubDistribution(size_t index) const
    {jassert(index < distributions.size()); return distributions[index];}

  void setSubDistribution(size_t index, const DistributionPtr& distribution)
    {jassert(index < distributions.size()); distributions[index] = distribution;}
  
  virtual DistributionBuilderPtr createBuilder() const;
   
  lbcpp_UseDebuggingNewOperator

protected:
  friend class IndependentMultiVariateDistributionClass;

  std::vector<DistributionPtr> distributions;
};

typedef ReferenceCountedObjectPtr<IndependentMultiVariateDistribution> IndependentMultiVariateDistributionPtr;
extern ClassPtr independentMultiVariateDistributionClass(TypePtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_PROBABILITY_DISTRIBUTION_MULTI_VARIATE_H_

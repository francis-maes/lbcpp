/*-----------------------------------------.---------------------------------.
| Filename: IndependentDoubleVectorDist...h| DistributionBuilder associated  |
| Author  : Arnaud Schoofs                 | with IndependentDoubleVector    |
| Started : 07/03/2011                     | Distribution                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef LBCPP_DISTRIBUTION_BUILDER_INDEPENDENT_DOUBLE_VECTOR_H_
# define LBCPP_DISTRIBUTION_BUILDER_INDEPENDENT_DOUBLE_VECTOR_H_

# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class IndependentDoubleVectorDistributionBuilder : public DistributionBuilder
{
public:
  IndependentDoubleVectorDistributionBuilder(EnumerationPtr elementsEnumeration, const std::vector<DistributionBuilderPtr>& subBuilders)
    : DistributionBuilder(independentDoubleVectorDistributionClass(elementsEnumeration)), elementsEnumeration(elementsEnumeration), subBuilders(subBuilders) {}
  IndependentDoubleVectorDistributionBuilder() {}

  virtual TypePtr getInputType() const
    {return denseDoubleVectorClass(elementsEnumeration);}

  virtual void clear()
  {
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->clear();
  }

  virtual void addElement(const Variable& element, double weight = 1.0)
  {
    const DenseDoubleVectorPtr& doubleVector = element.getObjectAndCast<DenseDoubleVector>();
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->addElement(doubleVector->getValue(i), weight);
  }

  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0)
  {
    IndependentDoubleVectorDistributionPtr indepDistribution = distribution.staticCast<IndependentDoubleVectorDistribution>();
    jassert(indepDistribution);
    
    if (indepDistribution->getElementsType() != getInputType()) jassertfalse;
    
    for (size_t i = 0; i < subBuilders.size(); ++i) 
      subBuilders[i]->addDistribution(indepDistribution->getSubDistribution(i));  
  }
  

  virtual DistributionPtr build(ExecutionContext& context) const
  {
    IndependentDoubleVectorDistributionPtr res = new IndependentDoubleVectorDistribution(elementsEnumeration);
    jassert(res->getNumSubDistributions() == subBuilders.size());
    for (size_t i = 0; i < subBuilders.size(); ++i)
      res->setSubDistribution(i, subBuilders[i]->build(context));
    return res;
  }

private:
  EnumerationPtr elementsEnumeration;
  std::vector<DistributionBuilderPtr> subBuilders;
};

};  /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTION_BUILDER_INDEPENDENT_DOUBLE_VECTOR_H_

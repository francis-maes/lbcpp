/*-----------------------------------------.---------------------------------.
| Filename: Ind..Mul..Distri..Builder.h    | DistributionBuilder associated  |
| Author  : Arnaud Schoofs                 | with IndependentMultiVariate    |
| Started : 07/03/2011                     | Distribution                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_
# define LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_

# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>

namespace lbcpp
{

class IndependentMultiVariateDistributionBuilder : public DistributionBuilder
{
public:
  IndependentMultiVariateDistributionBuilder(ClassPtr elementsType, const std::vector<DistributionBuilderPtr>& subBuilders)
    : DistributionBuilder(independentMultiVariateDistributionBuilderClass(elementsType)), subBuilders(subBuilders) {}
  IndependentMultiVariateDistributionBuilder() {}
  
  virtual TypePtr getInputType() const
    {return getClass()->getTemplateArgument(0);}
  
  virtual void clear() 
  {
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->clear();
  }
  
  virtual void addElement(const Variable& element, double weight = 1.0)
  {
    ObjectPtr res = element.getObject();
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->addElement(res->getVariable(i), weight);
  }
  
  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0)
  {
    IndependentMultiVariateDistributionPtr indepDistribution = distribution.staticCast<IndependentMultiVariateDistribution>();
    jassert(indepDistribution);
    
    if (indepDistribution->getElementsType() != getInputType()) jassertfalse;
    
    for (size_t i = 0; i < subBuilders.size(); ++i) 
      subBuilders[i]->addDistribution(indepDistribution->getSubDistribution(i));
  } 
  
  virtual DistributionPtr build(ExecutionContext& context) const
  {
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(getInputType());
    for (size_t i = 0; i < subBuilders.size(); ++i)
      distributions->setSubDistribution(i, subBuilders[i]->build(context));
    return distributions;
  }

  lbcpp_UseDebuggingNewOperator
    
protected:
  friend class IndependentMultiVariateDistributionBuilderClass;

  std::vector<DistributionBuilderPtr> subBuilders;
};

typedef ReferenceCountedObjectPtr<IndependentMultiVariateDistributionBuilder> IndependentMultiVariateDistributionBuilderPtr;

};  /* namespace lbcpp */

#endif // !LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_

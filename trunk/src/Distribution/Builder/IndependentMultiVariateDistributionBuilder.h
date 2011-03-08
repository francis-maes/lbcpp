/*
 *  IndependentMultiVariateDistributionBuilder.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 7/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_
# define LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_

# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <vector>

namespace lbcpp
{
  
  class IndependentMultiVariateDistributionBuilder : public DistributionBuilder
  {
  public:
    IndependentMultiVariateDistributionBuilder() {}
    IndependentMultiVariateDistributionBuilder(ClassPtr elementsType);
    
    virtual TypePtr getInputType() const
    {return getClass()->getTemplateArgument(0);}
    
    virtual void clear() 
    {
      for (size_t i = 0; i < distributionsBuilders.size(); ++i)
        distributionsBuilders[i]->clear();
    }
    
    virtual void addElement(const Variable& element, double weight = 1.0)
    {
      ObjectPtr res = element.getObject();
      for (size_t i = 0; i < distributionsBuilders.size(); ++i)
        distributionsBuilders[i]->addElement(res->getVariable(i), weight);
    }
    
    virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0)
    {jassertfalse;}  // not implemented // TODO arnaud
    
    virtual DistributionPtr build(ExecutionContext& context) const
    {
      IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(getInputType());
      for (size_t i = 0; i < distributionsBuilders.size(); ++i)
        distributions->setSubDistribution(i, distributionsBuilders[i]->build(context));
      return distributions;
    }
    
    const DistributionBuilderPtr& getSubDistributionBuilder(size_t index) const
      {jassert(index < distributionsBuilders.size()); return distributionsBuilders[index];}
    
    void setSubDistributionBuilder(size_t index, const DistributionBuilderPtr& distributionBuilder)
      {jassert(index < distributionsBuilders.size()); distributionsBuilders[index] = distributionBuilder;}
    
    lbcpp_UseDebuggingNewOperator
      
  protected:
    friend class IndependentMultiVariateDistributionBuilderClass;

    std::vector<DistributionBuilderPtr> distributionsBuilders;

  };
  
  typedef ReferenceCountedObjectPtr<IndependentMultiVariateDistributionBuilder> IndependentMultiVariateDistributionBuilderPtr;
  
};  /* namespace lbcpp */

#endif // !LBCPP_INDEPENDENT_MULTI_VARIATE_DISTRIBUTION_BUILDER_H_

/*
 *  predeclarations.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 11/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_DISTRIBUTION_PREDECLARATIONS_H_
# define LBCPP_DISTRIBUTION_PREDECLARATIONS_H_

# include "../Core/predeclarations.h"
namespace lbcpp
{
  class DistributionBuilder;
  typedef ReferenceCountedObjectPtr<DistributionBuilder> DistributionBuilderPtr; 
  
  class BernoulliDistributionBuilder;
  typedef ReferenceCountedObjectPtr<BernoulliDistributionBuilder> BernoulliDistributionBuilderPtr; 
  
  class EnumerationDistributionBuilder;
  typedef ReferenceCountedObjectPtr<EnumerationDistributionBuilder> EnumerationDistributionBuilderPtr;
  
  class GaussianDistributionBuilder;
  typedef ReferenceCountedObjectPtr<GaussianDistributionBuilder> GaussianDistributionBuilderPtr;
  
  class IntegerGaussianDistributionBuilder;
  typedef ReferenceCountedObjectPtr<IntegerGaussianDistributionBuilder> IntegerGaussianDistributionBuilderPtr;
  
  class PositiveIntegerGaussianDistributionBuilder;
  typedef ReferenceCountedObjectPtr<PositiveIntegerGaussianDistributionBuilder> PositiveIntegerGaussianDistributionBuilderPtr;
  
  class IndependentMultivariateDistributionBuilder;
  typedef ReferenceCountedObjectPtr<IndependentMultivariateDistributionBuilder> IndependentMultivariateDistributionBuilderPtr;

  
}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_PREDECLARATIONS_H_

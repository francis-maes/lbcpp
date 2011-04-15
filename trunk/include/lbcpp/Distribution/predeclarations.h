/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Distribution predeclarations    |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 11/03/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


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

#endif //!LBCPP_DISTRIBUTION_PREDECLARATIONS_H_

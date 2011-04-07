/*-----------------------------------------.---------------------------------.
| Filename: DistributionBuilder.h          | Probability Distributions       |
| Author  : Julien Becker                  | Builder                         |
| Started : 26/07/2010 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISTRIBUTION_BUILDER_H_
# define LBCPP_DISTRIBUTION_BUILDER_H_

# include "Distribution.h"

// TODO arnaud : weight is NOT implemented !!!

namespace lbcpp
{
  
class DistributionBuilder : public Object
{
public:
  DistributionBuilder(ClassPtr thisClass) : Object(thisClass) {}
  DistributionBuilder() : Object() {}
  
  static TypePtr getTemplateParameter(TypePtr type)
  {
    TypePtr dvType = type->findBaseTypeFromTemplateName(T("DistributionBuilder"));
    jassert(dvType && dvType->getNumTemplateArguments() == 1);
    TypePtr res = dvType->getTemplateArgument(0);
    jassert(res);
    return res;
  }
  static bool getTemplateParameter(ExecutionContext& context, TypePtr type, TypePtr& res)
  {
    TypePtr dvType = type->findBaseTypeFromTemplateName(T("DistributionBuilder"));
    if (!dvType)
    {
      context.errorCallback(type->getName() + T(" is not a DistributionBuilder"));
      return false;
    }
    jassert(dvType->getNumTemplateArguments() == 1);
    res = dvType->getTemplateArgument(0);
    return true;    
  }
  
  virtual TypePtr getInputType() const = 0;
  virtual void clear() = 0;
  virtual void addElement(const Variable& element, double weight = 1.0) = 0;
  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0) = 0;
  virtual DistributionPtr build(ExecutionContext& context) const = 0;
};

typedef ReferenceCountedObjectPtr<DistributionBuilder> DistributionBuilderPtr;

extern ClassPtr distributionBuilderClass(TypePtr elementsType = anyType);
  
extern ClassPtr bernoulliDistributionBuilderClass;
extern ClassPtr gaussianDistributionBuilderClass;
extern ClassPtr integerGaussianDistributionBuilderClass;
extern ClassPtr positiveGaussianDistributionBuilderClass;
extern ClassPtr independentMultiVariateDistributionBuilderClass(TypePtr elementsType);

extern DistributionBuilderPtr gaussianDistributionBuilder();
extern DistributionBuilderPtr integerGaussianDistributionBuilder();
extern DistributionBuilderPtr positiveIntegerGaussianDistributionBuilder();
extern DistributionBuilderPtr enumerationDistributionBuilder(EnumerationPtr enumeration);
extern DistributionBuilderPtr bernoulliDistributionBuilder();
  
}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTION_BUILDER_H_

/*-----------------------------------------.---------------------------------.
| Filename: Distribution.h                 | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

# include "../Data/RandomGenerator.h"
# include "../Core/Variable.h"
# include "predeclarations.h"

namespace lbcpp
{ 
  
class Distribution : public Object
{
public:
  Distribution(ClassPtr thisClass) : Object(thisClass) {}
  Distribution() : Object() {}

  static TypePtr getTemplateParameter(TypePtr type);
  static bool getTemplateParameter(ExecutionContext& context, TypePtr type, TypePtr& res);

  virtual TypePtr getElementsType() const = 0;

  virtual double computeEntropy() const = 0;
  virtual double computeProbability(const Variable& value) const = 0;
  virtual Variable sample(RandomGeneratorPtr random) const = 0;
  virtual Variable sampleBest(RandomGeneratorPtr random) const = 0;
  
  virtual DistributionBuilderPtr createBuilder() const = 0;
  
};

typedef ReferenceCountedObjectPtr<Distribution> DistributionPtr;

extern ClassPtr distributionClass(TypePtr elementsType = anyType);

extern FunctionPtr distributionEntropyFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

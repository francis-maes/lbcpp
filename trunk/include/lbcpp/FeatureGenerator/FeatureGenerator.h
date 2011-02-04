/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Base class for Feature          |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_H_

# include "../Function/Function.h"
# include "../Data/DoubleVector.h"

namespace lbcpp
{

class FeatureGeneratorCallback
{
public:
  virtual ~FeatureGeneratorCallback() {}

  virtual void sense(size_t index, double value) = 0;
  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight) = 0;
};

class FeatureGenerator : public Function
{
public:
  FeatureGenerator(bool lazy = true) : lazyComputation(lazy) {}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName) = 0;
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const = 0;

  virtual ClassPtr getNonLazyOutputType(EnumerationPtr featuresEnumeration, TypePtr featuresType) const
    {return sparseDoubleVectorClass(featuresEnumeration, featuresType);}

  virtual ClassPtr getLazyOutputType(EnumerationPtr featuresEnumeration, TypePtr featuresType) const
    {return lazyDoubleVectorClass(featuresEnumeration, featuresType);}

  const EnumerationPtr& getFeaturesEnumeration() const
    {return featuresEnumeration;}

  const TypePtr& getFeaturesType() const
    {return featuresType;}

  void setLazyComputation(bool lazyComputation)
    {this->lazyComputation = lazyComputation;}

  virtual DoubleVectorPtr toLazyVector(const Variable* inputs) const;
  virtual DoubleVectorPtr toComputedVector(const Variable* inputs) const;

  // Function
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class FeatureGeneratorClass;
  bool lazyComputation;

  EnumerationPtr featuresEnumeration;
  TypePtr featuresType;
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;


extern FeatureGeneratorPtr enumerationFeatureGenerator();
extern FeatureGeneratorPtr enumerationDistributionFeatureGenerator();

extern FeatureGeneratorPtr windowFeatureGenerator(size_t windowSize);
extern FeatureGeneratorPtr concatenateFeatureGenerator(bool lazy);

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_

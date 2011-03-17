/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Base class for Feature          |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_H_

# include "../Core/Function.h"
# include "../Data/DoubleVector.h"

namespace lbcpp
{

class FeatureGeneratorCallback
{
public:
  virtual ~FeatureGeneratorCallback() {}

  virtual void sense(size_t index, double value) = 0;
  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight) = 0;
  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight) = 0;
};

class FeatureGenerator : public Function
{
public:
  FeatureGenerator(bool lazy = true) : lazyComputation(lazy) {}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName) = 0;
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

  virtual double entropy(const Variable* inputs) const;
  virtual size_t l0norm(const Variable* inputs) const;
  virtual double sumOfSquares(const Variable* inputs) const;
  virtual double getMaximumValue(const Variable* inputs, size_t* index) const;
  virtual void appendTo(const Variable* inputs, const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const;
  virtual void addWeightedTo(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const;
  virtual double dotProduct(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const;

  // Function
  virtual String getOutputPostFix() const
    {return T("Features");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FeatureGeneratorClass;
  bool lazyComputation;

  EnumerationPtr featuresEnumeration;
  TypePtr featuresType;
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

// atomic
extern FeatureGeneratorPtr booleanFeatureGenerator(bool includeMissingValue = true);
extern FeatureGeneratorPtr enumerationFeatureGenerator(bool includeMissingValue = true);
extern FeatureGeneratorPtr doubleFeatureGenerator();

// atomic - number features
extern FeatureGeneratorPtr hardDiscretizedNumberFeatureGenerator(double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures = true);
extern FeatureGeneratorPtr softDiscretizedNumberFeatureGenerator(double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures = true, bool cyclicBehavior = false);
extern FeatureGeneratorPtr softDiscretizedLogNumberFeatureGenerator(double minimumLogValue, double maximumLogValue, size_t numIntervals, bool doOutOfBoundsFeatures = true);
extern FeatureGeneratorPtr signedNumberFeatureGenerator(FeatureGeneratorPtr positiveNumberFeatures);

// atomic - number features presets
extern FeatureGeneratorPtr defaultPositiveIntegerFeatureGenerator(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern FeatureGeneratorPtr defaultIntegerFeatureGenerator(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern FeatureGeneratorPtr defaultDoubleFeatureGenerator(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern FeatureGeneratorPtr defaultPositiveDoubleFeatureGenerator(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern FeatureGeneratorPtr defaultProbabilityFeatureGenerator(size_t numIntervals = 5);

// generic
extern FeatureGeneratorPtr containerWindowFeatureGenerator(size_t windowSize);
extern FeatureGeneratorPtr matrixWindowFeatureGenerator(size_t windowRows, size_t windowColumns);

extern FunctionPtr concatenateFeatureGenerator(bool lazy);
extern FeatureGeneratorPtr concatenateDoubleFeatureGenerator(bool lazy);
extern FeatureGeneratorPtr concatenateDoubleVectorFeatureGenerator(bool lazy);

extern FeatureGeneratorPtr cartesianProductFeatureGenerator(bool lazy = true);

// composite
extern CompositeFunctionPtr enumerationDistributionFeatureGenerator(size_t probabilityDiscretization = 1, size_t entropyDiscretization = 10, double minEntropy = -1.0, double maxEntropy = 4.0);

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_

/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/FeatureGenerator.h>
using namespace lbcpp;

class FillSparseVectorFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  FillSparseVectorFeatureGeneratorCallback(const SparseDoubleVectorPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    jassert(weight == 1.0);
    vector->appendTo(target, index);
  }

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
  {
    jassert(weight == 1.0);
    featureGenerator->appendTo(inputs, target, index);
  }

private:
  SparseDoubleVectorPtr target;
};

TypePtr FeatureGenerator::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  featuresType = doubleType;
  featuresEnumeration = initializeFeatures(context, inputVariables, featuresType, outputName, outputShortName);
  jassert(featuresEnumeration && featuresType);
  return lazyComputation ? getLazyOutputType(featuresEnumeration, featuresType) : getNonLazyOutputType(featuresEnumeration, featuresType);
}

DoubleVectorPtr FeatureGenerator::toLazyVector(const Variable* inputs) const
  {return new LazyDoubleVector(refCountedPointerFromThis(this), inputs);}

DoubleVectorPtr FeatureGenerator::toComputedVector(const Variable* inputs) const
{
  SparseDoubleVectorPtr res(new SparseDoubleVector(getOutputType()));
  FillSparseVectorFeatureGeneratorCallback callback(res);
  computeFeatures(inputs, callback);
  return res;
}

Variable FeatureGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  if (lazyComputation)
    return toLazyVector(inputs);
  else
    return toComputedVector(inputs);
}

class ComputeL0NormFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeL0NormFeatureGeneratorCallback() : res(0) {}

  virtual void sense(size_t index, double value)
    {if (value) ++res;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {if (weight) res += vector->l0norm();}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {if (weight) res += featureGenerator->l0norm(inputs);}

  size_t res;
};

size_t FeatureGenerator::l0norm(const Variable* inputs) const
{
  ComputeL0NormFeatureGeneratorCallback callback;
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

class AppendToFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AppendToFeatureGeneratorCallback(const SparseDoubleVectorPtr& target, size_t offsetInSparseVector)
    : target(target), offset(offsetInSparseVector) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(offset + index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {jassert(weight == 1.0); target->appendTo(target, offset + index);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {jassert(weight == 1.0); featureGenerator->appendTo(inputs, target, offset + index);}

protected:
  SparseDoubleVectorPtr target;
  size_t offset;
};

void FeatureGenerator::appendTo(const Variable* inputs, const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const
{
  AppendToFeatureGeneratorCallback callback(sparseVector, offsetInSparseVector);
  computeFeatures(&inputs[0], callback);
}

class AddWeightedToFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AddWeightedToFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, size_t offsetInSparseVector, double weight)
    : target(target), offset(offsetInSparseVector), weight(weight) {}

  virtual void sense(size_t index, double value)
    {target->getValueReference(index + offset) += value * weight;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {vector->addWeightedTo(target, index + offset, weight * this->weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->addWeightedTo(inputs, target, offset + index, weight * this->weight);}

protected:
  DenseDoubleVectorPtr target;
  size_t offset;
  double weight;
};

void FeatureGenerator::addWeightedTo(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  if (weight)
  {
    AddWeightedToFeatureGeneratorCallback callback(denseVector, offsetInDenseVector, weight);
    computeFeatures(inputs, callback);
  }
}

class DotProductFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  DotProductFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, size_t offsetInSparseVector)
    : target(target), offset(offsetInSparseVector), res(0.0) {}

  virtual void sense(size_t index, double value)
    {res += target->getValue(index + offset) * value;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {res += weight * vector->dotProduct(target, index + offset);}
 
  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {res += weight * featureGenerator->dotProduct(inputs, target, index + offset);}
 
  double res;

protected:
  DenseDoubleVectorPtr target;
  size_t offset;
};

double FeatureGenerator::dotProduct(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  DotProductFeatureGeneratorCallback callback(denseVector, offsetInDenseVector);
  computeFeatures(inputs, callback);
  return callback.res;
}

/*
** Number Feature Generators
*/
FeatureGeneratorPtr lbcpp::defaultPositiveIntegerFeatureGenerator(size_t numIntervals, double maxPowerOfTen)
  {return softDiscretizedLogNumberFeatureGenerator(0.0, maxPowerOfTen, numIntervals, true);}

FeatureGeneratorPtr lbcpp::defaultIntegerFeatureGenerator(size_t numIntervals, double maxPowerOfTen)
  {return signedNumberFeatureGenerator(softDiscretizedLogNumberFeatureGenerator(0.0, maxPowerOfTen, numIntervals, true));}

FeatureGeneratorPtr lbcpp::defaultProbabilityFeatureGenerator(size_t numIntervals)
  {return softDiscretizedNumberFeatureGenerator(0.0, 1.0, numIntervals, false, false);}

FeatureGeneratorPtr lbcpp::defaultPositiveDoubleFeatureGenerator(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return softDiscretizedLogNumberFeatureGenerator(minPowerOfTen, maxPowerOfTen, numIntervals, true);}

FeatureGeneratorPtr lbcpp::defaultDoubleFeatureGenerator(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return signedNumberFeatureGenerator(defaultPositiveDoubleFeatureGenerator(numIntervals, minPowerOfTen, maxPowerOfTen));}

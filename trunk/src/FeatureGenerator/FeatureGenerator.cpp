/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/FeatureGenerator/FeatureGenerator.h>
#include "FeatureGeneratorCallbacks.hpp"
using namespace lbcpp;

FeatureGenerator::FeatureGenerator(bool lazy)
  : lazyComputation(lazy), sparse(true), sparseVectorSizeUpperBound(10)
{
}

TypePtr FeatureGenerator::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  featuresType = doubleType;
  featuresEnumeration = initializeFeatures(context, inputVariables, featuresType, outputName, outputShortName);
  if (!featuresEnumeration || !featuresType)
    return TypePtr();
  sparse = isSparse();
  lazyOutputType = getLazyOutputType(featuresEnumeration, featuresType);
  nonLazyOutputType = getNonLazyOutputType(featuresEnumeration, featuresType);
  return lazyComputation ? lazyOutputType : nonLazyOutputType;
}

DoubleVectorPtr FeatureGenerator::toLazyVector(const Variable* inputs) const
  {return new LazyDoubleVector(lazyOutputType, refCountedPointerFromThis(this), inputs);}

void FeatureGenerator::pushSparseVectorSize(size_t size)
{
  int delta = (int)size - (int)sparseVectorSizeUpperBound;
  if (delta > 10)
    sparseVectorSizeUpperBound += 3 * delta / 4;
  else if (delta > 0)
    sparseVectorSizeUpperBound += 10;
  else if (3 * sparseVectorSizeUpperBound / 4 > size)
    --sparseVectorSizeUpperBound;
}

SparseDoubleVectorPtr FeatureGenerator::createEmptySparseVector() const
{
  SparseDoubleVectorPtr res(new SparseDoubleVector(nonLazyOutputType));
  size_t s = sparseVectorSizeUpperBound;
  if (s)
    res->reserveValues(s);
  return res;
}

DoubleVectorPtr FeatureGenerator::toComputedVector(const Variable* inputs) const
{
  if (sparse)
  {
    SparseDoubleVectorPtr res = createEmptySparseVector();
    FillSparseVectorFeatureGeneratorCallback callback(res);
    computeFeatures(inputs, callback);
    const_cast<FeatureGenerator* >(this)->pushSparseVectorSize(res->getNumValues());
    return res;
  }
  else
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(nonLazyOutputType);
    FillDenseVectorFeatureGeneratorCallback callback(res);
    computeFeatures(inputs, callback);
    return res;
  }
}

Variable FeatureGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
  {return lazyComputation ? toLazyVector(inputs) : toComputedVector(inputs);}

double FeatureGenerator::entropy(const Variable* inputs) const
{
  ComputeEntropyFeatureGeneratorCallback callback;
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

size_t FeatureGenerator::l0norm(const Variable* inputs) const
{
  ComputeL0NormFeatureGeneratorCallback callback;
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

double FeatureGenerator::l1norm(const Variable* inputs) const
{
  ComputeL1NormFeatureGeneratorCallback callback;
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

double FeatureGenerator::sumOfSquares(const Variable* inputs) const
{
  ComputeSumOfSquaresFeatureGeneratorCallback callback;
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

double FeatureGenerator::getExtremumValue(const Variable* inputs, bool lookForMaximum, size_t* index) const
{
  ComputeExtremumValueFeatureGeneratorCallback callback(lookForMaximum, index);
  computeFeatures(&inputs[0], callback);
  return callback.res;
}

void FeatureGenerator::appendTo(const Variable* inputs, const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  AppendToFeatureGeneratorCallback callback(sparseVector, offsetInSparseVector, weight);
  computeFeatures(&inputs[0], callback);
}

void FeatureGenerator::addWeightedTo(const Variable* inputs, const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (weight)
  {
    AddWeightedToSparseFeatureGeneratorCallback callback(sparseVector, offsetInSparseVector, weight);
    computeFeatures(inputs, callback);
  }
}

void FeatureGenerator::addWeightedTo(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  if (weight)
  {
    AddWeightedToDenseFeatureGeneratorCallback callback(denseVector, offsetInDenseVector, weight);
    computeFeatures(inputs, callback);
  }
}

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
{
  jassert(numIntervals);
  if (numIntervals == 1)
    return doubleFeatureGenerator();
  else
    return softDiscretizedNumberFeatureGenerator(0.0, 1.0, numIntervals, false, false);
}

FeatureGeneratorPtr lbcpp::defaultPositiveDoubleFeatureGenerator(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return softDiscretizedLogNumberFeatureGenerator(minPowerOfTen, maxPowerOfTen, numIntervals, true);}

FeatureGeneratorPtr lbcpp::defaultDoubleFeatureGenerator(size_t numIntervals, double minPowerOfTen, double maxPowerOfTen)
  {return signedNumberFeatureGenerator(defaultPositiveDoubleFeatureGenerator(numIntervals, minPowerOfTen, maxPowerOfTen));}
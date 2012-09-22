/*-----------------------------------------.---------------------------------.
| Filename: NearestNeighborFunction.cpp    | Nearest Neighbor                |
| Author  : Julien Becker                  |                                 |
| Started : 04/08/2011 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "NearestNeighborFunction.h"
#include "StreamBasedNearestNeighbor.h"
#include <lbcpp/Data/RandomVariable.h>
#include <lbcpp/FeatureGenerator/FeatureGenerator.h>

using namespace lbcpp;

Variable NearestNeighborFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ScoresMap scoredIndices;
  DenseDoubleVectorPtr baseVector = inputs[0].getObjectAndCast<DoubleVector>(context)->toDenseDoubleVector();
  jassert(baseVector);

  const size_t n = inputData.size();
  jassert(n && n == supervisionData.size());
  jassert(inputData[0]->getElementsEnumeration()->getNumElements() == baseVector->getElementsEnumeration()->getNumElements());
  for (size_t i = 0; i < n; ++i)
  {
    //const double score = baseVector->getDistanceTo(inputData[i]);
    const double score = baseVector->l2norm(inputData[i]->toSparseVector());
    scoredIndices.insert(std::pair<double, size_t>(-score, i));
  }

  if (!includeTheNearestNeighbor)
    scoredIndices.erase(scoredIndices.begin());

  return computeOutput(scoredIndices);
}

Variable BinaryNearestNeighbor::computeOutput(ScoresMap& scoredIndices) const
{
  size_t numTrues = 0;
  double sumOfScores = 0.0;
  double sumOfTrueScores = 0.0;
  const size_t maxNumNeighbors = scoredIndices.size() < numNeighbors ? scoredIndices.size() : numNeighbors;
  ScoresMap::reverse_iterator it = scoredIndices.rbegin();
  for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
  {
    const size_t index = it->second;
    const double score = it->first;
    jassert(index < supervisionData.size());
    const Variable v = supervisionData[index];
    if (v.isBoolean() && v.getBoolean() || v.isDouble() && v.getDouble() > 0.5)
    {
      ++numTrues;
      sumOfTrueScores += score;
    }
    sumOfScores += score;
  }
  if (useWeightedScore)
    return probability(sumOfScores == 0.0 ? 0.0 : sumOfTrueScores / sumOfScores);
  return probability(numTrues / (double)maxNumNeighbors);
}

Variable RegressionNearestNeighbor::computeOutput(ScoresMap& scoredIndices) const
{
  double sum = 0.0;
  const size_t maxNumNeighbors = scoredIndices.size() < numNeighbors ? scoredIndices.size() : numNeighbors;
  ScoresMap::reverse_iterator it = scoredIndices.rbegin();
  for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
    sum += supervisionData[it->second].getDouble();
  return Variable(maxNumNeighbors ? sum / (double)maxNumNeighbors : 0.f, doubleType);
}

Variable ClassificationNearestNeighbor::computeOutput(ScoresMap& scoredIndices) const
{
  std::vector<double> sums(enumeration->getNumElements(), 0.0);

  const size_t maxNumNeighbors = scoredIndices.size() < numNeighbors ? scoredIndices.size() : numNeighbors;
  ScoresMap::reverse_iterator it = scoredIndices.rbegin();
  for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
    for (size_t j = 0; j < sums.size(); ++j)
      sums[j] += supervisionData[it->second].getObjectAndCast<DoubleVector>()->getElement(j).getDouble();
  
  DenseDoubleVectorPtr res = new DenseDoubleVector(enumeration, probabilityType);
  for (size_t i = 0; i < sums.size(); ++i)
    res->setValue(i, maxNumNeighbors ? sums[i] / (double)maxNumNeighbors : 0.f);
  return res;
}

bool NearestNeighborBatchLearner::train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  NearestNeighborFunctionPtr nnFunction = function.staticCast<NearestNeighborFunction>();
  const size_t n = trainingData.size();
  jassert(n);
  const EnumerationPtr featuresEnumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>(context)->getElementsEnumeration();

  for (size_t i = 0; i < n; ++i)
  {
    jassert(trainingData[i]->getNumVariables() == 2);
    DoubleVectorPtr v = trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>(context);
    if (!v)
    {
      context.errorCallback(T("NearestNeighborBatchLearner::train"), T("Training example without DoubleVector as input data !"));
      return false;
    }
//    SparseDoubleVectorPtr inputVector = v->toSparseVector();
//    nnFunction->inputData.push_back(inputVector);
    nnFunction->inputData.push_back(v);
    nnFunction->supervisionData.push_back(trainingData[i]->getVariable(1));
  }

  return true;
}

/*
** Stream Based Nearest Neighbor
*/

namespace lbcpp
{

class NormalizedSquaredL2NormFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  NormalizedSquaredL2NormFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, const std::vector<double>& standardDeviation)
    : threshold(DBL_MAX), res(0.0), target(target), standardDeviation(standardDeviation), nextIndex(0), currentIndexOffset(0)
  {
    const size_t n = target->getNumElements();
    jassert(standardDeviation.size() == n);
    normalizedTarget.resize(n);
    squareTarget.resize(n);
    std::vector<double>& values = target->getValues();
    for (size_t i = 0; i < n; ++i)
    {
      normalizedTarget[i] = values[i] / standardDeviation[i];
      squareTarget[i] = normalizedTarget[i] * normalizedTarget[i];
    }
  }

  virtual void sense(size_t index, double value)
  {
    const size_t currentIndex = currentIndexOffset + index;
    jassert(nextIndex <= currentIndex && currentIndex < standardDeviation.size());
    for (; nextIndex != currentIndex && !shouldStop(); ++nextIndex)
      res += squareTarget[nextIndex];
    const double diff = normalizedTarget[currentIndex] - value / standardDeviation[currentIndex];
    res += diff * diff;
    ++nextIndex;
  }

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    jassert(weight == 1.f);
    currentIndexOffset += index;
    vector->computeFeatures(*this);
    currentIndexOffset -= index;
  }

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
  {
    jassert(weight == 1.f);
    currentIndexOffset += index;
    featureGenerator->computeFeatures(inputs, *this);
    currentIndexOffset -= index;
  }

  void finalize()
  {
    for (; nextIndex < standardDeviation.size() && !shouldStop(); ++nextIndex)
      res += squareTarget[nextIndex];
    jassert(currentIndexOffset == 0);
  }

  virtual bool shouldStop() const
    {return res > threshold;}

  double getDistance() const
    {return res;}

  void reset()
  {
    res = 0.f;
    nextIndex = 0;
    currentIndexOffset = 0;
  }

  double threshold;

protected:
  double res;

  DenseDoubleVectorPtr target;
  std::vector<double> normalizedTarget;
  std::vector<double> squareTarget;
  const std::vector<double>& standardDeviation;

  size_t nextIndex;
  size_t currentIndexOffset;
};

}; /* namespace lbcpp */


StreamBasedNearestNeighbor::StreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor)
  : stream(stream), numNeighbors(numNeighbors), includeTheNearestNeighbor(includeTheNearestNeighbor)
{
  ComputeMeanAndVarianceFeatureGeneratorCallback callback(stream->getElementsType()->getTemplateArgument(0)->getTemplateArgument(0).dynamicCast<Enumeration>());

  stream->rewind();
  while (!stream->isExhausted())
  {
    ObjectPtr obj = stream->next().getObject();
    jassert(obj);
    jassert(obj->getVariable(0).getObjectAndCast<LazyDoubleVector>()); // Method is efficient only if the double vector is lazy
    DoubleVectorPtr dv = obj->getVariable(0).getObjectAndCast<DoubleVector>();
    dv->computeFeatures(callback);
    callback.finalizeSense();
  }

  callback.computeStandardDeviation(standardDeviation);
}

StreamBasedNearestNeighbor::StreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor)
  : numNeighbors(numNeighbors), includeTheNearestNeighbor(includeTheNearestNeighbor)
  {setBatchLearner(streamBasedNearestNeighborBatchLearner());}

Variable StreamBasedNearestNeighbor::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ScoresMap scoresVariable;
  DenseDoubleVectorPtr baseVector = inputs[0].getObjectAndCast<DoubleVector>(context)->toDenseDoubleVector();
  jassert(baseVector);

  NormalizedSquaredL2NormFeatureGeneratorCallback featureCallback(baseVector, standardDeviation);

  StreamPtr clonedStream = stream->cloneAndCast<Stream>(context);
  clonedStream->rewind();
  size_t numToPrecompute = numNeighbors + 1;
  while (!clonedStream->isExhausted())
  {
    ObjectPtr obj = clonedStream->next().getObject();
    jassert(obj);
    DoubleVectorPtr dv = obj->getVariable(0).dynamicCast<DoubleVector>();
    featureCallback.reset();
    dv->computeFeatures(featureCallback);
    featureCallback.finalize();

    if (!includeTheNearestNeighbor && featureCallback.getDistance() < 1e-6) // Skip all neighbor with distance near to 0
      continue;

    if (featureCallback.shouldStop())
      continue;

    scoresVariable.insert(std::pair<double, Variable>(-featureCallback.getDistance(), obj->getVariable(1)));

    if (numToPrecompute > 0)
      --numToPrecompute;
    else
    {
      scoresVariable.erase(scoresVariable.begin());
      featureCallback.threshold = -scoresVariable.begin()->first;
    }
  }

  return computeOutput(scoresVariable);    
}

Variable ClassificationStreamBasedNearestNeighbor::computeOutput(ScoresMap& scoredIndices) const
{
  std::vector<double> sums(enumeration->getNumElements(), 0.0);

  const size_t maxNumNeighbors = scoredIndices.size() < numNeighbors ? scoredIndices.size() : numNeighbors;
  ScoresMap::reverse_iterator it = scoredIndices.rbegin();

  for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
    for (size_t j = 0; j < sums.size(); ++j)
      sums[j] += it->second.getObjectAndCast<DoubleVector>()->getElement(j).getDouble();

  DenseDoubleVectorPtr res = new DenseDoubleVector(enumeration, probabilityType);
  for (size_t i = 0; i < sums.size(); ++i)
    res->setValue(i, maxNumNeighbors ? sums[i] / (double)maxNumNeighbors : 0.f);
  return res;
}

Variable BinaryClassificationStreamBasedNearestNeighbor::computeOutput(ScoresMap& scoredIndices) const
{
  size_t numTrues = 0;
  const size_t maxNumNeighbors = scoredIndices.size() < numNeighbors ? scoredIndices.size() : numNeighbors;
  ScoresMap::reverse_iterator it = scoredIndices.rbegin();

  for (size_t i = 0; i < maxNumNeighbors; ++i, it++)
    if (it->second.isBoolean() && it->second.getBoolean()
        || it->second.isDouble() && it->second.getDouble() >= 0.5)
      ++numTrues;
  
  return probability(numTrues / (double)maxNumNeighbors);
}

bool StreamBasedNearestNeighborBatchLearner::train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  if (trainingData.size() == 0)
  {
    context.errorCallback(T("StreamBasedNearestNeighborBatchLearner::train"), T("No training data !"));
    return false;
  }

  StreamBasedNearestNeighborPtr target = function.staticCast<StreamBasedNearestNeighbor>();
  target->stream = objectStream(trainingData[0]->getClass(), trainingData);

  ComputeMeanAndVarianceFeatureGeneratorCallback callback(trainingData[0]->getVariable(0).getType()->getTemplateArgument(0).dynamicCast<Enumeration>());

  target->stream->rewind();
  while (!target->stream->isExhausted())
  {
    ObjectPtr obj = target->stream->next().getObject();
    jassert(obj);
    jassert(obj->getVariable(0).getObjectAndCast<LazyDoubleVector>()); // Method is efficient only if the double vector is lazy
    DoubleVectorPtr dv = obj->getVariable(0).getObjectAndCast<DoubleVector>();
    dv->computeFeatures(callback);
    callback.finalizeSense();
  }

  callback.computeStandardDeviation(target->standardDeviation);
  return true;
}


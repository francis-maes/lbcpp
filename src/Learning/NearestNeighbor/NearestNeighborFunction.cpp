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
    const double score = baseVector->getDistanceTo(inputData[i]->toSparseVector());
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

Variable StreamBasedNearestNeighbor::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ScoresMap scoresVariable;
  DenseDoubleVectorPtr baseVector = inputs[0].getObjectAndCast<DoubleVector>(context)->toDenseDoubleVector();
  jassert(baseVector);
  StreamPtr clonedStream = stream->cloneAndCast<Stream>(context);
  clonedStream->rewind();
  while (!clonedStream->isExhausted())
  {
    ObjectPtr obj = clonedStream->next().getObject();
    jassert(obj);
    SparseDoubleVectorPtr sdv = obj->getVariable(0).dynamicCast<SparseDoubleVector>();
    jassert(sdv->getElementsEnumeration() == baseVector->getElementsEnumeration());
    Variable supervision = obj->getVariable(1);
    const double score = baseVector->getDistanceTo(sdv);
    scoresVariable.insert(std::pair<double, Variable>(-score, supervision));
  }

  if (!includeTheNearestNeighbor)
    scoresVariable.erase(scoresVariable.begin());

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


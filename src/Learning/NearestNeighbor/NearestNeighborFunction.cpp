/*-----------------------------------------.---------------------------------.
| Filename: NearestNeighborFunction.cpp    | Nearest Neighbor                |
| Author  : Julien Becker                  |                                 |
| Started : 04/08/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "NearestNeighborFunction.h"
using namespace lbcpp;

Variable NearestNeighborFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ScoresMap scoredIndices;
  DenseDoubleVectorPtr baseVector = inputs[0].getObjectAndCast<DoubleVector>(context)->toDenseDoubleVector();
  jassert(baseVector);
  
  const size_t n = inputData.size();
  jassert(n && n == supervisionData.size());
  for (size_t i = 0; i < n; ++i)
  {
    const double score = baseVector->getDistanceTo(inputData[i]);
    scoredIndices.insert(std::pair<double, size_t>(-score, i));
  }

  return computeOuput(scoredIndices);
}

Variable BinaryNearestNeighborFunction::computeOuput(ScoresMap& scoredIndices) const
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

bool NearestNeighborBatchLearner::train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  NearestNeighborFunctionPtr nnFunction = function.staticCast<NearestNeighborFunction>();
  const size_t n = trainingData.size();
  jassert(n);
  for (size_t i = 0; i < n; ++i)
  {
    jassert(trainingData[i]->getNumVariables() == 2);
    DoubleVectorPtr v = trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>(context);
    if (!v)
    {
      context.errorCallback(T("NearestNeighborBatchLearner::train"), T("Training example without DoubleVector as input data !"));
      return false;
    }
    nnFunction->inputData.push_back(v->toSparseVector());
    nnFunction->supervisionData.push_back(trainingData[i]->getVariable(1));
  }
  return true;
}

namespace lbcpp
{

FunctionPtr binaryNearestNeighbor(size_t numNeighbors, bool useWeightedScore)
  {return new BinaryNearestNeighborFunction(numNeighbors, useWeightedScore);}

};
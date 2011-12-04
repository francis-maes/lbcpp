/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.cpp             | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeInference.h"
using namespace lbcpp;

LuapeInference::LuapeInference()
  : universe(new LuapeNodeUniverse())
{
}

LuapeSamplesCachePtr LuapeInference::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t n = data.size();
  LuapeSamplesCachePtr res = new LuapeSamplesCache(inputs, n);
  for (size_t i = 0; i < n; ++i)
  {
    const PairPtr& example = data[i].staticCast<Pair>();
    res->setInputObject(inputs, i, example->getFirst().getObject());
  }
  return res;
}

Variable LuapeInference::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  return computeNode(context, inputs[0].getObject());
}

Variable LuapeInference::computeNode(ExecutionContext& context, const ObjectPtr& inputObject) const
{
  LuapeInstanceCachePtr cache = new LuapeInstanceCache();
  cache->setInputObject(inputs, inputObject);
  return cache->compute(context, node);
}

/*
** LuapeRegressor
*/
size_t LuapeRegressor::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRegressor::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? doubleType : (TypePtr)objectClass;}

TypePtr LuapeRegressor::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return doubleType;
}

double LuapeRegressor::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == data.size());
  double res = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = pred->getValue(i) - data[i].staticCast<Pair>()->getSecond().getDouble();
    res += delta * delta;
  }
  return sqrt(res / (double)n);
}

/*
** LuapeBinaryClassifier
*/
size_t LuapeBinaryClassifier::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeBinaryClassifier::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? booleanType : (TypePtr)objectClass;}

TypePtr LuapeBinaryClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return booleanType;
}

Variable LuapeBinaryClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  double activation = computeNode(context, inputs[0].getObject()).getDouble();
  return activation > 0;
}

double LuapeBinaryClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  jassert(false); // not yet implemented
  return 0.0;
}

/*
** LuapeClassifier
*/
size_t LuapeClassifier::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeClassifier::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? enumValueType : (TypePtr)objectClass;}

TypePtr LuapeClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  EnumerationPtr enumeration = inputVariables[1]->getType().dynamicCast<Enumeration>();
  jassert(enumeration);
  doubleVectorClass = denseDoubleVectorClass(enumeration, doubleType);
  node = new LuapeVectorSumNode(enumeration);
  return enumeration;
}

DenseDoubleVectorPtr LuapeClassifier::computeActivations(ExecutionContext& context, const ObjectPtr& input) const
  {return computeNode(context, input).getObjectAndCast<DenseDoubleVector>();}

Variable LuapeClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  DenseDoubleVectorPtr activations = computeActivations(context, inputs[0].getObject());
  return Variable(activations->getIndexOfMaximumValue(), getOutputType());
}

double LuapeClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
  size_t numErrors = 0;
  size_t n = pred->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
    if (j != (size_t)data[i].staticCast<Pair>()->getSecond().getInteger())
      ++numErrors;
  }
  return numErrors / (double)n;
}

/*
** LuapeRanker
*/
size_t LuapeRanker::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRanker::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? simpleDenseDoubleVectorClass : vectorClass();}

TypePtr LuapeRanker::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return simpleDenseDoubleVectorClass;
}

Variable LuapeRanker::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  ObjectVectorPtr alternatives = inputs[0].getObjectAndCast<ObjectVector>();
  size_t n = alternatives->getNumElements();
  DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr alternative = alternatives->getElement(i).getObject();
    double score = computeNode(context, alternative).getDouble();
    scores->setValue(i, score);
  }
  return scores;
}

LuapeSamplesCachePtr LuapeRanker::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t numSamples = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    numSamples += alternatives->getNumElements();
  }
  
  LuapeSamplesCachePtr res = new LuapeSamplesCache(inputs, numSamples);
  size_t index = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();
    for (size_t j = 0; j < n; ++j)
      res->setInputObject(inputs, index++, alternatives->getElement(j).getObject());
  }
  return res;
}

double LuapeRanker::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  jassert(false); // not yet implemented
  return 0.0;
}

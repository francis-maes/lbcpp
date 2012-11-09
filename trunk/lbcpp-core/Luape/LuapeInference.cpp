/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.cpp             | LuapeInference                  |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeInference.h>
#include <lbcpp/Luape/LuapeLearner.h>
#include <lbcpp-ml/PostfixExpression.h>
#include "NodeBuilder/NodeBuilderDecisionProblem.h"
using namespace lbcpp;

/*
** LuapeInference
*/
LuapeSamplesCachePtr LuapeInference::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t n = data.size();
  LuapeSamplesCachePtr res = createCache(n, 512); // default: 512 Mb cache
  VectorPtr supervisionValues = vector(data[0]->getVariableType(1), n);
  for (size_t i = 0; i < n; ++i)
  {
    res->setInputObject(inputs, i, data[i]->getVariable(0).getObject());
    supervisionValues->setElement(i, data[i]->getVariable(1));
  }
  res->cacheNode(context, supervision, supervisionValues, T("Supervision"), false);
  res->recomputeCacheSize();
  return res;
}

void LuapeInference::setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
{
  createSupervision(trainingData[0]->getVariableType(1), T("supervision"));
  trainingCache = createSamplesCache(context, trainingData);
  if (validationData.size())
    validationCache = createSamplesCache(context, validationData);
}

std::vector<LuapeSamplesCachePtr> LuapeInference::getSamplesCaches() const
{
  std::vector<LuapeSamplesCachePtr> res;
  res.push_back(trainingCache);
  if (validationCache)
    res.push_back(validationCache);
  return res;
}

VectorPtr LuapeInference::getTrainingPredictions() const
{
  jassert(trainingCache->isNodeDefinitivelyCached(node));
  return trainingCache->getNodeCache(node);
}

VectorPtr LuapeInference::getTrainingSupervisions() const
{
  jassert(trainingCache->isNodeDefinitivelyCached(supervision));
  return trainingCache->getNodeCache(supervision);
}

VectorPtr LuapeInference::getValidationPredictions() const
{
  if (!validationCache)
    return VectorPtr();
  jassert(validationCache->isNodeDefinitivelyCached(node));
  return validationCache->getNodeCache(node);
}

VectorPtr LuapeInference::getValidationSupervisions() const
{
  if (!validationCache)
    return VectorPtr();
  jassert(validationCache->isNodeDefinitivelyCached(supervision));
  return validationCache->getNodeCache(supervision);
}

/*
** Rooot node
*/
Variable LuapeInference::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  return computeNode(context, inputs[0].getObject());
}

Variable LuapeInference::computeNode(ExecutionContext& context, const ObjectPtr& inputObject) const
{
  // broken
  jassertfalse;
  return Variable();
}

/*void ExpressionDomain::setLearner(const LuapeLearnerPtr& learner, bool verbose)
{
  learner->setVerbose(verbose);
  //setBatchLearner(new LuapeBatchLearner(learner));
}*/

void LuapeInference::setRootNode(ExecutionContext& context, const ExpressionPtr& node)
{
  if (node != this->node)
  {
    if (this->node)
    {
      if (trainingCache)
        trainingCache->uncacheNode(context, this->node);
      if (validationCache)
        validationCache->uncacheNode(context, this->node);
    }
    this->node = node;
    if (this->node)
    {
      if (trainingCache)
        trainingCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
      if (validationCache)
        validationCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
    }
  }
}

void LuapeInference::clearRootNode(ExecutionContext& context)
  {if (node) setRootNode(context, ExpressionPtr());}


/*
** LuapeRegressor
*/
size_t LuapeRegressor::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRegressor::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? doubleType : (TypePtr)objectClass;}

TypePtr LuapeRegressor::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  return doubleType;
}

Variable LuapeRegressor::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  Variable res = computeNode(context, inputs[0].getObject());
  return res.toDouble();
}

double LuapeRegressor::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  // compute RMSE
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  const DenseDoubleVectorPtr& sup = supervisions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == sup->getNumValues());
  double res = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = pred->getValue(i) - sup->getValue(i);
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
  {return index ? sumType(booleanType, probabilityType) : (TypePtr)objectClass;}

TypePtr LuapeBinaryClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  return probabilityType;
}

Variable LuapeBinaryClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  Variable res = computeNode(context, inputs[0].getObject());
  if (res.isBoolean())
    return Variable(res.getBoolean() ? 1.0 : 0.0, probabilityType);
  else if (res.getType() == probabilityType)
    return res;
  else
  {
    double activation = res.getDouble();
    jassert(activation != doubleMissingValue);
    return Variable(1.0 / (1.0 + exp(-activation)), probabilityType);
  }
}

double LuapeBinaryClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  size_t n = predictions->getNumElements();
  jassert(n == supervisions->getNumElements());

  size_t numErrors = 0;

  if (predictions.isInstanceOf<DenseDoubleVector>())
  {
    const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
    TypePtr elementsType = pred->getElementsType();
    for (size_t i = 0; i < n; ++i)
    {
      bool prediction;
      if (elementsType == probabilityType)
        prediction = pred->getValue(i) > 0.5;
      else
        prediction = pred->getValue(i) > 0.0;
      Variable supervision = supervisions->getElement(i);
      bool correct;
      if (!lbcpp::convertSupervisionVariableToBoolean(supervision, correct))
        jassert(false);
      if (prediction != correct)
        ++numErrors;
    }
  }
  else if (predictions.isInstanceOf<BooleanVector>())
  {
    const BooleanVectorPtr& pred = predictions.staticCast<BooleanVector>();
    for (size_t i = 0; i < n; ++i)
    {
      bool predicted = pred->get(i);
      Variable supervision = supervisions->getElement(i);
      bool correct;
      if (!lbcpp::convertSupervisionVariableToBoolean(supervision, correct))
        jassert(false);
      if (predicted != correct)
        ++numErrors;
    }
  }
  else
    jassert(false);
  return numErrors / (double)n;
}

/*
** LuapeClassifier
*/
size_t LuapeClassifier::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeClassifier::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? sumType(enumValueType, lbcpp::doubleVectorClass()) : (TypePtr)objectClass;}

EnumerationPtr LuapeClassifier::getLabelsFromSupervision(TypePtr supervisionType)
{
  return supervisionType.isInstanceOf<Enumeration>()
    ? supervisionType.staticCast<Enumeration>() : DoubleVector::getElementsEnumeration(supervisionType);
}

TypePtr LuapeClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  labels = getLabelsFromSupervision(inputVariables[1]->getType());
  jassert(labels);
  doubleVectorClass = denseDoubleVectorClass(labels, doubleType);
  return denseDoubleVectorClass(labels, probabilityType);
}

DenseDoubleVectorPtr LuapeClassifier::computeActivations(ExecutionContext& context, const ObjectPtr& input) const
  {return computeNode(context, input).getObjectAndCast<DenseDoubleVector>();}

Variable LuapeClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // normalize activations to make a probability distribution
  DenseDoubleVectorPtr activations = computeActivations(context, inputs[0].getObject());
  DenseDoubleVectorPtr res = new DenseDoubleVector(doubleVectorClass);
  double Z = activations->l1norm();
  if (Z)
  {
    size_t n = activations->getNumValues();
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, activations->getValue(i) / Z);
  }
  return res;
}

double LuapeClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
  if (!pred)
    return DBL_MAX;

  size_t n = pred->getNumElements();

  if (supervisions->getElementsType() == denseDoubleVectorClass(labels, doubleType))
  {
    const ObjectVectorPtr& sup = supervisions.staticCast<ObjectVector>();
    double totalCost = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
      double cost = sup->get(i).staticCast<DenseDoubleVector>()->getValue(j);
      totalCost += cost;
    }
    return totalCost / (double)n;    
  }
  else
  {
    size_t numErrors = 0;
    for (size_t i = 0; i < n; ++i)
    {
      size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
      Variable supervision = supervisions->getElement(i);
      size_t correctClass;
      if (lbcpp::convertSupervisionVariableToEnumValue(supervision, correctClass))
      {
        if (j != correctClass)
          ++numErrors;
      }
    }
    return numErrors / (double)n;
  }
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

LuapeSamplesCachePtr LuapeRanker::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data, std::vector<size_t>& exampleSizes) const
{
  size_t numSamples = 0;
  
  exampleSizes.resize(data.size());
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();;
    exampleSizes[i] = n;
    numSamples += n;
  }
  
  LuapeSamplesCachePtr res = new LuapeSamplesCache(universe, inputs, numSamples);
  DenseDoubleVectorPtr supervisionValues = new DenseDoubleVector(numSamples, 0.0);
  size_t index = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const ObjectPtr& rankingExample = data[i];
    const ContainerPtr& alternatives = rankingExample->getVariable(0).getObjectAndCast<Container>();
    const DenseDoubleVectorPtr& costs = rankingExample->getVariable(1).getObjectAndCast<DenseDoubleVector>();
    size_t n = exampleSizes[i];
    for (size_t j = 0; j < n; ++j)
    {
      res->setInputObject(inputs, index, alternatives->getElement(j).getObject());
      supervisionValues->setValue(index, costs->getValue(j));
      ++index;
    }
  }
  res->cacheNode(context, supervision, supervisionValues, T("Supervision"), false);
  res->recomputeCacheSize();
  return res;
}

void LuapeRanker::setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
{
  supervision = new VariableExpression(trainingData[0]->getVariableType(1), T("supervision"), inputs.size());
  trainingCache = createSamplesCache(context, trainingData, trainingExampleSizes);
  trainingCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
  if (validationData.size())
  {
    validationCache = createSamplesCache(context, validationData, validationExampleSizes);
    validationCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
  }
}
double LuapeRanker::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  jassert(false); // not yet implemented
  return 0.0;
}

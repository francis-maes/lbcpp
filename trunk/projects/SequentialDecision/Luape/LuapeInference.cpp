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

/*
** LuapeInference
*/
VectorPtr LuapeInference::makeCachedPredictions(ExecutionContext& context, bool isTrainingSamples) const
{
  size_t numSamples = graph->getNumSamples(isTrainingSamples);
  VectorPtr predictions = vector(getPredictionsInternalType(), numSamples);

  size_t yieldIndex = 0;
  for (size_t i = 0; i < graph->getNumNodes(); ++i)
  {
    LuapeYieldNodePtr yieldNode = graph->getNode(i).dynamicCast<LuapeYieldNode>();
    if (yieldNode)
    {
      LuapeNodePtr weakNode = yieldNode->getArgument();
      VectorPtr weakPredictions = graph->updateNodeCache(context, weakNode, isTrainingSamples);
      updatePredictions(predictions, yieldIndex, weakPredictions);
      ++yieldIndex;
    }
  }
  return predictions;
}

void LuapeInference::setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  size_t n = data.size();
  graph->resizeSamples(isTrainingData, n);
  for (size_t i = 0; i < n; ++i)
  {
    const PairPtr& example = data[i].staticCast<Pair>();
    graph->setSample(isTrainingData, i, example->getFirst().getObject());
  }
}

struct ComputeWeakPredictionsCallback : public LuapeGraphCallback
{
  ComputeWeakPredictionsCallback(DenseDoubleVectorPtr res) : res(res), index(0) {}

  DenseDoubleVectorPtr res;
  size_t index;

  virtual void valueYielded(const Variable& value)
  {
    double v;
    if (value.isBoolean())
      v = value.getBoolean() ? 1.0 : -1.0;
    else if (value.getType() == probabilityType)
      v = value.getDouble() * 2.0 - 1.0;
    else
      jassert(false);
    res->setValue(index++, v);
  }
};

DenseDoubleVectorPtr LuapeInference::computeSignedWeakPredictions(ExecutionContext& context, const ObjectPtr& input) const
{
  DenseDoubleVectorPtr res = new DenseDoubleVector(graph->getNumYieldNodes(), 0.0);
  ComputeWeakPredictionsCallback callback(res);
  std::vector<Variable> state;
  computeGraph(context, input, state, &callback);
  return res;
}

void LuapeInference::computeGraph(ExecutionContext& context, const ObjectPtr& inputs, std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  size_t n;
  state.resize(graph->getNumNodes());
  if (inputs->getClass().isInstanceOf<DynamicClass>())
  {
    n = inputs->getNumVariables();
    jassert(n <= state.size());
    for (size_t i = 0; i < n; ++i)
      state[i] = inputs->getVariable(i);
  }
  else
  {
    ContainerPtr container = inputs.dynamicCast<Container>();
    jassert(container);
    n = container->getNumElements();
    jassert(n <= state.size());
    for (size_t i = 0; i < n; ++i)
      state[i] = container->getElement(i);
  }
  graph->compute(context, state, n, callback);
}

/*
** LuapeRegressor
*/
size_t LuapeRegressor::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRegressor::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? doubleType : (TypePtr)objectClass;}

TypePtr LuapeRegressor::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {return doubleType;}

struct LuapeRegressorComputeCallback : public LuapeGraphCallback
{
  LuapeRegressorComputeCallback(DenseDoubleVectorPtr votes) 
    : votes(votes), index(0), res(0.0) {}

  DenseDoubleVectorPtr votes;
  size_t index;
  double res;

  virtual void valueYielded(const Variable& value)
  {
    jassert(value.isBoolean());
    res += votes->getValue(value.getBoolean() ? index + 1 : index);
  }
};

Variable LuapeRegressor::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  LuapeRegressorComputeCallback callback(votes);   
  std::vector<Variable> state;
  computeGraph(context, inputs[0].getObject(), state, &callback);
  return callback.res;
}

VectorPtr LuapeRegressor::createVoteVector() const
  {return new DenseDoubleVector(0, 0.0);}

void LuapeRegressor::updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const
{
  double negativeVote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex * 2);
  double positiveVote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex * 2 + 1);
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();

  BooleanVectorPtr yieldBooleans = yieldOutputs.dynamicCast<BooleanVector>();
  if (yieldBooleans)
  {
    std::vector<bool>::const_iterator it = yieldBooleans->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      pred->incrementValue(i, *it++ ? positiveVote : negativeVote);
  }
  else
  {
    DenseDoubleVectorPtr yieldScalars = yieldOutputs.dynamicCast<DenseDoubleVector>();
    jassert(yieldScalars);
    for (size_t i = 0; i < n; ++i)
    {
      double weak = yieldScalars->getValue(i) * 2.0 - 1.0;
      pred->incrementValue(i, (weak > 0 ? positiveVote : negativeVote) * weak); // !!
    }
  }
}

void LuapeRegressor::setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  graph->clearSamples(isTrainingData, !isTrainingData);
  LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
  inputNodeCache->resizeSamples(isTrainingData, data.size());
  DenseDoubleVectorPtr supervisions = new DenseDoubleVector(data.size(), 0.0);
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& example = data[i].staticCast<Pair>();
    inputNodeCache->setSample(isTrainingData, i, example->getFirst());
    supervisions->setValue(i, example->getSecond().getDouble());
  }
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
  {return booleanType;}

struct LuapeBinaryClassifierComputeCallback : public LuapeGraphCallback
{
  LuapeBinaryClassifierComputeCallback(VectorPtr votes) 
    : votes(votes), index(0), res(0.0) {}

  VectorPtr votes;
  size_t index;
  double res;

  virtual void valueYielded(const Variable& value)
    {res += votes->getElement(index++).getDouble() * (value.getBoolean() ? 1.0 : -1.0);}
};

Variable LuapeBinaryClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]

  LuapeBinaryClassifierComputeCallback callback(votes);   
  std::vector<Variable> state;
  computeGraph(context, inputs[0].getObject(), state, &callback);
  return callback.res > 0;
}

VectorPtr LuapeBinaryClassifier::createVoteVector() const
  {return new DenseDoubleVector(0, 0.0);}

void LuapeBinaryClassifier::updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const
{
  double vote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex);
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();

  BooleanVectorPtr yieldBooleans = yieldOutputs.dynamicCast<BooleanVector>();
  if (yieldBooleans)
  {
    std::vector<bool>::const_iterator it = yieldBooleans->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      pred->incrementValue(i, vote * (*it++ ? 1.0 : -1.0));
  }
  else
  {
    DenseDoubleVectorPtr yieldScalars = yieldOutputs.dynamicCast<DenseDoubleVector>();
    jassert(yieldScalars);
    for (size_t i = 0; i < n; ++i)
      pred->incrementValue(i, vote * (yieldScalars->getValue(i) * 2.0 - 1.0));
  }
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
  return enumeration;
}

struct LuapeClassifierComputeCallback : public LuapeGraphCallback
{
  LuapeClassifierComputeCallback(VectorPtr votes, ClassPtr doubleVectorClass)
    : votes(votes), index(0), res(new DenseDoubleVector(doubleVectorClass)) {}

  VectorPtr votes;
  size_t index;
  DenseDoubleVectorPtr res;

  virtual void valueYielded(const Variable& value)
  {
    DenseDoubleVectorPtr vote = votes->getElement(index++).getObjectAndCast<DenseDoubleVector>();
    if (value.isBoolean())
      vote->addWeightedTo(res, 0, value.getBoolean() ? 1.0 : -1.0);
    else if (value.getType() == probabilityType)
      vote->addWeightedTo(res, 0, value.getDouble() * 2.0 - 1.0);
    else
      jassert(false);
  }
};

DenseDoubleVectorPtr LuapeClassifier::computeActivations(ExecutionContext& context, const ObjectPtr& input) const
{
  LuapeClassifierComputeCallback callback(votes, doubleVectorClass);   
  std::vector<Variable> state;
  computeGraph(context, input, state, &callback);
  return callback.res;
}

Variable LuapeClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  DenseDoubleVectorPtr activations = computeActivations(context, inputs[0].getObject());
  return Variable(activations->getIndexOfMaximumValue(), getOutputType());
}

VectorPtr LuapeClassifier::createVoteVector() const
  {return new ObjectVector(doubleVectorClass, 0);}

TypePtr LuapeClassifier::getPredictionsInternalType() const
  {return doubleVectorClass;}

void LuapeClassifier::updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const
{
  DenseDoubleVectorPtr vote = votes->getElement(yieldIndex).getObjectAndCast<DenseDoubleVector>();
  const ObjectVectorPtr& pred = predictions.staticCast<ObjectVector>();
  size_t n = pred->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    if (!pred->get(i))
      pred->set(i, new DenseDoubleVector(doubleVectorClass));
  }

  BooleanVectorPtr yieldBooleans = yieldOutputs.dynamicCast<BooleanVector>();
  if (yieldBooleans)
  {
    std::vector<bool>::const_iterator it = yieldBooleans->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      vote->addWeightedTo(pred->getAndCast<DenseDoubleVector>(i), 0, *it++ ? 1.0 : -1.0);
  }
  else
  {
    DenseDoubleVectorPtr yieldScalars = yieldOutputs.dynamicCast<DenseDoubleVector>();
    jassert(yieldScalars);
    for (size_t i = 0; i < n; ++i)
      vote->addWeightedTo(pred->getAndCast<DenseDoubleVector>(i), 0, yieldScalars->getValue(i) * 2.0 - 1.0);
  }
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
  {return index ? denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType) : vectorClass();}

TypePtr LuapeRanker::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType);}

struct LuapeRankerComputeCallback : public LuapeGraphCallback
{
  LuapeRankerComputeCallback(DenseDoubleVectorPtr votes) 
    : votes(votes), index(0), res(0.0) {}

  DenseDoubleVectorPtr votes;
  size_t index;
  double res;

  virtual void valueYielded(const Variable& value)
  {
    jassert(value.isBoolean());
    res += votes->getValue(index++);
  }
};

Variable LuapeRanker::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  ObjectVectorPtr alternatives = inputs[0].getObjectAndCast<ObjectVector>();
  size_t n = alternatives->getNumElements();
  DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
  for (size_t i = 0; i < n; ++i)
  {
    LuapeRankerComputeCallback callback(votes);   
    std::vector<Variable> state;
    computeGraph(context, alternatives->get(i), state, &callback);
    scores->setValue(i, callback.res);
  }
  return scores;
}

// votes are scalars (alpha values)
VectorPtr LuapeRanker::createVoteVector() const
  {return new DenseDoubleVector(0, 0.0);}

void LuapeRanker::updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const
{
  jassert(false); // not yet implemented
}

void LuapeRanker::setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  graph->clearSamples(isTrainingData, !isTrainingData);
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();

    LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
    size_t firstIndex = inputNodeCache->getNumSamples(isTrainingData);
    inputNodeCache->resizeSamples(isTrainingData, firstIndex + n);
    for (size_t i = 0; i < n; ++i)
      inputNodeCache->setSample(true, firstIndex + 1, alternatives->getElement(i));
  }
}

double LuapeRanker::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  jassert(false); // not yet implemented
  return 0.0;
}

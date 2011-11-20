/*-----------------------------------------.---------------------------------.
| Filename: LuapeNode.cpp                  | Luape Graph Node                |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "LuapeNode.h"
#include <algorithm>
using namespace lbcpp;

/*
** LuapeFunction
*/
String LuapeFunction::toShortString(const std::vector<LuapeNodePtr>& inputs) const
{
  String res = getClass()->getShortName() + T("(");
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    res += inputs[i]->toShortString();
    if (i < inputs.size() - 1)
      res += T(", ");
  }
  return res + T(")");
}

Variable LuapeFunction::compute(ExecutionContext& context, const std::vector<LuapeNodePtr>& inputs, const std::vector<Variable>& state) const
{
  std::vector<Variable> inputValues(inputs.size());
  for (size_t i = 0; i < inputValues.size(); ++i)
    inputValues[i] = state[inputs[i]->getIndexInGraph()];
  return compute(context, &inputValues[0]);
}

VectorPtr LuapeFunction::compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
{
  jassert(inputs.size() == getNumInputs());
  jassert(inputs.size());

  size_t n = inputs[0]->getNumElements();
  VectorPtr res = vector(outputType, n);
  for (size_t i = 0; i < n; ++i)
  {
    std::vector<Variable> inputValues(inputs.size());
    for (size_t j = 0; j < inputValues.size(); ++j)
      inputValues[j] = inputs[j]->getElement(i);
    res->setElement(i, compute(context, &inputValues[0]));
  }
  
  /*
    cache->resizeSamples(isTrainingSamples, minCacheSize);
    std::vector<Variable> inputs(arguments.size());

    if (inputBaseType == doubleType)
    {
      std::vector<DenseDoubleVectorPtr> inputCaches(arguments.size());
      for (size_t i = 0; i < inputCaches.size(); ++i)
        inputCaches[i] = arguments[i]->getCache()->getSamples(isTrainingSamples).staticCast<DenseDoubleVector>();
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = inputCaches[j]->getValue(i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
    else if (inputBaseType == booleanType)
    {
      std::vector<BooleanVectorPtr> inputCaches(arguments.size());
      for (size_t i = 0; i < inputCaches.size(); ++i)
        inputCaches[i] = arguments[i]->getCache()->getSamples(isTrainingSamples).staticCast<BooleanVector>();
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = inputCaches[j]->get(i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
    else
    {
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = arguments[j]->getCache()->getSample(isTrainingSamples, i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
    */
  return res;
}

bool LuapeFunction::acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const
{
  size_t n = getNumInputs();
  if (n > stack.size())
    return false;
  size_t stackFirstIndex = stack.size() - n;
  for (size_t i = 0; i < n; ++i)
    if (!doAcceptInputType(i, stack[stackFirstIndex + i]->getType()))
      return false;

  if (n)
  {
    if (hasFlags(commutativeFlag))
    {
      LuapeNodePtr node = stack[stackFirstIndex];
      for (size_t i = 1; i < n; ++i)
      {
        LuapeNodePtr newNode = stack[stackFirstIndex + i];
        if (newNode < node)
          return false;
        node = newNode;
      }
    }
    if (hasFlags(allSameArgIrrelevantFlag))
    {
      bool ok = false;
      for (size_t i = 1; i < n; ++i)
        if (stack[stackFirstIndex + i] != stack[stackFirstIndex])
        {
          ok = true;
          break;
        }
      if (!ok)
        return false;
    }
  }
  return true;
}

/*
** LuapeNodeCache
*/
LuapeNodeCache::LuapeNodeCache() 
  : convertibleToDouble(false), scoreComputed(false), score(0.0)
{
}

void LuapeNodeCache::initialize(TypePtr type)
{
  trainingSamples = vector(type, 0);
  validationSamples = vector(type, 0);
  convertibleToDouble = type->isConvertibleToDouble();
}

void LuapeNodeCache::clear()
{
  scoreComputed = false;
  score = 0.0;
  TypePtr type = trainingSamples->getElementsType();
  initialize(type);
}

void LuapeNodeCache::resizeSamples(bool isTrainingSamples, size_t size)
  {(isTrainingSamples ? trainingSamples : validationSamples)->resize(size);}

void LuapeNodeCache::resizeSamples(size_t numTrainingSamples, size_t numValidationSamples)
{
  trainingSamples->resize(numTrainingSamples);
  validationSamples->resize(numValidationSamples);
}

void LuapeNodeCache::setSample(bool isTrainingSample, size_t index, const Variable& value)
  {(isTrainingSample ? trainingSamples : validationSamples)->setElement(index, value);}

void LuapeNodeCache::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  if (clearTrainingSamples)
  {
    trainingSamples->clear();
    sortedDoubleValues.clear();
  }
  if (clearValidationSamples)
    validationSamples->clear();
}

struct SortDoubleValuesOperator
{
  bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
    {return a.second == b.second ? a.first < b.first : a.second < b.second;}
};

const std::vector< std::pair<size_t, double> >& LuapeNodeCache::getSortedDoubleValues() const
{
  size_t n = trainingSamples->getNumElements();
  if (sortedDoubleValues.size() < n)
  {
    std::vector< std::pair<size_t, double> >& v = const_cast<LuapeNodeCache* >(this)->sortedDoubleValues;
    v.resize(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = std::make_pair(i, trainingSamples->getElement(i).toDouble());
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  }
  return sortedDoubleValues;
}

BinaryKeyPtr LuapeNodeCache::makeKeyFromSamples(bool useTrainingSamples) const
{
  ContainerPtr samples = getSamples(useTrainingSamples);
  size_t n = samples->getNumElements();
  
  BooleanVectorPtr booleanVector = samples.dynamicCast<BooleanVector>();
  if (booleanVector)
  {
    BinaryKeyPtr res = new BinaryKey(1 + n / 8);
    for (size_t i = 0; i < n; ++i)
      res->pushBit(booleanVector->get(i));
    res->fillBits();
    return res;
  }

  DenseDoubleVectorPtr doubleVector = samples.dynamicCast<DenseDoubleVector>();
  if (doubleVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * 4);
    for (size_t i = 0; i < n; ++i)
      res->push32BitInteger((int)(doubleVector->getValue(i) * 10e6));
    return res;
  }

  ObjectVectorPtr objectVector = samples.dynamicCast<ObjectVector>();
  if (objectVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * sizeof (void* ));
    for (size_t i = 0; i < n; ++i)
      res->pushPointer(objectVector->get(i));
    return res;
  }

  GenericVectorPtr genericVector = samples.dynamicCast<GenericVector>();
  if (genericVector)
  {
    if (genericVector->getElementsType()->inheritsFrom(integerType))
    {
      BinaryKeyPtr res = new BinaryKey(n * sizeof (VariableValue));
      res->pushBytes((unsigned char* )&genericVector->getValues()[0], n * sizeof (VariableValue));
      return res;
    }

    jassert(false);
  }

  jassert(false);
  return BinaryKeyPtr();
}

String LuapeNodeCache::toShortString() const
{
  size_t n = trainingSamples->getNumElements();
  if (n == 0)
    return "<no examples>";

  TypePtr elementsType = trainingSamples->getElementsType();
  if (elementsType == booleanType)
  {
    BooleanVectorPtr booleans = trainingSamples.staticCast<BooleanVector>();
    size_t countOfTrue = 0;
    for (size_t i = 0; i < n; ++i)
      if (booleans->get(i))
        ++countOfTrue;
    return String(countOfTrue) + T(" / ") + String((int)n);
  }
  else if (elementsType.isInstanceOf<Enumeration>())
  {
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(elementsType.staticCast<Enumeration>(), doubleType);
    double invZ = 1.0 / (double)n;
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = trainingSamples->getElement(i);
      if (!v.isMissingValue())
        probabilities->incrementValue(v.getInteger(), invZ);
    }
    return probabilities->toShortString();
  }
  else if (elementsType->isConvertibleToDouble())
  {
    ScalarVariableStatistics stats;
    for (size_t i = 0; i < n; ++i)
      stats.push(trainingSamples->getElement(i).toDouble());
    return stats.toShortString();
  }
  else
    return String((int)n);
}

/*
** LuapeNode
*/
LuapeNode::LuapeNode(const TypePtr& type, const String& name)
  : NameableObject(name), type(type), cache(new LuapeNodeCache()), indexInGraph((size_t)-1)
{
  cache->initialize(type);
}

void LuapeNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeNodePtr& target = t.staticCast<LuapeNode>();
  target->name = name;
  target->type = type;
  target->cache = cache;
}

/*
** LuapeInputNode
*/
LuapeInputNode::LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex)
  : LuapeNode(type, name), inputIndex(inputIndex) {}

String LuapeInputNode::toShortString() const
  {return getName();}

Variable LuapeInputNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {jassert(false); return Variable();}

void LuapeInputNode::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  target.staticCast<LuapeInputNode>()->inputIndex = inputIndex;
  LuapeNode::clone(context, target);
}

/*
** LuapeFunctionNode
*/
LuapeFunctionNode::LuapeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& arguments)
  : function(function), arguments(arguments)
{
  initialize();
}

LuapeFunctionNode::LuapeFunctionNode(const LuapeFunctionPtr& function, LuapeNodePtr argument)
  : function(function), arguments(1, argument)
{
  initialize();
}

String LuapeFunctionNode::toShortString() const
  {return function->toShortString(arguments);}

void LuapeFunctionNode::initialize()
{
  size_t numInputs = function->getNumInputs();
  jassert(arguments.size() == numInputs);
  std::vector<TypePtr> inputTypes(numInputs);
  for (size_t i = 0; i < numInputs; ++i)
  {
    inputTypes[i] = arguments[i]->getType();
    jassert(function->doAcceptInputType(i, inputTypes[i]));
  }

  type = function->getOutputType(inputTypes);
  name = function->toShortString(arguments);
  cache = new LuapeNodeCache();
  cache->initialize(type);
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {return function->compute(context, arguments, state);}

void LuapeFunctionNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  const LuapeFunctionNodePtr& target = t.staticCast<LuapeNode>();
  target->function = function;
  target->arguments = arguments;
}

size_t LuapeFunctionNode::getDepth() const
{
  size_t maxInputDepth = 0;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    size_t d = arguments[i]->getDepth();
    if (d > maxInputDepth)
      maxInputDepth = d;
  }
  return maxInputDepth + 1;
}

void LuapeFunctionNode::updateCache(ExecutionContext& context, bool isTrainingSamples)
{
  jassert(arguments.size());
  TypePtr inputBaseType = arguments[0]->getType();
  arguments[0]->updateCache(context, isTrainingSamples);
  size_t minCacheSize = arguments[0]->getCache()->getNumSamples(isTrainingSamples);
  for (size_t i = 1; i < arguments.size(); ++i)
  {
    arguments[i]->updateCache(context, isTrainingSamples);
    size_t cacheSize = arguments[i]->getCache()->getNumSamples(isTrainingSamples);
    if (cacheSize < minCacheSize)
      minCacheSize = cacheSize;
    inputBaseType = Type::findCommonBaseType(inputBaseType, arguments[i]->getType());
  }

  size_t currentSize = cache->getNumSamples(isTrainingSamples);
  if (minCacheSize > currentSize)
  {
    std::vector<VectorPtr> inputs(arguments.size());
    for (size_t i = 0; i < inputs.size(); ++i)
      inputs[i] = arguments[i]->getCache()->getSamples(isTrainingSamples);
    cache->setSamples(isTrainingSamples, function->compute(context, inputs, type));
    //context.informationCallback(T("Compute ") + toShortString() + T(" => ") + cache->toShortString());
  }
}

/*
** LuapeYieldNode
*/
LuapeYieldNode::LuapeYieldNode(const LuapeNodePtr& argument)
  : LuapeNode(nilType, argument->getName()), argument(argument)
{
}

String LuapeYieldNode::toShortString() const
{
  return T("yield(") + argument->toShortString() + T(") [") + argument->getCache()->toShortString() + T("]");
}

Variable LuapeYieldNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  if (callback)
    callback->valueYielded(state[argument]);
  return state[argument];
}

size_t LuapeYieldNode::getDepth() const
  {return argument->getDepth() + 1;}

void LuapeYieldNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  t.staticCast<LuapeYieldNode>()->argument = argument;
}
/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.cpp                 | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeCache.h"
#include "LuapeNode.h"
#include <algorithm>
using namespace lbcpp;

/*
** LuapeInstanceCache
*/
void LuapeInstanceCache::setInputObject(const std::vector<LuapeInputNodePtr>& inputs, const ObjectPtr& object)
{
  ContainerPtr container = object.dynamicCast<Container>();
  if (container)
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
      set(inputs[i], container->getElement(i));
  }
  else
  {
    size_t n = object->getNumVariables();
    if (n == inputs.size())
    {
      for (size_t i = 0; i < n; ++i)
        set(inputs[i], object->getVariable(i));
    }
    else
    {
      jassert(inputs.size() == 1 && inputs[0]->getType() == object->getClass());
      set(inputs[0], object);
    }
  }
}

void LuapeInstanceCache::set(const LuapeNodePtr& node, const Variable& value)
{
  jassert(m.find(node) == m.end());
  m[node] = value;
}

Variable LuapeInstanceCache::compute(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodeToValueMap::const_iterator it = m.find(node);
  if (it == m.end())
  {
    Variable res = node->compute(context, LuapeInstanceCachePtr(this));
    m[node] = res;
    return res;
  }
  else
    return it->second;
}

/*
** LuapeSamplesCache
*/
LuapeSamplesCache::LuapeSamplesCache(const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSizeInMb)
  : maxCacheSize(maxCacheSizeInMb * 1024), actualCacheSize(0)
{
  inputCaches.resize(inputs.size());
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    inputCaches[i] = vector(inputs[i]->getType(), size);
    set(inputs[i], inputCaches[i]);
  }
}

void LuapeSamplesCache::setInputObject(const std::vector<LuapeInputNodePtr>& inputs, size_t index, const ObjectPtr& object)
{
  ContainerPtr container = object.dynamicCast<Container>();
  if (container)
  {
    size_t n = container->getNumElements();
    jassert(n <= inputs.size());
    for (size_t i = 0; i < n; ++i)
      inputCaches[i]->setElement(index, container->getElement(i));
  }
  else if (object.isInstanceOf<SparseDoubleObject>())
  {
    SparseDoubleObjectPtr sparseObject = object.staticCast<SparseDoubleObject>();
    const std::vector< std::pair<size_t, double> >& values = sparseObject->getValues();
    for (size_t i = 0; i < values.size(); ++i)
      inputCaches[values[i].first]->setElement(index, values[i].second);
  }
  else
  {
    size_t n = object->getNumVariables();
    if (n == inputs.size())
    {
      for (size_t i = 0; i < n; ++i)
        inputCaches[i]->setElement(index, object->getVariable(i));
    }
    else
    {
      jassert(inputs.size() == 1 && inputs[0]->getType() == object->getClass());
      inputCaches[0]->setElement(index, object);
    }
  }
}

size_t LuapeSamplesCache::getSizeInBytes(const VectorPtr& samples) const
{
  if (!samples)
    return 0;
  SparseDoubleVectorPtr sparseVector = samples.dynamicCast<SparseDoubleVector>();
  if (sparseVector)
    return sparseVector->getNumValues() * 16;
  else
  {
    TypePtr elementsType = samples->getElementsType();
    size_t n = samples->getNumElements();
    if (elementsType == booleanType)
      return n / 8;
    else
      return n * 8;
  }
}

void LuapeSamplesCache::set(const LuapeNodePtr& node, const VectorPtr& samples)
{
  jassert(m.find(node) == m.end());
  m[node] = std::make_pair(samples, SparseDoubleVectorPtr());
  actualCacheSize += getSizeInBytes(samples);
}

VectorPtr LuapeSamplesCache::get(const LuapeNodePtr& node) const
{
  NodeToSamplesMap::const_iterator it = m.find(node);
  return it == m.end() ? VectorPtr() : it->second.first;
}

SparseDoubleVectorPtr LuapeSamplesCache::getSortedDoubleValues(ExecutionContext& context, const LuapeNodePtr& node, const std::vector<size_t>& examples)
{
  std::pair<VectorPtr, SparseDoubleVectorPtr>& c = internalCompute(context, node, true);
  if (examples.size() == getNumSamples()) // we only perform caching if all examples are selected
  {
    if (!c.second)
    {
      c.second = computeSortedDoubleValues(context, c.first, examples);
      actualCacheSize += getSizeInBytes(c.second);
    }
    return c.second;
  }
  else
    return computeSortedDoubleValues(context, c.first, examples);
}

VectorPtr LuapeSamplesCache::compute(ExecutionContext& context, const LuapeNodePtr& node, bool isRemoveable)
{
  jassert(node);
  return internalCompute(context, node, isRemoveable).first;
}

std::pair<VectorPtr, SparseDoubleVectorPtr>& LuapeSamplesCache::internalCompute(ExecutionContext& context, const LuapeNodePtr& node, bool isRemoveable)
{
  jassert(node);
  NodeToSamplesMap::iterator it = m.find(node);
  if (it == m.end())
  {
    std::pair<VectorPtr, SparseDoubleVectorPtr>& res = m[node];
    res.first = node->compute(context, LuapeSamplesCachePtr(this));
    actualCacheSize += getSizeInBytes(res.first);
    if (maxCacheSize)
    {
      if (isRemoveable)
        cacheSequence.push_back(node);
      while (actualCacheSize > maxCacheSize && cacheSequence.size() && cacheSequence.front() != node)
      {
        actualCacheSize -= getSizeInBytes(res.first) + getSizeInBytes(res.second);
        m.erase(cacheSequence.front());
        cacheSequence.pop_front();
      }
    }
    return res;
  }
  else
    return it->second;
}

struct SortDoubleValuesOperator
{
  static double transformIntoValidNumber(double input)
    {return input;}

  bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
  {
    double aa = transformIntoValidNumber(a.second);
    double bb = transformIntoValidNumber(b.second);
    return aa == bb ? a.first < b.first : aa < bb;
  }
};

SparseDoubleVectorPtr LuapeSamplesCache::computeSortedDoubleValues(ExecutionContext& context, const VectorPtr& samples, const std::vector<size_t>& examples) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector();
  std::vector< std::pair<size_t, double> >& v = res->getValuesVector();
  size_t n = examples.size();
  v.reserve(n);
  DenseDoubleVectorPtr scalarSamples = samples.dynamicCast<DenseDoubleVector>();
  if (scalarSamples)
  {
    // optimized versions if double samples
    double* ptr = scalarSamples->getValuePointer(0);
    for (size_t i = 0; i < n; ++i)
    {
      double value = ptr[examples[i]];
      if (value != doubleMissingValue)
        v.push_back(std::make_pair(examples[i], value));
    }
  }
  else
  {
    for (size_t i = 0; i < n; ++i)
    {
      Variable value = samples->getElement(examples[i]);
      if (value.exists())
        v.push_back(std::make_pair(examples[i], value.toDouble()));
    }
  }
  std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  return res;
}

/*
** LuapeNodeUniverse
*/
LuapeFunctionNodePtr LuapeNodeUniverse::makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs)
{
  FunctionKey key;
  key.functionClass = functionClass;
  key.arguments = arguments;
  key.inputs = inputs;
  FunctionNodesMap::const_iterator it = functionNodes.find(key);
  if (it == functionNodes.end())
  {
    LuapeFunctionPtr function = LuapeFunction::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    LuapeFunctionNodePtr res = new LuapeFunctionNode(function, inputs);
    functionNodes[key] = res;
    return res;
  }
  else
    return it->second;
}

LuapeFunctionNodePtr LuapeNodeUniverse::makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
{
  std::vector<Variable> arguments(function->getNumVariables());
  for (size_t i = 0; i < arguments.size(); ++i)
    arguments[i] = function->getVariable(i);
  return makeFunctionNode(function->getClass(), arguments, inputs);
}

#if 0
/*
** LuapeNodeKeysMap
*/
void LuapeNodeKeysMap::clear()
{
  keyToNodes.clear();
  nodeToKeys.clear();
}

// return true if it is a new node
bool LuapeNodeKeysMap::addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodeToKeyMap::const_iterator it = nodeToKeys.find(node);
  if (it != nodeToKeys.end())
    return false; // we already know about this node

  // compute node key
  graph->updateNodeCache(context, node, true);
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();

  KeyToNodeMap::iterator it2 = keyToNodes.find(key);
  if (it2 == keyToNodes.end())
  {
    // this is a new node equivalence class
    nodeToKeys[node] = key;
    keyToNodes[key] = node;
    addSubNodesToCache(context, node);
    return true;
  }
  else
  {
    // existing node equivalence class
    //  see if new node is better than previous one to represent the class
    LuapeNodePtr previousNode = it2->second;
    if (node->getDepth() < previousNode->getDepth())
    {
      it2->second = node;
      context.informationCallback(T("Change computation of ") + previousNode->toShortString() + T(" into ") + node->toShortString());
      LuapeFunctionNodePtr sourceFunctionNode = node.dynamicCast<LuapeFunctionNode>();
      LuapeFunctionNodePtr targetFunctionNode = previousNode.dynamicCast<LuapeFunctionNode>();
      if (sourceFunctionNode && targetFunctionNode)
        sourceFunctionNode->clone(context, targetFunctionNode);
      addSubNodesToCache(context, node);
    }
    nodeToKeys[node] = it2->first;
    return false;
  }
}

void LuapeNodeKeysMap::addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (functionNode)
  {
    std::vector<LuapeNodePtr> arguments = functionNode->getArguments();
    for (size_t i = 0; i < arguments.size(); ++i)
      addNodeToCache(context, arguments[i]);
  }
}

bool LuapeNodeKeysMap::isNodeKeyNew(const LuapeNodePtr& node) const
{
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();
  return keyToNodes.find(key) == keyToNodes.end();
}
#endif // 0

#if 0
BinaryKeyPtr LuapeNodeCache::makeKeyFromSamples(bool useTrainingSamples) const
{
  ContainerPtr samples = getSamples(useTrainingSamples);
  size_t n = samples->getNumElements();
  
  BooleanVectorPtr booleanVector = samples.dynamicCast<BooleanVector>();
  if (booleanVector)
  {
    BinaryKeyPtr res = new BinaryKey(1 + n / 8);
    std::vector<bool>::const_iterator it = booleanVector->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      res->pushBit(*it++);
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
  size_t n = trainingSamples ? trainingSamples->getNumElements() : 0;
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
    return String((int)countOfTrue) + T(" / ") + String((int)n);
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

#endif // 0

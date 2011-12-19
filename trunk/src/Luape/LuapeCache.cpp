/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.cpp                 | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeCache.h>
#include <lbcpp/Luape/LuapeNode.h>
#include <lbcpp/Core/DynamicObject.h>
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
** LuapeSampleVector
*/
LuapeSampleVector::LuapeSampleVector(Implementation implementation, const IndexSetPtr& indices, const TypePtr& elementsType)
  : implementation(implementation), indices(indices), elementsType(elementsType), constantRawBoolean(2) {}

LuapeSampleVector::LuapeSampleVector(const IndexSetPtr& indices, const VectorPtr& ownedVector)
  : implementation(ownedVectorImpl), indices(indices), elementsType(ownedVector->getElementsType()), constantRawBoolean(2), vector(ownedVector) {}

LuapeSampleVector::LuapeSampleVector() : implementation(noImpl), constantRawBoolean(2)
{
}

LuapeSampleVectorPtr LuapeSampleVector::createConstant(IndexSetPtr indices, const Variable& constantValue)
{
  LuapeSampleVectorPtr res(new LuapeSampleVector(constantValueImpl, indices, constantValue.getType()));
  res->constantValue = constantValue;
  res->constantRawBoolean = (constantValue.exists() && constantValue.isBoolean() ? (constantValue.getBoolean() ? 1 : 0) : 2);
  res->constantRawDouble = constantValue.isConvertibleToDouble() ? constantValue.toDouble() : 0.0;
  res->constantRawObject = constantValue.isObject() ? constantValue.getObject() : ObjectPtr();
  return res;
}

LuapeSampleVectorPtr LuapeSampleVector::createCached(IndexSetPtr indices, const VectorPtr& cachedVector)
{
  LuapeSampleVectorPtr res(new LuapeSampleVector(cachedVectorImpl, indices, cachedVector->getElementsType()));
  res->vector = cachedVector;
  return res;
}

/*
** LuapeSamplesCache
*/
LuapeSamplesCache::LuapeSamplesCache(LuapeNodeUniversePtr universe, const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSizeInMb)
  : universe(universe), inputNodes(inputs), maxCacheSize(maxCacheSizeInMb * 1024 * 1024), actualCacheSize(0), allIndices(new IndexSet(0, size))
{
  inputCaches.resize(inputs.size());
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    inputCaches[i] = vector(inputs[i]->getType(), size);
    cacheNode(defaultExecutionContext(), inputs[i], inputCaches[i], "Input node");
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

size_t LuapeSamplesCache::NodeCache::getSizeInBytes() const
  {return (samples ? getSizeInBytes(samples) : 0) + (sortedDoubleValues ? getSizeInBytes(sortedDoubleValues) : 0);}

size_t LuapeSamplesCache::NodeCache::getSizeInBytes(VectorPtr samples)
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

void LuapeSamplesCache::cacheNode(ExecutionContext& context, const LuapeNodePtr& node, const VectorPtr& values, const String& reason)
{
  NodeCache& nodeCache = m[node];
  jassert(!nodeCache.samples);
  nodeCache.samples = values ? values : node->compute(context, refCountedPointerFromThis(this), allIndices)->getVector();
  jassert(nodeCache.samples);

  size_t sizeInBytes = nodeCache.getSizeInBytes();
  actualCacheSize += sizeInBytes;
  std::cout << (const char* )reason << ". Node " << node->toShortString() << " -> size = " << sizeInBytes / 1024 << " Kb" << std::endl;
}

bool LuapeSamplesCache::isNodeCached(const LuapeNodePtr& node) const
{
  NodeCacheMap::const_iterator it = m.find(node);
  return it != m.end() && it->second.samples;
}

VectorPtr LuapeSamplesCache::getNodeCache(const LuapeNodePtr& node) const
{
  NodeCacheMap::const_iterator it = m.find(node);
  return it == m.end() ? VectorPtr() : it->second.samples;
}

double LuapeSamplesCache::computeExpectedComputingTimePerSample(const LuapeNodePtr& node) const
{
  double res = 0.0;
  for (size_t i = 0; i < node->getNumSubNodes(); ++i)
  {
    const LuapeNodePtr& subNode = node->getSubNode(i);
    if (!isNodeCached(subNode))
      res += computeExpectedComputingTimePerSample(subNode);
  }
  return res + universe->getExpectedComputingTime(node);
}

LuapeSampleVectorPtr LuapeSamplesCache::getSamples(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices, bool isRemoveable)
{
  if (indices->empty())
    return LuapeSampleVector::createConstant(indices, Variable::missingValue(node->getType()));

  NodeCache& nodeCache = m[node];

  if (nodeCache.samples)
  {
    nodeCache.observeComputingTime(computeExpectedComputingTimePerSample(node) * indices->size());
    return LuapeSampleVector::createCached(indices, nodeCache.samples);
  }
  else
  {
    double startTime = Time::getMillisecondCounterHiRes();
    LuapeSampleVectorPtr res = node->compute(context, refCountedPointerFromThis(this), indices);
    double endTime = Time::getMillisecondCounterHiRes();
    nodeCache.observeComputingTime(endTime - startTime);

    if (indices == allIndices && res->getVector() && actualCacheSize < 2 * maxCacheSize / 3)
      cacheNode(context, node, res->getVector(), "Cache by opportunism");
    return res;
  }
}

void LuapeSamplesCache::observeNodeComputingTime(const LuapeNodePtr& node, size_t numInstances, double timeInMilliseconds)
  {universe->observeNodeComputingTime(node, numInstances, timeInMilliseconds);}

bool LuapeSamplesCache::checkCacheIsCorrect(ExecutionContext& context, const LuapeNodePtr& node)
{
  size_t n = node->getNumSubNodes();
  for (size_t i = 0; i < n; ++i)
    if (!checkCacheIsCorrect(context, node->getSubNode(i)))
      return false;

  LuapeSampleVectorPtr outputs = getSamples(context, node, allIndices);
  for (LuapeSampleVector::const_iterator it = outputs->begin(); it != outputs->end(); ++it)
  {
    size_t index = it.getIndex();
    LuapeInstanceCachePtr instanceCache(new LuapeInstanceCache());
    jassert(inputNodes.size() == inputCaches.size());
    for (size_t j = 0; j < inputNodes.size(); ++j)
    {
      jassert(inputCaches[j]->getNumElements() == n);
      instanceCache->set(inputNodes[j], inputCaches[j]->getElement(index));
    }
    Variable sampleCacheOutput = *it;
    Variable instanceCacheOutput = instanceCache->compute(context, node);
    if (sampleCacheOutput != instanceCacheOutput)
    {
      context.errorCallback(T("Invalid cache for node ") + node->toShortString() + T(" at index ") + String((int)index));
      context.resultCallback(T("sampleCacheOutput"), sampleCacheOutput);
      context.resultCallback(T("instanceCacheOutput"), instanceCacheOutput);
      jassert(false);
      return false;
    }
  }
  return true;
}

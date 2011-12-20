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
  computingTimeThresholdToCache = 0.1; // 0.1 ms is the minimum computing time spent before considering entering cache
  inputCaches.resize(inputs.size());
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    inputCaches[i] = vector(inputs[i]->getType(), size);
    cacheNode(defaultExecutionContext(), inputs[i], inputCaches[i], "Input node", false);
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
  {return (samples ? samples->getSizeInBytes() : 0) + (sortedDoubleValues ? sortedDoubleValues->getSizeInBytes() : 0);}

size_t LuapeSamplesCache::getCacheSizeInBytes() const
{
  return actualCacheSize + m.size() * (sizeof (NodeCache) + sizeof (LuapeNodePtr) + 128); // estimated over-head for storing a new entry in the map
}

void LuapeSamplesCache::cacheNode(ExecutionContext& context, const LuapeNodePtr& node, const VectorPtr& values, const String& reason, bool isUncachable)
{
  NodeCache& nodeCache = m[node];
  jassert(!nodeCache.samples);
  nodeCache.samples = values ? values : node->compute(context, refCountedPointerFromThis(this), allIndices)->getVector();
  if (!isUncachable)
    nodeCache.timeSpentInComputingSamples = -1.0;
  jassert(nodeCache.samples);

  size_t sizeInBytes = nodeCache.getSizeInBytes();
  actualCacheSize += sizeInBytes;
  std::cout << (const char* )reason << ". Node " << node->toShortString() << " -> size = " << sizeInBytes / 1024.0 << " Kb" << std::endl;
  std::cout << "Cache size: " << getCacheSizeInBytes() / (1024.0 * 1024.0) << " / " << maxCacheSize / (1024 * 1024) << " Mb" << std::endl;
  ensureSizeInLowerThanMaxSize(context);
}

void LuapeSamplesCache::ensureSizeInLowerThanMaxSize(ExecutionContext& context)
{
  if (maxCacheSize)
  {
    // un-cache nodes if the cache is full
    size_t size = getCacheSizeInBytes();
    if (size > maxCacheSize)
    {
      size_t prevSize;
      do
      {
        uncacheNodes(context, 10);
        prevSize = size;
        size = getCacheSizeInBytes();
      }
      while (size > maxCacheSize && size != prevSize);
    }
  }
}

void LuapeSamplesCache::uncacheNode(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodeCache& nodeCache = m[node];
  jassert(nodeCache.samples);
  size_t sizeInBytes = nodeCache.getSizeInBytes();
  nodeCache.samples = VectorPtr();
  nodeCache.sortedDoubleValues = SparseDoubleVectorPtr();
  actualCacheSize -= sizeInBytes;
  std::cout << "Uncache node " << node->toShortString() << " -> size = " << sizeInBytes / 1024 << " Kb" << std::endl; 
  std::cout << "Cache size: " << getCacheSizeInBytes() / (1024 * 1024) << " / " << maxCacheSize / (1024 * 1024) << " Mb" << std::endl;
}

void LuapeSamplesCache::uncacheNodes(ExecutionContext& context, size_t count)
{
  std::multimap<double, LuapeNodePtr> sortedNodes;
  for (NodeCacheMap::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    double score = it->second.timeSpentInComputingSamples;
    if (score >= 0.0 && it->second.samples)
    {
      if (sortedNodes.size() < count || score < computingTimeThresholdToCache)
      {
        sortedNodes.insert(std::make_pair(score, it->first));
        if (sortedNodes.size() > count)
          sortedNodes.erase(sortedNodes.rbegin()->first);
        computingTimeThresholdToCache = sortedNodes.rbegin()->first;
        jassert(isNumberValid(computingTimeThresholdToCache));
      }
    }
  }
  
  // if the cache it not big enough to store all inputs and their sorted double values, 
  // it may happen that no nodes can be uncached
  // in this case, the cache will big bigger than the maximum limit ...
  if (sortedNodes.size())
  {
    for (std::multimap<double, LuapeNodePtr>::const_iterator it = sortedNodes.begin(); it != sortedNodes.end(); ++it)
      uncacheNode(context, it->second);
    jassert(isNumberValid(computingTimeThresholdToCache));
    std::cout << "New threshold: " << computingTimeThresholdToCache / 1000.0 << "s" << std::endl;
  }
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

void LuapeSamplesCache::displayCacheInformation(ExecutionContext& context)
{
  /*
  ** Infos by Node type
  */
  std::map<LuapeNodeUniverse::NodeTypeKey, NodeTypeCache> infoByKey;
  for (NodeCacheMap::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    LuapeNodeUniverse::NodeTypeKey key = LuapeNodeUniverse::makeNodeStatisticsKey(it->first);
    infoByKey[key].observe(it->second);
  }

  for (std::map<LuapeNodeUniverse::NodeTypeKey, NodeTypeCache>::const_iterator it = infoByKey.begin();
        it != infoByKey.end(); ++it)
  {
    String info = it->first.second ? it->first.second->getName() : it->first.first->getName();
    
    info += T(" ") + String((int)it->second.numCached) + T(" / ") + String((int)it->second.count) + " cached nodes";
    info += T(", size = ") + String((int)it->second.cacheSizeInBytes / 1024) + T(" Kb, ");
    info += it->second.cachedComputingTime.toShortString() + T(" -- ") + it->second.uncachedComputingTime.toShortString();
    context.informationCallback(info);
  }

  /*
  ** Infos by node
  */
  std::multimap<double, LuapeNodePtr> cachedNodes;
  std::multimap<double, LuapeNodePtr> uncachedNodes;
  double worstScore = DBL_MAX;

  for (NodeCacheMap::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    double score = it->second.timeSpentInComputingSamples;
    if (score < 0.0)
      continue;
    if (it->second.samples)
      cachedNodes.insert(std::make_pair(score, it->first));
    else
      uncachedNodes.insert(std::make_pair(score, it->first));
  }

  size_t i;

  i = 0;
  for (std::multimap<double, LuapeNodePtr>::const_reverse_iterator it = cachedNodes.rbegin(); it != cachedNodes.rend() && i < 5; ++it, ++i)
    context.informationCallback(T("Best cached: ") + it->second->toShortString() + T(" [") + String(it->first / 1000.0) + T("s]"));
  i = 0;
  for (std::multimap<double, LuapeNodePtr>::const_iterator it = cachedNodes.begin(); it != cachedNodes.end() && i < 5; ++it, ++i)
    context.informationCallback(T("Worst cached: ") + it->second->toShortString() + T(" [") + String(it->first / 1000.0) + T("s]"));
  i = 0;

  for (std::multimap<double, LuapeNodePtr>::const_reverse_iterator it = uncachedNodes.rbegin(); it != uncachedNodes.rend() && i < 5; ++it, ++i)
    context.informationCallback(T("Best uncached: ") + it->second->toShortString() + T(" [") + String(it->first / 1000.0) + T("s]"));
  i = 0;
  for (std::multimap<double, LuapeNodePtr>::const_iterator it = uncachedNodes.begin(); it != uncachedNodes.end() && i < 5; ++it, ++i)
    context.informationCallback(T("Worst uncached: ") + it->second->toShortString() + T(" [") + String(it->first / 1000.0) + T("s]"));
}

double LuapeSamplesCache::computeExpectedComputingTimePerSample(const LuapeNodePtr& node) const
{
  double res = 0.0;
  /*for (size_t i = 0; i < node->getNumSubNodes(); ++i)
  {
    const LuapeNodePtr& subNode = node->getSubNode(i);
    if (!isNodeCached(subNode))
      res += computeExpectedComputingTimePerSample(subNode);
  }*/
  return res + universe->getExpectedComputingTime(node);
}

LuapeSampleVectorPtr LuapeSamplesCache::getSamples(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices, bool isRemoveable)
{
  if (indices->empty())
    return LuapeSampleVector::createConstant(indices, Variable::missingValue(node->getType()));

  NodeCache& nodeCache = m[node];

  // cache nodes on which we spend much computation time
  if (!nodeCache.samples && nodeCache.timeSpentInComputingSamples > computingTimeThresholdToCache && computingTimeThresholdToCache > 10)
    cacheNode(context, node, VectorPtr(), "Deliberate caching");

  LuapeSampleVectorPtr res;
  if (nodeCache.samples)
  {
    // update stats and return cached data
    res = LuapeSampleVector::createCached(indices, nodeCache.samples);
  }
  else
  {
    // compute
    res = node->compute(context, refCountedPointerFromThis(this), indices);

    // see if we can cache by opportunism
    if (indices == allIndices && res->getVector() &&
        (!maxCacheSize || getCacheSizeInBytes() < maxCacheSize) &&
        nodeCache.timeSpentInComputingSamples > computingTimeThresholdToCache)
      cacheNode(context, node, res->getVector(), "Cache by opportunism");
  }
  nodeCache.observeComputingTime(computeExpectedComputingTimePerSample(node) * indices->size());
  ensureSizeInLowerThanMaxSize(context);
  return res;
}

SparseDoubleVectorPtr LuapeSamplesCache::getSortedDoubleValues(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices)
{
  if (indices->empty())
    return SparseDoubleVectorPtr();

  NodeCache& nodeCache = m[node];
  if (nodeCache.sortedDoubleValues)
  {
    if (indices == allIndices)
      return nodeCache.sortedDoubleValues;
    else
      return computeSortedDoubleValuesSubset(nodeCache.sortedDoubleValues, indices);
  }

  // compute sorted double values
  SparseDoubleVectorPtr res = computeSortedDoubleValuesFromSamples(getSamples(context, node, indices));
  if (indices == allIndices && nodeCache.samples)
  {
    // opportunism caching
    nodeCache.sortedDoubleValues = res;
    actualCacheSize += res->getSizeInBytes();
    ensureSizeInLowerThanMaxSize(context);
  }
  return res;
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

SparseDoubleVectorPtr LuapeSamplesCache::computeSortedDoubleValuesFromSamples(const LuapeSampleVectorPtr& samples) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
  std::vector< std::pair<size_t, double> >& v = res->getValuesVector();
  
  size_t n = samples->size();
  v.reserve(n);
  bool isDouble = (samples->getElementsType() == doubleType);
  for (LuapeSampleVector::const_iterator it = samples->begin(); it != samples->end(); ++it)
  {
    double value = isDouble ? it.getRawDouble() : (*it).toDouble();
    if (value != doubleMissingValue)
      v.push_back(std::make_pair(it.getIndex(), value));
  }
  std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  return res;
}

SparseDoubleVectorPtr LuapeSamplesCache::computeSortedDoubleValuesSubset(const SparseDoubleVectorPtr& allValues, const IndexSetPtr& indices) const
{
  // std::vector<int> is faster than std::vector<bool>
  std::vector<int> flags(allIndices->size(), 0);
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    flags[*it] = 1;
  
  SparseDoubleVectorPtr res = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
  std::vector<std::pair<size_t, double> >& resValues = res->getValuesVector();
  
  resValues.resize(indices->size());
  size_t index = 0;
  for (size_t i = 0; i < allValues->getNumValues(); ++i)
    if (flags[allValues->getValue(i).first] > 0)
      resValues[index++] = allValues->getValue(i);
  jassert(index == indices->size());
  return res;
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

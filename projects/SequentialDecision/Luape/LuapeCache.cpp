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
  if (object->getClass().isInstanceOf<DynamicClass>())
  {
    size_t n = object->getNumVariables();
    for (size_t i = 0; i < n; ++i)
      set(inputs[i], object->getVariable(i));
  }
  else
  {
    ContainerPtr container = object.dynamicCast<Container>();
    jassert(container);
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
      set(inputs[i], container->getElement(i));
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
LuapeSamplesCache::LuapeSamplesCache(const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSize)
  : maxCacheSize(maxCacheSize)
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
  else
  {
    size_t n = object->getNumVariables();
    jassert(n <= inputs.size());
    for (size_t i = 0; i < n; ++i)
      inputCaches[i]->setElement(index, object->getVariable(i));
  }
}

void LuapeSamplesCache::set(const LuapeNodePtr& node, const VectorPtr& samples)
{
  jassert(m.find(node) == m.end());
  m[node] = std::make_pair(samples, SparseDoubleVectorPtr());
}

VectorPtr LuapeSamplesCache::get(const LuapeNodePtr& node) const
{
  NodeToSamplesMap::const_iterator it = m.find(node);
  return it == m.end() ? VectorPtr() : it->second.first;
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

void LuapeSamplesCache::ensureSortedDoubleValuesAreComputed(const VectorPtr& samples, SparseDoubleVectorPtr& sortedDoubleValues)
{
  if (!sortedDoubleValues)
  {
    size_t n = samples->getNumElements();
    sortedDoubleValues = new SparseDoubleVector();
    std::vector< std::pair<size_t, double> >& v = sortedDoubleValues->getValuesVector();
    v.resize(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = std::make_pair(i, samples->getElement(i).toDouble());
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  }
}

VectorPtr LuapeSamplesCache::compute(ExecutionContext& context, const LuapeNodePtr& node, SparseDoubleVectorPtr* sortedDoubleValues, bool isRemoveable)
{
  NodeToSamplesMap::iterator it = m.find(node);
  if (it == m.end())
  {
    std::pair<VectorPtr, SparseDoubleVectorPtr>& res = m[node];
    VectorPtr samples = res.first = node->compute(context, LuapeSamplesCachePtr(this));
    if (sortedDoubleValues)
    {
      ensureSortedDoubleValuesAreComputed(res.first, res.second);
      *sortedDoubleValues = res.second;
    }

    if (maxCacheSize)
    {
      if (isRemoveable)
        cacheSequence.push_back(node);
      if (m.size() >= maxCacheSize && cacheSequence.size())
      {
        m.erase(cacheSequence.front());
        cacheSequence.pop_front();
      }
    }
    return samples;
  }
  else
  {
    if (sortedDoubleValues)
    {
      ensureSortedDoubleValuesAreComputed(it->second.first, it->second.second);
      *sortedDoubleValues = it->second.second;
    }
    return it->second.first;
  }
}

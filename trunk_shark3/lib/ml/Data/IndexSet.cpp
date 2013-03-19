/*-----------------------------------------.---------------------------------.
| Filename: IndexSet.cpp                   | Index Set                       |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <oil/Core.h>
#include <ml/IndexSet.h>
#include <algorithm>
using namespace lbcpp;

IndexSet::IndexSet(size_t begin, size_t end)
  {addInterval(begin, end);}

IndexSet::IndexSet(size_t size)
  {addInterval(0, size);}

void IndexSet::addInterval(size_t begin, size_t end)
{
  jassert(begin <= end);
  jassert(v.empty() || begin >= v.back());
  size_t s = v.size();
  v.resize(s + end - begin);
  s -= begin;
  for (size_t i = begin; i < end; ++i)
    v[s + i] = i;
}

IndexSetPtr IndexSet::sampleSubset(RandomGeneratorPtr random, size_t subsetSize) const
{
  IndexSetPtr res = new IndexSet();
  jassert(subsetSize <= v.size());
  if (subsetSize == v.size())
    return refCountedPointerFromThis(this);
  std::vector<size_t> order;
  random->sampleOrder(v.size(), order);
  res->v.resize(subsetSize);
  for (size_t i = 0; i < subsetSize; ++i)
    res->v[i] = v[order[i]];
  std::sort(res->v.begin(), res->v.end());
  return res;
}

IndexSetPtr IndexSet::sampleBootStrap(RandomGeneratorPtr random) const
{
  IndexSetPtr res = new IndexSet();
  size_t n = v.size();
  res->v.resize(n);
  for (size_t i = 0; i < n; ++i)
    res->v[i] = random->sampleSize(n);
  std::sort(res->v.begin(), res->v.end());
  return res;
}

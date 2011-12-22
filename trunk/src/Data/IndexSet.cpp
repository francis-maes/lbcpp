/*-----------------------------------------.---------------------------------.
| Filename: IndexSet.cpp                   | Index Set                       |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Data/IndexSet.h>
using namespace lbcpp;

IndexSet::IndexSet(size_t begin, size_t end)
{
  addInterval(begin, end);
}

IndexSet::IndexSet()
{
}

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

void IndexSet::randomlyExpandUsingSource(ExecutionContext& context, size_t newSize, const IndexSetPtr& source)
{
  RandomGeneratorPtr random = context.getRandomGenerator();

  jassert(newSize > v.size());
  jassert(newSize <= source->size());

  if (newSize == source->size())
  {
    v = source->v;
    return;
  }

  std::set<size_t> thisIndices;
  for (size_t i = 0; i < v.size(); ++i)
    thisIndices.insert(v[i]);

  size_t remainingInSource = source->size() - v.size();
  size_t numRequired = newSize - v.size();
  
  if (numRequired < remainingInSource / 3)
  {
    while (thisIndices.size() < newSize)
      thisIndices.insert(source->v[random->sampleSize(source->size())]);
  }
  else
  {
    std::vector<size_t> order;
    random->sampleOrder(source->v.size(), order);
    size_t i = 0;
    while (thisIndices.size() < newSize)
      thisIndices.insert(source->v[order[i++]]);
  }

  v.resize(thisIndices.size());
  size_t i = 0;
  for (std::set<size_t>::const_iterator it = thisIndices.begin(); it != thisIndices.end(); ++it)
    v[i++] = *it;

  jassert(v.size() == newSize);
}

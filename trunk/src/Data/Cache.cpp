/*-----------------------------------------.---------------------------------.
| Filename: Cache.cpp                      | Variables cache                 |
| Author  : Francis Maes                   |                                 |
| Started : 01/10/2010 18:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Cache.h>
#include <lbcpp/Data/RandomVariable.h>
using namespace lbcpp;

/*
** Cache
*/
Cache::Cache(size_t pruningFrequency)
  : pruningFrequency(pruningFrequency) {}

void Cache::pruneUnreferencedEntries()
{
  ScopedLock _(cacheLock);
  size_t previousSize = cache.size();
  CacheMap::iterator it, nxt;
  for (it = cache.begin(); it != cache.end(); it = nxt)
  {
    nxt = it; ++nxt;
    if (it->first->getReferenceCount() == 1)
    {
      cache.erase(it);
      ++numDeletions;
    }
  }
}

void Cache::prune()
{
  pruneUnreferencedEntries();
}

Variable Cache::getEntry(ObjectPtr object) const
{
  ScopedLock _(cacheLock);
  ++(const_cast<Cache* >(this)->numAccesses);
  CacheMap::const_iterator it = cache.find(object);
  return it == cache.end() ? Variable() : it->second;
}

Variable& Cache::getOrCreateEntry(ObjectPtr object)
{
  ScopedLock _(cacheLock);
  ++numAccesses;
  Variable& entry = cache[object];
  if (!entry)
  {
    ++numInsertions;
    if (pruningFrequency && (numInsertions % pruningFrequency) == 0)
      prune();
    entry = createEntry(object);
  }
  return entry;
}

/*
** AverageValuesCache
*/
AverageValuesCache::AverageValuesCache(size_t pruningFrequency)
  : Cache(pruningFrequency)
{
}

void AverageValuesCache::addValue(ObjectPtr object, double value)
{
  ScopedLock _(cacheLock);
  ScalarVariableMeanPtr mean = getOrCreateEntryAndCast<ScalarVariableMean>(object);
  mean->push(value);
}

double AverageValuesCache::getMeanValue(ObjectPtr object, size_t* numSamples) const
{
  ScopedLock _(cacheLock);
  ScalarVariableMeanPtr mean = getEntryAndCast<ScalarVariableMean>(object);
  if (numSamples)
    *numSamples = mean ? (size_t)mean->getCount() : 0;
  return mean ? mean->getMean() : 0.0;
}

Variable AverageValuesCache::createEntry(ObjectPtr object) const
  {return new ScalarVariableMean(object->getName());}

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
Cache::Cache(size_t pruningFrequency, double pruningDieTime)
  : pruningFrequency(pruningFrequency), pruningDieTime(pruningDieTime),
    numInsertions(0), numDeletions(0), numAccesses(0)
{
}

void Cache::pruneUnreferencedEntries()
{
  ScopedLock _(cacheLock);
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

void Cache::pruneUnreferencedSinceMoreThan(double timeInSeconds)
{
  juce::uint32 limitTime = Time::getMillisecondCounter() - (juce::uint32)(timeInSeconds * 1000.0);
  ScopedLock _(cacheLock);
  CacheMap::iterator it, nxt;
  for (it = cache.begin(); it != cache.end(); it = nxt)
  {
    nxt = it; ++nxt;
    if (it->second.second < limitTime)
    {
      cache.erase(it);
      ++numDeletions;
    }
  }
}

void Cache::prune()
{
  pruneUnreferencedEntries();
  if (pruningDieTime)
    pruneUnreferencedSinceMoreThan(pruningDieTime);
}

Variable Cache::getEntry(ObjectPtr object) const
{
  ScopedLock _(cacheLock);
  ++(const_cast<Cache* >(this)->numAccesses);
  CacheMap::const_iterator it = cache.find(object);
  if (it == cache.end())
    return Variable();
  const_cast<juce::uint32& >(it->second.second) = juce::Time::getMillisecondCounter();
  return it->second.first;
}

Variable& Cache::getOrCreateEntry(ObjectPtr object)
{
  ScopedLock _(cacheLock);
  ++numAccesses;
  std::pair<Variable, juce::uint32>& entry = cache[object];
  entry.second = juce::Time::getMillisecondCounter();
  if (!entry.first.exists())
  {
    ++numInsertions;
    if (pruningFrequency && (numInsertions % pruningFrequency) == 0)
      prune();
    entry.first = createEntry(object);
  }
  return entry.first;
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

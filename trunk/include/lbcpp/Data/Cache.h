/*-----------------------------------------.---------------------------------.
| Filename: Cache.h                        | Variables cache                 |
| Author  : Francis Maes                   |                                 |
| Started : 01/10/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CACHE_H_
# define LBCPP_DATA_CACHE_H_

# include "Variable.h"

namespace lbcpp
{

class Cache : public Object
{
public:
  Cache(size_t pruningFrequency = 0, double pruningDieTime = 0.0);

  Variable getEntry(ObjectPtr object) const;
  Variable& getOrCreateEntry(ObjectPtr object);

  template<class T>
  ReferenceCountedObjectPtr<T> getEntryAndCast(ObjectPtr object) const
  {
    Variable variable = getEntry(object);
    return variable.isNil() ? ReferenceCountedObjectPtr<T>() : variable.getObjectAndCast<T>();
  }
  
  template<class Type>
  ReferenceCountedObjectPtr<Type> getOrCreateEntryAndCast(ObjectPtr object)
    {Variable entry = getOrCreateEntry(object); return entry.getObjectAndCast<Type>();}

  void pruneUnreferencedEntries();
  void pruneUnreferencedSinceMoreThan(double timeInSeconds);

  void prune();

protected:
  virtual Variable createEntry(ObjectPtr object) const = 0;

  size_t pruningFrequency;
  double pruningDieTime;

  CriticalSection cacheLock;
  typedef std::map<ObjectPtr, std::pair<Variable, juce::uint32> > CacheMap;
  CacheMap cache;
  size_t numInsertions;
  size_t numDeletions;
  size_t numAccesses;
};

class AverageValuesCache : public Cache
{
public:
  AverageValuesCache(size_t pruningFrequency = 0);

  void addValue(ObjectPtr object, double value);
  double getMeanValue(ObjectPtr object, size_t* numSamples = NULL) const;

protected:
  virtual Variable createEntry(ObjectPtr object) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CACHE_H_

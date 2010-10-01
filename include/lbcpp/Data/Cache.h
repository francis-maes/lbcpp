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
  Cache(size_t pruningFrequency = 0);

  void pruneUnreferencedEntries();
  void prune();

protected:
  virtual Variable createEntry(ObjectPtr object) const = 0;

  size_t pruningFrequency;

  CriticalSection cacheLock;
  typedef std::map<ObjectPtr, Variable> CacheMap;
  CacheMap cache;
  size_t numInsertions;
  size_t numDeletions;
  size_t numAccesses;

  Variable getEntry(ObjectPtr object) const;
  Variable& getOrCreateEntry(ObjectPtr object);

  template<class T>
  ReferenceCountedObjectPtr<T> getEntryAndCast(ObjectPtr object) const
  {
    Variable variable = getEntry(object);
    return variable.isNil() ? ReferenceCountedObjectPtr<T>() : variable.getObjectAndCast<T>();
  }
  
  template<class T>
  ReferenceCountedObjectPtr<T> getOrCreateEntryAndCast(ObjectPtr object)
    {return getOrCreateEntry(object).getObjectAndCast<T>();}
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

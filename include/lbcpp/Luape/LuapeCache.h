/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

# include "LuapeUniverse.h"
# include "../Core/Vector.h"
# include "../Data/DoubleVector.h"
# include "../Data/IndexSet.h"

namespace lbcpp
{

/*
** LuapeInstanceCache
*/
class LuapeInstanceCache : public Object
{
public:
  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, const ObjectPtr& object);
  void set(const LuapeNodePtr& node, const Variable& value);
  Variable compute(ExecutionContext& context, const LuapeNodePtr& node);

protected:
  typedef std::map<LuapeNodePtr, Variable> NodeToValueMap;
  NodeToValueMap m;
};

typedef ReferenceCountedObjectPtr<LuapeInstanceCache> LuapeInstanceCachePtr;

/*
** LuapeSampleVector
*/
class LuapeSampleVector : public Object
{
public:
  enum Implementation
  {
    constantValueImpl = 0,
    ownedVectorImpl,
    cachedVectorImpl,
    noImpl
  };

  LuapeSampleVector(Implementation implementation, const IndexSetPtr& indices, const TypePtr& elementsType);
  LuapeSampleVector(const IndexSetPtr& indices, const VectorPtr& ownedVector);
  LuapeSampleVector();

  static LuapeSampleVectorPtr createConstant(IndexSetPtr indices, const Variable& constantValue);
  static LuapeSampleVectorPtr createCached(IndexSetPtr indices, const VectorPtr& cachedVector);
  
  const TypePtr& getElementsType() const
    {return elementsType;}

  struct const_iterator
  {
    typedef std::vector<size_t>::const_iterator index_iterator;

    const_iterator(const LuapeSampleVector* owner, size_t position, index_iterator it)
      : owner(owner), position(position), it(it) {}
    const_iterator(const const_iterator& other)
      : owner(other.owner), position(other.position), it(other.it) {}
    const_iterator() : owner(NULL), position(0) {}

    const_iterator& operator =(const const_iterator& other)
      {owner = other.owner; position = other.position; it = other.it; return *this;}

    const_iterator& operator ++()
    {
      ++position;
      ++it;
      return *this;
    }

    inline Variable operator *() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->getConstantValue();
      case ownedVectorImpl: return owner->vector->getElement(position);
      case cachedVectorImpl: return owner->vector->getElement(*it);
      default: jassert(false); return Variable();
      }
    }

    inline unsigned char getRawBoolean() const
    {
      if (owner->implementation == constantValueImpl)
        return owner->constantRawBoolean;
      size_t index = (owner->implementation == ownedVectorImpl ? position : *it);
      if (owner->elementsType == booleanType)
        return owner->vector.staticCast<BooleanVector>()->getData()[index];
      else
      {
        jassert(owner->elementsType == doubleType);
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0 ? 1 : 0);
      }
    }

    inline double getRawDouble() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawDouble;
      case ownedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(position);
      case cachedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(*it);
      default: jassert(false); return 0.0;
      }
    }

    inline const ObjectPtr& getRawObject() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawObject;
      case ownedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(position);
      case cachedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(*it);
      default: jassert(false); static ObjectPtr empty; return empty;
      }
    }

    bool operator ==(const const_iterator& other) const
      {return owner == other.owner && position == other.position;}

    bool operator !=(const const_iterator& other) const
      {return owner != other.owner || position != other.position;}

    size_t getIndex() const
      {return *it;}

  private:
    friend class LuapeSampleVector;

    const LuapeSampleVector* owner;
    size_t position;
    index_iterator it;
  };

  const_iterator begin() const
    {return const_iterator(this, 0, indices->begin());}

  const_iterator end() const
    {return const_iterator(this, indices->size(), indices->end());}

  size_t size() const
    {return indices->size();}

  const IndexSetPtr& getIndices() const
    {return indices;}

  Implementation getImplementation() const
    {return implementation;}

  const Variable& getConstantValue() const
    {return constantValue;}

  const VectorPtr& getVector() const
    {return vector;}

protected:
  Implementation implementation;
  IndexSetPtr indices;
  TypePtr elementsType;

  Variable constantValue; // constantValueImpl only
  unsigned char constantRawBoolean;
  double constantRawDouble;
  ObjectPtr constantRawObject;

  VectorPtr vector;       // ownedVectorImpl and cachedVectorImpl
};

typedef ReferenceCountedObjectPtr<LuapeSampleVector> LuapeSampleVectorPtr;

/*
** LuapeSamplesCache
*/
class LuapeSamplesCache : public Object
{
public:
  /*
  ** Construction
  */
  LuapeSamplesCache(LuapeNodeUniversePtr universe, const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSizeInMb = 1024);
  LuapeSamplesCache() : maxCacheSize(0), actualCacheSize(0) {}

  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, size_t index, const ObjectPtr& object);


  /*
  ** Cache methods
  */
  void cacheNode(ExecutionContext& context, const LuapeNodePtr& node, const VectorPtr& values = VectorPtr(), const String& reason = String::empty, bool isUncachable = true);
  void uncacheNode(ExecutionContext& context, const LuapeNodePtr& node);
  void uncacheNodes(ExecutionContext& context, size_t count);
  void ensureSizeInLowerThanMaxSize(ExecutionContext& context);

  bool isNodeCached(const LuapeNodePtr& node) const;
  VectorPtr getNodeCache(const LuapeNodePtr& node) const;

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return allIndices->size();}

  size_t getCacheSizeInBytes() const;

  void displayCacheInformation(ExecutionContext& context);
  bool checkCacheIsCorrect(ExecutionContext& context, const LuapeNodePtr& node);

  /*
  ** Compute operation
  */
  LuapeSampleVectorPtr getSamples(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices, bool isRemoveable = true);
  SparseDoubleVectorPtr getSortedDoubleValues(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices);

  /*
  ** Misc
  */
  const IndexSetPtr& getAllIndices() const
    {return allIndices;}

  void observeNodeComputingTime(const LuapeNodePtr& node, size_t numInstances, double timeInMilliseconds);

protected:
  LuapeNodeUniversePtr universe;

  struct NodeCache
  {
    NodeCache(const VectorPtr& samples) : samples(samples), timeSpentInComputingSamples(0) {}
    NodeCache() : timeSpentInComputingSamples(0.0) {}

    VectorPtr samples;
    SparseDoubleVectorPtr sortedDoubleValues;

    // in ms; when cached, this is an expected value w.r.t. the current caching state, if this node was not cached
    // if not cached, this is an observed value
    // -1 if the node cannot be uncached
    double timeSpentInComputingSamples;

    void observeComputingTime(double timeInMilliseconds)
    {
      if (timeSpentInComputingSamples >= 0)
      {
        jassert(isNumberValid(timeInMilliseconds));
        timeSpentInComputingSamples += timeInMilliseconds;
      }
    }

    size_t getSizeInBytes() const;
  };

  typedef std::map<LuapeNodePtr, NodeCache> NodeCacheMap;
  NodeCacheMap m;
  double computingTimeThresholdToCache;
  
  std::vector<LuapeInputNodePtr> inputNodes;
  std::vector<VectorPtr> inputCaches;
  size_t maxCacheSize; // in bytes
  size_t actualCacheSize; // in bytes

  IndexSetPtr allIndices;

  double computeExpectedComputingTimePerSample(const LuapeNodePtr& node) const;
  SparseDoubleVectorPtr computeSortedDoubleValuesSubset(const SparseDoubleVectorPtr& allValues, const IndexSetPtr& indices) const;
  SparseDoubleVectorPtr computeSortedDoubleValuesFromSamples(const LuapeSampleVectorPtr& samples) const;

  struct NodeTypeCache
  {
    NodeTypeCache() : count(0), numCached(0), cacheSizeInBytes(0) {}

    void observe(const NodeCache& nodeCache)
    {
      ++count;
      if (nodeCache.samples)
      {
        ++numCached;
        cacheSizeInBytes += nodeCache.getSizeInBytes();
        cachedComputingTime.push(nodeCache.timeSpentInComputingSamples);
      }
      else
        uncachedComputingTime.push(nodeCache.timeSpentInComputingSamples);
    }

    size_t count;
    size_t numCached;
    size_t cacheSizeInBytes;
    ScalarVariableStatistics cachedComputingTime;
    ScalarVariableStatistics uncachedComputingTime;
  };
};

typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CACHE_H_

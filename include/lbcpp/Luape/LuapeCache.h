/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

# include <lbcpp-ml/ExpressionUniverse.h>
# include "../Core/Vector.h"
# include "../Data/DoubleVector.h"
# include "../Data/IndexSet.h"

namespace lbcpp
{

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
      else if (owner->elementsType == probabilityType)
      {
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0.5 ? 1 : 0);
      }
      else
      {
        jassert(owner->elementsType == doubleType);
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0 ? 1 : 0);
      }
    }

    inline int getRawInteger() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantValue.getInteger();
      case ownedVectorImpl: return owner->vector->getElement(position).getInteger();
      case cachedVectorImpl: return owner->vector->getElement(*it).getInteger();
      default: jassert(false); return 0;
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

  Variable sampleElement(RandomGeneratorPtr random) const;

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
  LuapeSamplesCache(ExpressionUniversePtr universe, const std::vector<VariableExpressionPtr>& variables, size_t numSamples, size_t maxCacheSizeInMb = 1024);
  LuapeSamplesCache() : maxCacheSize(0), actualCacheSize(0) {}

  void setInputObject(const std::vector<VariableExpressionPtr>& inputs, size_t index, const ObjectPtr& object);


  /*
  ** Cache methods
  */
  void cacheNode(ExecutionContext& context, const ExpressionPtr& node, const VectorPtr& values = VectorPtr(), const String& reason = String::empty, bool isRemoveable = true);
  void uncacheNode(ExecutionContext& context, const ExpressionPtr& node);
  void uncacheNodes(ExecutionContext& context, size_t count);
  void recacheNode(ExecutionContext& context, const ExpressionPtr& node, const VectorPtr& values = VectorPtr());
  void ensureSizeInLowerThanMaxSize(ExecutionContext& context);
  void clearCache(ExecutionContext& context);

  bool isNodeCached(const ExpressionPtr& node) const;
  bool isNodeDefinitivelyCached(const ExpressionPtr& node) const;
  VectorPtr getNodeCache(const ExpressionPtr& node) const;

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return allIndices->size();}

  size_t getCacheSizeInBytes() const;
  void setMaxSizeInBytes(size_t sizeInBytes)
    {maxCacheSize = sizeInBytes;}
  void setMaxSizeInMegaBytes(size_t sizeInMegaBytes)
    {setMaxSizeInBytes(sizeInMegaBytes * 1024 * 1024);}

  void recomputeCacheSize();

  void displayCacheInformation(ExecutionContext& context);
  bool checkCacheIsCorrect(ExecutionContext& context, const ExpressionPtr& node, bool recursively);

  void disableCaching()
    {cachingEnabled = false;}

  void enableCaching()
    {cachingEnabled = true;}

  /*
  ** Compute operation (use all indices by default)
  */
  LuapeSampleVectorPtr getSamples(ExecutionContext& context, const ExpressionPtr& node, const IndexSetPtr& indices = IndexSetPtr());
  SparseDoubleVectorPtr getSortedDoubleValues(ExecutionContext& context, const ExpressionPtr& node, const IndexSetPtr& indices = IndexSetPtr());

  /*
  ** Misc
  */
  const IndexSetPtr& getAllIndices() const
    {return allIndices;}

  void observeNodeComputingTime(const ExpressionPtr& node, size_t numInstances, double timeInMilliseconds);

  /*
  ** Internal
  */
  struct NodeCache
  {
    NodeCache(const VectorPtr& samples) : samples(samples), numRequests(0) {}
    NodeCache() : numRequests(0) {}

    VectorPtr samples;
    SparseDoubleVectorPtr sortedDoubleValues;
    juce::int64 numRequests;

    size_t getSizeInBytes(bool recursively) const;
  };

  void ensureActualSizeIsCorrect() const;

protected:
  friend class LuapeSamplesCacheClass;

  ExpressionUniversePtr universe;

  typedef std::map<ExpressionPtr, NodeCache> NodeCacheMap;
  NodeCacheMap m;

  std::vector<VariableExpressionPtr> inputNodes;
  std::vector<VectorPtr> inputCaches;
  size_t maxCacheSize; // in bytes
  size_t actualCacheSize; // in bytes
  juce::int64 minNumRequestsToBeCached;

  IndexSetPtr allIndices;
  bool cachingEnabled;

  SparseDoubleVectorPtr computeSortedDoubleValuesSubset(const SparseDoubleVectorPtr& allValues, const IndexSetPtr& indices) const;
  SparseDoubleVectorPtr computeSortedDoubleValuesFromSamples(const LuapeSampleVectorPtr& samples) const;
  NodeCache& getOrCreateNodeCache(const ExpressionPtr& node);
  bool isCandidateForCaching(const ExpressionPtr& node) const;
};

typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CACHE_H_

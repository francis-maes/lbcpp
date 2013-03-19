/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

# include "ExpressionUniverse.h"
# include "../Core/Vector.h"
# include "../Data/DoubleVector.h"
# include "../Data/IndexSet.h"

namespace lbcpp
{

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
  DataVectorPtr getSamples(ExecutionContext& context, const ExpressionPtr& node, const IndexSetPtr& indices = IndexSetPtr());
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
  SparseDoubleVectorPtr computeSortedDoubleValuesFromSamples(const DataVectorPtr& samples) const;
  NodeCache& getOrCreateNodeCache(const ExpressionPtr& node);
  bool isCandidateForCaching(const ExpressionPtr& node) const;
};

typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CACHE_H_

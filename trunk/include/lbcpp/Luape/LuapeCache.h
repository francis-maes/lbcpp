/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

# include "../Core/Vector.h"
# include "../Data/DoubleVector.h"
# include "../Data/RandomVariable.h"
# include "../Data/IndexSet.h"
# include "LuapeFunction.h"
# include <deque>

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

class LuapeSampleVector : public Object
{
public:
  LuapeSampleVector(VectorPtr data, IndexSetPtr indices)
    : elementsType(data->getElementsType()), indices(indices)
  {
    dataChunks.resize(indices->getNumChunks());
    for (size_t i = 0; i < dataChunks.size(); ++i)
      dataChunks[i] = std::make_pair(data, indices->getChunkBegin(i));
  }
  LuapeSampleVector() {}
  
  const TypePtr& getElementsType() const
    {return elementsType;}

  struct const_iterator
  {
    const_iterator(const LuapeSampleVector* owner, size_t chunkNumber, size_t indexInChunk)
      : owner(owner), chunkNumber(chunkNumber), indexInChunk(indexInChunk) {}
    const_iterator(const const_iterator& other)
      : owner(other.owner), chunkNumber(other.chunkNumber), indexInChunk(other.indexInChunk) {}
    const_iterator() : owner(NULL), chunkNumber(0), indexInChunk(0) {}

    const_iterator& operator =(const const_iterator& other)
      {owner = other.owner; chunkNumber = other.chunkNumber; indexInChunk = other.indexInChunk; return *this;}

    const_iterator& operator ++()
    {
      jassert(chunkNumber < owner->indices->getNumChunks() && indexInChunk < owner->indices->getChunkNumElements(chunkNumber));
      ++indexInChunk;
      if (indexInChunk == owner->indices->getChunkNumElements(chunkNumber))
      {
        ++chunkNumber;
        indexInChunk = 0;
      }
      return *this;
    }

    Variable operator *() const
    {
      std::pair<VectorPtr, size_t> vectorAndElement = getCurrentVectorAndElement();
      return vectorAndElement.first->getElement(vectorAndElement.second);
    }

    unsigned char getRawBoolean() const
    {
      std::pair<VectorPtr, size_t> vectorAndElement = getCurrentVectorAndElement();
      return vectorAndElement.first.staticCast<BooleanVector>()->getData()[vectorAndElement.second];
    }

    double getRawDouble() const
    {
      std::pair<VectorPtr, size_t> vectorAndElement = getCurrentVectorAndElement();
      return vectorAndElement.first.staticCast<DenseDoubleVector>()->getValue(vectorAndElement.second);
    }

    bool operator ==(const const_iterator& other) const
      {return owner == other.owner && chunkNumber == other.chunkNumber && indexInChunk == other.indexInChunk;}
    bool operator !=(const const_iterator& other) const
      {return owner != other.owner || chunkNumber != other.chunkNumber || indexInChunk != other.indexInChunk;}

    size_t getIndex() const
      {return owner->indices->getChunkElement(chunkNumber, indexInChunk);}

    std::pair<VectorPtr, size_t> getCurrentVectorAndElement() const
    {
      jassert(chunkNumber < owner->indices->getNumChunks());
      std::pair<VectorPtr, size_t> res = owner->dataChunks[chunkNumber];
      res.second += owner->indices->getChunkElement(chunkNumber, indexInChunk) - owner->indices->getChunkBegin(chunkNumber);
      return res;
    }

  private:
    friend class LuapeSampleVector;

    const LuapeSampleVector* owner;
    size_t chunkNumber;
    size_t indexInChunk;
  };

  const_iterator begin() const
    {return const_iterator(this, 0, 0);}

  const_iterator end() const
    {return const_iterator(this, dataChunks.size(), 0);}

  size_t size() const
    {return indices->size();}

protected:
  TypePtr elementsType;
  IndexSetPtr indices;
  std::vector< std::pair<VectorPtr, size_t> > dataChunks;
};

typedef ReferenceCountedObjectPtr<LuapeSampleVector> LuapeSampleVectorPtr;

/*
** LuapeSamplesCache
*/
class LuapeSamplesCache : public Object
{
public:
  LuapeSamplesCache(const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSizeInMb = 1024);
  LuapeSamplesCache() : maxCacheSize(0), actualCacheSize(0) {}

  void set(const LuapeNodePtr& node, const VectorPtr& samples);
  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, size_t index, const ObjectPtr& object);
  VectorPtr get(const LuapeNodePtr& node) const;

  // new
  LuapeSampleVectorPtr getSamples(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices, bool isRemoveable = true);
  SparseDoubleVectorPtr getSortedDoubleValues(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices);

  // old
  VectorPtr compute(ExecutionContext& context, const LuapeNodePtr& node, bool isRemoveable = true);

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return inputCaches.size() ? inputCaches[0]->getNumElements() : 0;}

  size_t getCacheSizeInBytes() const
    {return actualCacheSize;}

  bool checkCacheIsCorrect(ExecutionContext& context, const std::vector<LuapeInputNodePtr>& inputs, const LuapeNodePtr& node);

  void getComputeTimeStatistics(ExecutionContext& context) const;

protected:
  // node -> (samples, sorted double values)
  typedef std::map<LuapeNodePtr, std::pair<VectorPtr, SparseDoubleVectorPtr> > NodeToSamplesMap;

//  typedef std::map<LuapeNodePtr, LuapeSampleVectorPtr> NodeToSampleVectorMap;
//  NodeToSampleVectorMap cache;

  std::map<ClassPtr, ScalarVariableStatistics> computingTimeByLuapeFunctionClass;

  NodeToSamplesMap m;
  std::vector<VectorPtr> inputCaches;
  std::deque<LuapeNodePtr> cacheSequence;
  size_t maxCacheSize; // in bytes
  size_t actualCacheSize; // in bytes

  SparseDoubleVectorPtr computeSortedDoubleValues(ExecutionContext& context, const LuapeSampleVectorPtr& samples) const;

  std::pair<VectorPtr, SparseDoubleVectorPtr>& internalCompute(ExecutionContext& context, const LuapeNodePtr& node, bool isRemoveable);
  size_t getSizeInBytes(const VectorPtr& samples) const;
};

typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

/*
** LuapeNodeUniverse
*/
class LuapeNodeUniverse : public Object
{
public:
  void addInputNode(const LuapeInputNodePtr& inputNode)
    {inputNodes.push_back(inputNode);}

  LuapeFunctionNodePtr makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input)
    {return makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));}

  lbcpp_UseDebuggingNewOperator

private:
  friend class LuapeNodeUniverseClass;

  struct FunctionKey
  {
    ClassPtr functionClass;
    std::vector<Variable> arguments;
    std::vector<LuapeNodePtr> inputs;

    bool operator <(const FunctionKey& other) const
    {
      if (functionClass != other.functionClass)
        return functionClass < other.functionClass;
      if (arguments != other.arguments)
        return arguments < other.arguments;
      return inputs < other.inputs;
    }
  };
  typedef std::map<FunctionKey, LuapeFunctionNodePtr> FunctionNodesMap;
  FunctionNodesMap functionNodes;

  std::vector<LuapeInputNodePtr> inputNodes;
};

typedef ReferenceCountedObjectPtr<LuapeNodeUniverse> LuapeNodeUniversePtr;
/*
class LuapeNodeKeysMap : public Object
{
public:
  LuapeNodeKeysMap(LuapeGraphPtr graph = LuapeGraphPtr())
    : graph(graph) {}

  void clear();

  // return true if it is a new node
  bool addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node);

  bool isNodeKeyNew(const LuapeNodePtr& node) const;

  lbcpp_UseDebuggingNewOperator

private:
  typedef std::map<BinaryKeyPtr, LuapeNodePtr, ObjectComparator> KeyToNodeMap;
  typedef std::map<LuapeNodePtr, BinaryKeyPtr> NodeToKeyMap;

  LuapeGraphPtr graph;
  KeyToNodeMap keyToNodes;
  NodeToKeyMap nodeToKeys;

  void addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node);
};

typedef ReferenceCountedObjectPtr<LuapeNodeKeysMap> LuapeNodeKeysMapPtr;
*/

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CACHE_H_

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
  LuapeSamplesCache(const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSizeInMb = 1024);
  LuapeSamplesCache() : maxCacheSize(0), actualCacheSize(0) {}

  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, size_t index, const ObjectPtr& object);


  /*
  ** Cache methods
  */
  void cacheNode(ExecutionContext& context, const LuapeNodePtr& node, const VectorPtr& values = VectorPtr());
  bool isNodeCached(const LuapeNodePtr& node) const;
  VectorPtr getNodeCache(const LuapeNodePtr& node) const;

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return allIndices->size();}

  size_t getCacheSizeInBytes() const
    {return actualCacheSize;}

  bool checkCacheIsCorrect(ExecutionContext& context, const LuapeNodePtr& node);

  /*
  ** Compute operation
  */
  LuapeSampleVectorPtr getSamples(ExecutionContext& context, const LuapeNodePtr& node, const IndexSetPtr& indices, bool isRemoveable = true);

  /*
  ** Misc
  */
  const IndexSetPtr& getAllIndices() const
    {return allIndices;}

  void getComputeTimeStatistics(ExecutionContext& context) const;

protected:
  // node -> (samples, sorted double values)
  typedef std::map<LuapeNodePtr, std::pair<VectorPtr, SparseDoubleVectorPtr> > NodeToSamplesMap;

  std::map<ClassPtr, ScalarVariableStatistics> computingTimeByLuapeFunctionClass;

  NodeToSamplesMap m;
  std::vector<LuapeInputNodePtr> inputNodes;
  std::vector<VectorPtr> inputCaches;
  std::deque<LuapeNodePtr> cacheSequence;
  size_t maxCacheSize; // in bytes
  size_t actualCacheSize; // in bytes

  IndexSetPtr allIndices;

  VectorPtr computeOnAllExamples(ExecutionContext& context, const LuapeNodePtr& node) const;
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

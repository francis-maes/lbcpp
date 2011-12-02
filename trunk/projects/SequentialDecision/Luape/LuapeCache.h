/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

# include "LuapeFunction.h"

namespace lbcpp
{

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;
class LuapeInputNode;
typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;
class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

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

  VectorPtr compute(ExecutionContext& context, const LuapeNodePtr& node, bool isRemoveable = true);
  SparseDoubleVectorPtr getSortedDoubleValues(ExecutionContext& context, const LuapeNodePtr& node, const std::vector<size_t>& examples);

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return inputCaches.size() ? inputCaches[0]->getNumElements() : 0;}

  size_t getCacheSizeInBytes() const
    {return actualCacheSize;}

protected:
  // node -> (samples, sorted double values)
  typedef std::map<LuapeNodePtr, std::pair<VectorPtr, SparseDoubleVectorPtr> > NodeToSamplesMap;

  NodeToSamplesMap m;
  std::vector<VectorPtr> inputCaches;
  std::deque<LuapeNodePtr> cacheSequence;
  size_t maxCacheSize; // in bytes
  size_t actualCacheSize; // in bytes

  SparseDoubleVectorPtr computeSortedDoubleValues(ExecutionContext& context, const VectorPtr& values, const std::vector<size_t>& examples) const;

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

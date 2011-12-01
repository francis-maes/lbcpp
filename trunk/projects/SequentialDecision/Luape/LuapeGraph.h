/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.h                   | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_H_
# define LBCPP_LUAPE_GRAPH_H_

# include "LuapeNode.h"
# include <deque>

namespace lbcpp
{

class LuapeGraphUniverse : public Object
{
public:
  //LuapeGraphUniverse(size_t maxCacheSize = 0)
  //  : maxCacheSize(maxCacheSize) {}

  //void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  void addInputNode(const LuapeInputNodePtr& inputNode)
    {inputNodes.push_back(inputNode);}

  LuapeFunctionNodePtr makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input)
    {return makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));}

  //void cacheUpdated(ExecutionContext& context, const LuapeNodePtr& node, bool isTrainingSamples);

  //void displayCacheInformation(ExecutionContext& context);

  lbcpp_UseDebuggingNewOperator

private:
  friend class LuapeGraphUniverseClass;

  //size_t maxCacheSize;

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

  //std::deque<LuapeNodePtr> trainingCacheSequence;
  //std::deque<LuapeNodePtr> validationCacheSequence;
};

typedef ReferenceCountedObjectPtr<LuapeGraphUniverse> LuapeGraphUniversePtr;

#if 0

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

class LuapeGraph : public Object
{
public:
  LuapeGraph(size_t maxCacheSize);
  LuapeGraph() : numYields(0) {}

  size_t getNumNodes() const
    {return nodes.size();}

  size_t getNumYieldNodes() const
    {return numYields;}

  LuapeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  const std::vector<LuapeNodePtr>& getNodes() const
    {return nodes;}

  TypePtr getNodeType(size_t index) const
    {return getNode(index)->getType();}

  LuapeNodePtr getLastNode() const
    {return nodes.back();}

  bool containsNode(const LuapeNodePtr& node) const
    {return nodesMap.find(node) != nodesMap.end();}

  LuapeNodePtr makeYieldNode()
    {return new LuapeYieldNode(numYields++);}

  LuapeNodePtr pushNode(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushMissingNodes(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushFunctionNode(ExecutionContext& context, const LuapeFunctionPtr& function, const LuapeNodePtr& input);
  void popNode(ExecutionContext& context);
  void removeNode(ExecutionContext& context, size_t nodeIndex, bool alsoRemoveNewUselessNodes = false, bool verbose = false);
  void removeYieldNode(ExecutionContext& context, size_t yieldIndex);

  void computeNodeUseCounts(std::vector<size_t>& res) const;

  VectorPtr updateNodeCache(ExecutionContext& context, const LuapeNodePtr& node, bool isTrainingSamples, SparseDoubleVectorPtr* sortedDoubleValues = NULL);

  size_t getNumTrainingSamples() const;
  size_t getNumValidationSamples() const;
  size_t getNumSamples(bool isTrainingSamples) const;
  void resizeSamples(bool isTrainingSamples, size_t numSamples);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example);
  void setSample(bool isTrainingSample, size_t index, const ObjectPtr& example);
  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

  virtual String toShortString() const;
  String graphToString(size_t firstNodeIndex = 0) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeGraphUniversePtr& getUniverse() const
    {return universe;}

  bool saveToGraphML(ExecutionContext& context, const File& file) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraphClass;

  typedef std::map<LuapeNodePtr, size_t> NodesMap;

  std::vector<LuapeNodePtr> nodes;
  NodesMap nodesMap;
  size_t numYields;

  LuapeGraphUniversePtr universe;
  
  void addNode(const LuapeNodePtr& node);  
};

extern ClassPtr luapeGraphClass;

#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

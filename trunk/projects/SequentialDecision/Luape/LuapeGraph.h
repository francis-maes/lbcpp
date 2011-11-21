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

namespace lbcpp
{

class LuapeGraphUniverse : public Object
{
public:
  void clear(bool clearTrainingSamples = true, bool clearValidationSamples = true, bool clearScores = true);

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true)
    {clear(clearTrainingSamples, clearValidationSamples, false);}

  void clearScores()
    {clear(false, false, true);}

  void addInputNode(const LuapeInputNodePtr& inputNode)
    {inputNodes.push_back(inputNode);}

  LuapeFunctionNodePtr makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input)
    {return makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));}

private:
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

typedef ReferenceCountedObjectPtr<LuapeGraphUniverse> LuapeGraphUniversePtr;

class LuapeNodeKeysMap : public Object
{
public:
  void clear();

  // return true if it is a new node
  bool addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node);

  bool isNodeKeyNew(const LuapeNodePtr& node) const;

private:
  typedef std::map<BinaryKeyPtr, LuapeNodePtr, ObjectComparator> KeyToNodeMap;
  typedef std::map<LuapeNodePtr, BinaryKeyPtr> NodeToKeyMap;

  KeyToNodeMap keyToNodes;
  NodeToKeyMap nodeToKeys;

  void addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node);
};

typedef ReferenceCountedObjectPtr<LuapeNodeKeysMap> LuapeNodeKeysMapPtr;

class LuapeGraph : public Object
{
public:
  LuapeGraph() : universe(new LuapeGraphUniverse()) {}

  size_t getNumNodes() const
    {return nodes.size();}

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

  LuapeNodePtr pushNode(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushMissingNodes(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushFunctionNode(ExecutionContext& context, const LuapeFunctionPtr& function, const LuapeNodePtr& input);
  void popNode();

  size_t getNumTrainingSamples() const;
  size_t getNumValidationSamples() const;
  size_t getNumSamples(bool isTrainingSamples) const;
  void resizeSamples(bool isTrainingSamples, size_t numSamples);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example);
  void setSample(bool isTrainingSample, size_t index, const ObjectPtr& example);
  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);
  void clearScores();

  void compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex = 0, LuapeGraphCallbackPtr callback = 0) const;

  virtual String toShortString() const;
  String graphToString(size_t firstNodeIndex = 0) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeGraphUniversePtr& getUniverse() const
    {return universe;}

protected:
  friend class LuapeGraphClass;

  typedef std::map<LuapeNodePtr, size_t> NodesMap;

  std::vector<LuapeNodePtr> nodes;
  NodesMap nodesMap;

  LuapeGraphUniversePtr universe;
  
  void addNode(const LuapeNodePtr& node);  
};

extern ClassPtr luapeGraphClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

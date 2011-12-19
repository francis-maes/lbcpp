/*-----------------------------------------.---------------------------------.
| Filename: LuapeUniverse.h                | Luape Universe                  |
| Author  : Francis Maes                   |                                 |
| Started : 19/12/2011 12:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_UNIVERSE_H_
# define LBCPP_LUAPE_UNIVERSE_H_

# include "LuapeFunction.h"
# include "../Data/RandomVariable.h"

namespace lbcpp
{

class LuapeNodeUniverse : public Object
{
public:
  void addInputNode(const LuapeInputNodePtr& inputNode)
    {inputNodes.push_back(inputNode);}

  LuapeFunctionNodePtr makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input)
    {return makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));}

  void observeNodeComputingTime(const LuapeNodePtr& node, size_t numInstances, double timeInMilliseconds);
  double getExpectedComputingTime(const LuapeNodePtr& node) const; // in milliseconds

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

  // contain values for LuapeFunctionNode, LuapeTestNode, LuapeSequenceNode
  // keys for LuapeFunctionNode: (luapeFunctionNodeClass, luapeFunctionClass)
  // keys for the others: (luapeNodeClass, ClassPtr())
  std::map<std::pair<ClassPtr, ClassPtr>, ScalarVariableStatistics> nodesComputingTimeStatistics;

  static std::pair<ClassPtr, ClassPtr> makeNodeStatisticsKey(const LuapeNodePtr& node);
};

typedef ReferenceCountedObjectPtr<LuapeNodeUniverse> LuapeNodeUniversePtr;

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
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_UNIVERSE_H_

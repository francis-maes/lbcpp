/*-----------------------------------------.---------------------------------.
| Filename: LuapeUniverse.h                | Luape Universe                  |
| Author  : Francis Maes                   |                                 |
| Started : 19/12/2011 12:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_UNIVERSE_H_
# define LBCPP_LUAPE_UNIVERSE_H_

# include "predeclarations.h"
# include "LuapeFunction.h"
# include "../Data/RandomVariable.h"

namespace lbcpp
{

class LuapeUniverse : public Object
{
public:
  LuapeConstantNodePtr makeConstantNode(const Variable& constantValue);

  LuapeNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);
  LuapeNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input);
  LuapeNodePtr makeFunctionNode(const LuapeFunctionPtr& function, const LuapeNodePtr& input1, const LuapeNodePtr& input2);

  LuapeFunctionPtr makeFunction(ClassPtr functionClass, const std::vector<Variable>& arguments);

  void observeNodeComputingTime(const LuapeNodePtr& node, size_t numInstances, double timeInMilliseconds);
  double getExpectedComputingTime(const LuapeNodePtr& node) const; // in milliseconds

  typedef std::pair<ClassPtr, ClassPtr> NodeTypeKey;

  static NodeTypeKey makeNodeStatisticsKey(const LuapeNodePtr& node);

  virtual LuapeNodePtr canonizeNode(const LuapeNodePtr& node)
    {return node;}


  void getImportances(std::map<LuapeNodePtr, double>& res) const;
  static void getImportances(const LuapeNodePtr& node, std::map<LuapeNodePtr, double>& res);
  static void clearImportances(const LuapeNodePtr& node);
  static void displayMostImportantNodes(ExecutionContext& context, const std::map<LuapeNodePtr, double>& importances);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeNodeUniverseClass;

  typedef std::pair<ClassPtr, std::vector<Variable> >  FunctionKey;
  typedef std::map<FunctionKey, LuapeFunctionPtr> FunctionsMap;
  FunctionsMap functions;

  typedef std::pair<LuapeFunctionPtr, std::vector<LuapeNodePtr> > FunctionNodeKey;
  typedef std::map<FunctionNodeKey, LuapeNodePtr> FunctionNodesMap;
  FunctionNodesMap functionNodes;

  typedef std::map<Variable, LuapeConstantNodePtr> ConstantNodesMap;
  ConstantNodesMap constantNodes;

  // contain values for LuapeFunctionNode, LuapeTestNode, LuapeSequenceNode
  // keys for LuapeFunctionNode: (luapeFunctionNodeClass, luapeFunctionClass)
  // keys for the others: (luapeNodeClass, ClassPtr())
  std::map<std::pair<ClassPtr, ClassPtr>, ScalarVariableStatistics> nodesComputingTimeStatistics;
};

typedef ReferenceCountedObjectPtr<LuapeUniverse> LuapeUniversePtr;

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

/*-----------------------------------------.---------------------------------.
| Filename: ExpressionUniverse.h           | Expression Universe             |
| Author  : Francis Maes                   |                                 |
| Started : 19/12/2011 12:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_UNIVERSE_H_
# define LBCPP_LUAPE_UNIVERSE_H_

# include "predeclarations.h"
# include "Function.h"
# include "Expression.h"
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class ExpressionUniverse : public Object
{
public:
  ExpressionUniverse();

  ConstantExpressionPtr makeConstantNode(const ObjectPtr& constantValue);

  ConstantExpressionPtr makeConstantNode(const Variable& constantValue) // compatibility
    {return makeConstantNode(constantValue.toObject());}

  ExpressionPtr makeFunctionExpression(const FunctionPtr& function, const std::vector<ExpressionPtr>& inputs);
  ExpressionPtr makeFunctionExpression(const FunctionPtr& function, const ExpressionPtr& input);
  ExpressionPtr makeFunctionExpression(const FunctionPtr& function, const ExpressionPtr& input1, const ExpressionPtr& input2);

  FunctionPtr makeFunction(ClassPtr functionClass, const std::vector<Variable>& arguments);

  void observeNodeComputingTime(const ExpressionPtr& node, size_t numInstances, double timeInMilliseconds);
  double getExpectedComputingTime(const ExpressionPtr& node) const; // in milliseconds

  typedef std::pair<ClassPtr, ClassPtr> NodeTypeKey;

  static NodeTypeKey makeNodeStatisticsKey(const ExpressionPtr& node);

  virtual ExpressionPtr canonizeNode(const ExpressionPtr& node)
    {return node;}


  void clearImportances();
  void getImportances(std::map<ExpressionPtr, double>& res) const;

  static void getImportances(const ExpressionPtr& node, std::map<ExpressionPtr, double>& res);
  static void clearImportances(const ExpressionPtr& node);
  static void displayMostImportantNodes(ExecutionContext& context, const std::map<ExpressionPtr, double>& importances);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExpressionUniverseClass;

  typedef std::pair<ClassPtr, std::vector<Variable> >  FunctionKey;
  typedef std::map<FunctionKey, FunctionPtr> FunctionsMap;
  FunctionsMap functions;

  typedef std::pair<FunctionPtr, std::vector<ExpressionPtr> > FunctionExpressionKey;
  typedef std::map<FunctionExpressionKey, ExpressionPtr> FunctionExpressionsMap;
  FunctionExpressionsMap functionNodes;
  size_t maxFunctionDepth;
  size_t maxObservedFunctionDepth;

  typedef std::map<ObjectPtr, ConstantExpressionPtr> ConstantNodesMap;
  ConstantNodesMap constantNodes;

  void cacheFunctionExpression(const FunctionExpressionKey& key, ExpressionPtr node);

  // contain values for FunctionExpression, TestExpression, SequenceExpression
  // keys for FunctionExpression: (functionExpressionClass, luapeFunctionClass)
  // keys for the others: (expressionClass, ClassPtr())

  std::map<std::pair<ClassPtr, ClassPtr>, ScalarVariableStatistics> nodesComputingTimeStatistics;
};

typedef ReferenceCountedObjectPtr<ExpressionUniverse> ExpressionUniversePtr;

#if 0
class ExpressionKeysMap : public Object
{
public:
  ExpressionKeysMap(LuapeGraphPtr graph = LuapeGraphPtr())
    : graph(graph) {}

  void clear();

  // return true if it is a new node
  bool addNodeToCache(ExecutionContext& context, const ExpressionPtr& node);

  bool isNodeKeyNew(const ExpressionPtr& node) const;

  lbcpp_UseDebuggingNewOperator

private:
  typedef std::map<BinaryKeyPtr, ExpressionPtr, ObjectComparator> KeyToNodeMap;
  typedef std::map<ExpressionPtr, BinaryKeyPtr> NodeToKeyMap;

  LuapeGraphPtr graph;
  KeyToNodeMap keyToNodes;
  NodeToKeyMap nodeToKeys;

  void addSubNodesToCache(ExecutionContext& context, const ExpressionPtr& node);
};

typedef ReferenceCountedObjectPtr<ExpressionKeysMap> ExpressionKeysMapPtr;
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_UNIVERSE_H_

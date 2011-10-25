/*-----------------------------------------.---------------------------------.
| Filename: LuapeProblem.h                 | Lua Program Evolution Problem   |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PROBLEM_H_
# define LBCPP_LUAPE_PROBLEM_H_

# include <lbcpp/Lua/Lua.h>
# include "../Core/DecisionProblem.h"

namespace lbcpp
{

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache() : convertibleToDouble(false) {}

  void initialize(TypePtr type)
  {
    examples = vector(type, 0);
    convertibleToDouble = type->isConvertibleToDouble();
  }
  
  void reserveExamples(size_t count)
    {examples->reserve(count);}

  void addExample(const Variable& value)
  {
    jassert(examples);
    jassert(value.exists());

    examples->append(value);
    if (convertibleToDouble)
      doubleValues.insert(value.toDouble());
  }

  size_t getNumExamples() const
    {return examples->getNumElements();}

  Variable getExample(size_t index) const
    {return examples->getElement(index);}

protected:
  VectorPtr examples;
  bool convertibleToDouble;
  std::set<double> doubleValues; // only if isConvertibleToDouble
  Variable objectiveToMinimize;
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;


//////  GRAPH NODES

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeNode : public NameableObject
{
public:
  LuapeNode(const TypePtr& type, const String& name)
    : NameableObject(name), type(type) {}
  LuapeNode() {}

  const TypePtr& getType() const
    {return type;}

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes)
  {
    if (!type)
      return false;
    if (!cache)
    {
      cache = new LuapeNodeCache();
      cache->initialize(type);
    }
    return true;
  }

  VariableSignaturePtr getSignature() const
    {return new VariableSignature(type, name);}

  void reserveExamples(size_t count)
    {jassert(cache); cache->reserveExamples(count);}

  void addExample(const Variable& value)
  {
    jassert(cache);
    jassert(value.getType()->inheritsFrom(type));
    cache->addExample(value);
  }

  const LuapeNodeCachePtr& getCache() const
    {return cache;}

protected:
  friend class LuapeNodeClass;

  TypePtr type;
  LuapeNodeCachePtr cache;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name)
    : LuapeNode(type, name) {}
  LuapeInputNode() {}

  virtual String toShortString() const
    {return T("input ") + getName() + T(" (type = ") + type->getName() + T(")");}
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const FunctionPtr& function, const std::vector<size_t>& arguments)
    : function(function->cloneAndCast<Function>()), arguments(arguments) {}
  LuapeFunctionNode() {}

  virtual String toShortString() const
  {
    String res = function->toShortString() + T("(");
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      res += String((int)arguments[i]);
      if (i < arguments.size() - 1)
        res += T(", ");
    }
    return res + T(")");
  }

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes)
  {
    std::vector<VariableSignaturePtr> inputs(arguments.size());
    inputNodes.resize(arguments.size());
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      inputNodes[i] = allNodes[arguments[i]];
      inputs[i] = inputNodes[i]->getSignature();
    }
    if (!function->initialize(context, inputs))
      return false;
    type = function->getOutputType();
    name = function->getOutputVariable()->getName();
    if (!LuapeNode::initialize(context, allNodes))
      return false;
    propagateCache(context);
    return true;
  }

  void propagateCache(ExecutionContext& context)
  {
    jassert(inputNodes.size());
    size_t minCacheSize = inputNodes[0]->getCache()->getNumExamples();
    for (size_t i = 1; i < inputNodes.size(); ++i)
    {
      size_t cacheSize = inputNodes[i]->getCache()->getNumExamples();
      if (cacheSize < minCacheSize)
        minCacheSize = cacheSize;
    }

    cache->reserveExamples(minCacheSize);    
    for (size_t i = 0; i < minCacheSize; ++i)
    {
      std::vector<Variable> inputs(inputNodes.size());
      for (size_t j = 0; j < inputs.size(); ++j)
        inputs[j] = inputNodes[j]->getCache()->getExample(i);
      cache->addExample(function->compute(context, inputs));
    }
  }

protected:
  friend class LuapeFunctionNodeClass;

  FunctionPtr function;
  std::vector<size_t> arguments;

  std::vector<LuapeNodePtr> inputNodes;
};

typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(size_t argument = 0)
    : argument(argument) {}

  virtual String toShortString() const
    {return T("yield(") + String((int)argument) + T(")");}

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes)
  {
    inputNode = allNodes[argument];
    type = inputNode->getType();
    name = inputNode->getName();
    return LuapeNode::initialize(context, allNodes);
  }

  size_t getArgument() const
    {return argument;}

protected:
  friend class LuapeYieldNodeClass;

  size_t argument;
  LuapeNodePtr inputNode;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;

//////  GRAPH

class LuapeGraph : public Object
{
public:
  LuapeGraph() : numExamples(0) {}

  virtual String toShortString() const
    {return graphToString(0);}
  
  String graphToString(size_t firstNodeIndex = 0) const
  {
    String res;
    for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
      res += String((int)i) + T(" ") + nodes[i]->toShortString() + T("\n");
    return res;
  }

  size_t getNumNodes() const
    {return nodes.size();}

  LuapeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  TypePtr getNodeType(size_t index) const
    {return getNode(index)->getType();}

  LuapeNodePtr getLastNode() const
    {return nodes.back();}

  bool pushNode(ExecutionContext& context, const LuapeNodePtr& node)
  {
    if (!node->initialize(context, nodes))
      return false;
    nodes.push_back(node);
    return true;
  }

  void popNode()
    {jassert(nodes.size()); nodes.pop_back();}

  void reserveExamples(size_t count)
  {
    for (size_t i = 0; i < nodes.size(); ++i)
      nodes[i]->reserveExamples(count);
  }

  void addExample(const std::vector<Variable>& example)
  {
    jassert(example.size() <= nodes.size());
    for (size_t i = 0; i < example.size(); ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->addExample(example[i]);
    }
    ++numExamples;
  }

protected:
  friend class LuapeGraphClass;

  std::vector<LuapeNodePtr> nodes;
  size_t numExamples;
};

typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;
extern ClassPtr luapeGraphClass;

////////// CACHE

class LuapeGraphCache : public Object
{
};

typedef ReferenceCountedObjectPtr<LuapeGraphCache> LuapeGraphCachePtr;

////////// PROBLEM

class LuapeProblem : public Object
{
public:
  LuapeProblem() : failed(false) {}

  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new VariableSignature(type, name));}

  void addFunction(const FunctionPtr& function)
    {functions.push_back(function);}

  size_t getNumFunctions() const
    {return functions.size();}

  FunctionPtr getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  LuapeGraphPtr createInitialGraph(ExecutionContext& context) const
  {
    LuapeGraphPtr res = new LuapeGraph();
    for (size_t i = 0; i < inputs.size(); ++i)
      res->pushNode(context, new LuapeInputNode(inputs[i]->getType(), inputs[i]->getName()));
    return res;
  }

  static int input(LuaState& state);
  static int function(LuaState& state);
  //static int objective(LuaState& state);
  
protected:
  friend class LuapeProblemClass;

  std::vector<VariableSignaturePtr> inputs;
  std::vector<FunctionPtr> functions;
  bool failed;
};

extern ClassPtr luapeProblemClass;
typedef ReferenceCountedObjectPtr<LuapeProblem> LuapeProblemPtr;

///////// Graph Builder

class LuapeGraphBuilderState : public DecisionProblemState
{
public:
  LuapeGraphBuilderState(const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeGraphCachePtr& cache, size_t maxSteps)
    : problem(problem), graph(graph), cache(cache), maxSteps(maxSteps), numSteps(0) {}
  LuapeGraphBuilderState() : maxSteps(0), numSteps(0) {}

  virtual String toShortString() const
    {return graph->graphToString(graph->getNumNodes() - numSteps);}

  virtual TypePtr getActionType() const
    {return luapeNodeClass;}

  virtual ContainerPtr getAvailableActions() const
  {
    if (numSteps >= maxSteps)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(luapeNodeClass, 0);

    // accessor actions
    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      TypePtr nodeType = graph->getNodeType(i);
      if (nodeType->inheritsFrom(objectClass))
      {
        std::vector<size_t> arguments(1, i);
        size_t nv = nodeType->getNumMemberVariables();
        for (size_t j = 0; j < nv; ++j)
          res->append(new LuapeFunctionNode(getVariableFunction(j), arguments));
      }
    }
    
    // function actions
    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      FunctionPtr function = problem->getFunction(i);
      std::vector<size_t> arguments;
      enumerateFunctionActionsRecursively(function, arguments, res);
    }

    // todo: stump actions

    // yield actions
    for (size_t i = 0; i < n; ++i)
      if (graph->getNodeType(i) == booleanType)
        res->append(new LuapeYieldNode(i));

    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    const LuapeNodePtr& node = action.getObjectAndCast<LuapeNode>();
    bool ok = graph->pushNode(context, node);
    jassert(ok);
    reward = 0.0;
    ++numSteps;
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    --numSteps;
    graph->popNode();
    return true;
  }

  virtual bool isFinalState() const
    {return numSteps >= maxSteps || (numSteps && graph->getLastNode().isInstanceOf<LuapeYieldNode>());}

  LuapeGraphPtr getGraph() const
    {return graph;}

protected:
  friend class LuapeGraphBuilderStateClass;

  LuapeProblemPtr problem;
  LuapeGraphPtr graph;
  LuapeGraphCachePtr cache;
  size_t maxSteps;

  size_t numSteps;

  void enumerateFunctionActionsRecursively(const FunctionPtr& function, std::vector<size_t>& arguments, const ObjectVectorPtr& res) const
  {
    size_t expectedNumArguments = function->getNumRequiredInputs();
    if (arguments.size() == expectedNumArguments)
      res->append(new LuapeFunctionNode(function, arguments));
    else
    {
      jassert(arguments.size() < expectedNumArguments);
      TypePtr expectedType = function->getRequiredInputType(arguments.size(), expectedNumArguments);
      size_t n = graph->getNumNodes();
      for (size_t i = 0; i < n; ++i)
        if (graph->getNodeType(i)->inheritsFrom(expectedType))
        {
          arguments.push_back(i);
          enumerateFunctionActionsRecursively(function, arguments, res);
          arguments.pop_back();
        }
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderState> LuapeGraphBuilderStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PROBLEM_H_

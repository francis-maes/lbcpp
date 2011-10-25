/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.h                   | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_H_
# define LBCPP_LUAPE_GRAPH_H_

namespace lbcpp
{

class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;


class LuapeGraphCallback
{
public:
  virtual ~LuapeGraphCallback() {}

  virtual void valueYielded(const Variable& value) {}
};

typedef LuapeGraphCallback* LuapeGraphCallbackPtr;


class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache() : convertibleToDouble(false), score(0.0) {}

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

  const VectorPtr& getExamples() const
    {return examples;}

  void setScore(double score)
    {this->score = score;}

  double getScore() const
    {return score;}

protected:
  VectorPtr examples;
  bool convertibleToDouble;
  std::set<double> doubleValues; // only if isConvertibleToDouble
  double score; // only for 'yield' nodes
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

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const = 0;

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

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const LuapeNodePtr& target = t.staticCast<LuapeNode>();
    target->name = name;
    target->type = type;
    target->cache = cache;
  }

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
    {return T("input ") + getName();}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
    {jassert(false); return Variable();}
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

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
      if (inputNodes.size())
        res += inputNodes[i]->toShortString();
      else
        res += T("node ") + String((int)arguments[i]);
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

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {
    size_t n = arguments.size();
    std::vector<Variable> inputs(n);
    for (size_t i = 0; i < n; ++i)
      inputs[i] = state[arguments[i]];
    return function->compute(context, inputs);
  }

  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    LuapeNode::clone(context, t);
    const LuapeFunctionNodePtr& target = t.staticCast<LuapeNode>();
    target->function = function;
    target->arguments = arguments;
  }

protected:
  friend class LuapeFunctionNodeClass;

  FunctionPtr function;
  std::vector<size_t> arguments;

  std::vector<LuapeNodePtr> inputNodes;

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

    size_t currentSize = cache->getNumExamples();
    if (minCacheSize > currentSize)
    {
      cache->reserveExamples(minCacheSize);    
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        std::vector<Variable> inputs(inputNodes.size());
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = inputNodes[j]->getCache()->getExample(i);
        cache->addExample(function->compute(context, inputs));
      }
    }
  }
};

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(size_t argument = 0)
    : argument(argument) {}

  virtual String toShortString() const
    {return T("yield(") + (inputNode ? inputNode->toShortString() : String((int)argument)) + T(")");}

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes)
  {
    inputNode = allNodes[argument];
    type = nilType;
    name = inputNode->getName();
    return LuapeNode::initialize(context, allNodes);
  }

  size_t getArgument() const
    {return argument;}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {
    if (callback)
      callback->valueYielded(state[argument]);
    return state[argument];
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    LuapeNode::clone(context, t);
    t.staticCast<LuapeYieldNode>()->argument = argument;
  }

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

  void addExample(const ContainerPtr& example)
  {
    size_t n = example->getNumElements();
    jassert(n <= nodes.size());
    for (size_t i = 0; i < n; ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->addExample(example->getElement(i));
    }
    ++numExamples;
  }

  void compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex = 0, LuapeGraphCallbackPtr callback = 0) const
  {
    for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
      state[i] = nodes[i]->compute(context, state, callback);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const LuapeGraphPtr& target = t.staticCast<LuapeGraph>();
    target->numExamples = numExamples;
    size_t n = nodes.size();
    target->nodes.reserve(n);
    for (size_t i = 0; i < n; ++i)
      target->pushNode(context, nodes[i]->cloneAndCast<LuapeNode>());      
  }

protected:
  friend class LuapeGraphClass;

  std::vector<LuapeNodePtr> nodes;
  size_t numExamples;
};

extern ClassPtr luapeGraphClass;

////////// CACHE

class LuapeGraphCache : public Object
{
};

typedef ReferenceCountedObjectPtr<LuapeGraphCache> LuapeGraphCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

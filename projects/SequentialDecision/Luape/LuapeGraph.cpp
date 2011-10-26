/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.cpp                 | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2011 00:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeGraph.h"
using namespace lbcpp;

/*
** LuapeNodeCache
*/
LuapeNodeCache::LuapeNodeCache() 
  : convertibleToDouble(false), scoreComputed(false), score(0.0)
{
}

void LuapeNodeCache::initialize(TypePtr type)
{
  examples = vector(type, 0);
  convertibleToDouble = type->isConvertibleToDouble();
}
 
void LuapeNodeCache::setExample(size_t index, const Variable& value)
{
  jassert(examples);
  jassert(value.exists());

  examples->setElement(index, value);
  if (convertibleToDouble)
    doubleValues.insert(value.toDouble());
}

/*
** LuapeGraphCache
*/
LuapeNodeCachePtr LuapeGraphCache::getNodeCache(const LuapeNodeKey& key, const TypePtr& nodeType)
{
  ScopedLock _(lock);
  CacheMap::iterator it = m.find(key);
  if (it == m.end())
  {
    LuapeNodeCachePtr res(new LuapeNodeCache());
    res->initialize(nodeType);
    m[key] = res;
    return res;
  }
  else
    return it->second;
}

void LuapeGraphCache::clearScores()
{
  ScopedLock _(lock);
  for (CacheMap::const_iterator it = m.begin(); it != m.end(); ++it)
    it->second->clearScore();
}

/*
** LuapeNode
*/
LuapeNode::LuapeNode(const TypePtr& type, const String& name)
  : NameableObject(name), type(type) {}

bool LuapeNode::initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& graphCache)
{
  if (!type)
    return false;
  if (!cache)
  {
    LuapeNodeKey key;
    fillKey(allNodes, key);
    if (graphCache)
      cache = graphCache->getNodeCache(key, type);
    else
    {
      cache = new LuapeNodeCache();
      cache->initialize(type);
    }
  }
  return true;
}

void LuapeNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeNodePtr& target = t.staticCast<LuapeNode>();
  target->name = name;
  target->type = type;
  target->cache = cache;
}

/*
** LuapeInputNode
*/
LuapeInputNode::LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex)
  : LuapeNode(type, name), inputIndex(inputIndex) {}

String LuapeInputNode::toShortString() const
  {return T("input ") + getName();}

Variable LuapeInputNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {jassert(false); return Variable();}

void LuapeInputNode::fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const
  {return res.push_back((juce::int64)inputIndex);}

void LuapeInputNode::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  target.staticCast<LuapeInputNode>()->inputIndex = inputIndex;
  LuapeNode::clone(context, target);
}

/*
** LuapeFunctionNode
*/
LuapeFunctionNode::LuapeFunctionNode(const FunctionPtr& function, const std::vector<size_t>& arguments)
  : function(function->cloneAndCast<Function>()), arguments(arguments) {}

String LuapeFunctionNode::toShortString() const
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

bool LuapeFunctionNode::initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& graphCache)
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
  if (!LuapeNode::initialize(context, allNodes, graphCache))
    return false;
  propagateCache(context);
  return true;
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  size_t n = arguments.size();
  std::vector<Variable> inputs(n);
  for (size_t i = 0; i < n; ++i)
    inputs[i] = state[arguments[i]];
  return function->compute(context, inputs);
}

void LuapeFunctionNode::fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const
{
  res.push_back((juce::int64)function->getClass().get());
  for (size_t i = 0; i < function->getNumVariables(); ++i)
  {
    Variable value = function->getVariable(i);
    if (value.isInteger())
      res.push_back((juce::int64)value.getInteger());
    else if (value.isConvertibleToDouble())
      res.push_back((juce::int64)(value.toDouble() * 1e6));
    else
      jassert(false);
  }
  for (size_t i = 0; i < arguments.size(); ++i)
    allNodes[arguments[i]]->fillKey(allNodes, res);
}

void LuapeFunctionNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  const LuapeFunctionNodePtr& target = t.staticCast<LuapeNode>();
  target->function = function;
  target->arguments = arguments;
}

void LuapeFunctionNode::propagateCache(ExecutionContext& context)
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
    cache->resizeExamples(minCacheSize);    
    std::vector<Variable> inputs(inputNodes.size());
    for (size_t i = currentSize; i < minCacheSize; ++i)
    {
      for (size_t j = 0; j < inputs.size(); ++j)
        inputs[j] = inputNodes[j]->getCache()->getExample(i);
      cache->setExample(i, function->compute(context, inputs));
    }
  }
}

/*
** LuapeYieldNode
*/
LuapeYieldNode::LuapeYieldNode(size_t argument)
  : argument(argument) {}

String LuapeYieldNode::toShortString() const
  {return T("yield(") + (inputNode ? inputNode->toShortString() : String((int)argument)) + T(")");}

bool LuapeYieldNode::initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& graphCache)
{
  inputNode = allNodes[argument];
  type = nilType;
  name = inputNode->getName();
  return LuapeNode::initialize(context, allNodes, graphCache);
}

Variable LuapeYieldNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  if (callback)
    callback->valueYielded(state[argument]);
  return state[argument];
}

void LuapeYieldNode::fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const
  {res.push_back(0); allNodes[argument]->fillKey(allNodes, res);}

void LuapeYieldNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  t.staticCast<LuapeYieldNode>()->argument = argument;
}

/*
** LuapeGraph
*/
LuapeGraph::LuapeGraph(bool useCache)
  : numExamples(0)
{
  if (useCache)
    cache = new LuapeGraphCache();
}

String LuapeGraph::toShortString() const
  {return graphToString(0);}

String LuapeGraph::graphToString(size_t firstNodeIndex) const
{
  String res;
  for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
    res += String((int)i) + T(" ") + nodes[i]->toShortString() + T("\n");
  return res;
}

bool LuapeGraph::pushNode(ExecutionContext& context, const LuapeNodePtr& node)
{
  if (!node->initialize(context, nodes, cache))
    return false;
  nodes.push_back(node);
  return true;
}

void LuapeGraph::popNode()
  {jassert(nodes.size()); nodes.pop_back();}

void LuapeGraph::resizeExamples(size_t count)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->getCache()->resizeExamples(count);
}

void LuapeGraph::setExample(size_t index, const std::vector<Variable>& example)
{
  jassert(example.size() <= nodes.size());
  for (size_t i = 0; i < example.size(); ++i)
  {
    LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
    jassert(node);
    node->getCache()->setExample(index, example[i]);
  }
  ++numExamples;
}

void LuapeGraph::setExample(size_t index, const ObjectPtr& example)
{
  ContainerPtr container = example.dynamicCast<Container>();
  if (container)
  {
    size_t n = container->getNumElements();
    jassert(n <= nodes.size());
    for (size_t i = 0; i < n; ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->getCache()->setExample(index, container->getElement(i));
    }
  }
  else
  {
    size_t n = example->getNumVariables();
    jassert(n <= nodes.size());
    for (size_t i = 0; i < n; ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->getCache()->setExample(index, example->getVariable(i));
    }
  }
  ++numExamples;
}

void LuapeGraph::compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex, LuapeGraphCallbackPtr callback) const
{
  for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
    state[i] = nodes[i]->compute(context, state, callback);
}

void LuapeGraph::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeGraphPtr& target = t.staticCast<LuapeGraph>();
  target->numExamples = numExamples;
  size_t n = nodes.size();
  target->nodes.reserve(n);
  for (size_t i = 0; i < n; ++i)
    target->pushNode(context, nodes[i]->cloneAndCast<LuapeNode>());
  target->cache = cache;
}

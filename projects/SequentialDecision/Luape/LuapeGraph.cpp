/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.cpp                 | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2011 00:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeGraph.h"
#include <algorithm>
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
  trainingSamples = vector(type, 0);
  validationSamples = vector(type, 0);
  convertibleToDouble = type->isConvertibleToDouble();
}

void LuapeNodeCache::clear()
{
  scoreComputed = false;
  score = 0.0;
  TypePtr type = trainingSamples->getElementsType();
  initialize(type);
}

void LuapeNodeCache::resizeSamples(bool isTrainingSamples, size_t size)
  {(isTrainingSamples ? trainingSamples : validationSamples)->resize(size);}

void LuapeNodeCache::resizeSamples(size_t numTrainingSamples, size_t numValidationSamples)
{
  trainingSamples->resize(numTrainingSamples);
  validationSamples->resize(numValidationSamples);
}

void LuapeNodeCache::setSample(bool isTrainingSample, size_t index, const Variable& value)
  {(isTrainingSample ? trainingSamples : validationSamples)->setElement(index, value);}

void LuapeNodeCache::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  if (clearTrainingSamples)
  {
    trainingSamples->clear();
    sortedDoubleValues.clear();
  }
  if (clearValidationSamples)
    validationSamples->clear();
}

struct SortDoubleValuesOperator
{
  bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
    {return a.second == b.second ? a.first < b.first : a.second < b.second;}
};

const std::vector< std::pair<size_t, double> >& LuapeNodeCache::getSortedDoubleValues() const
{
  size_t n = trainingSamples->getNumElements();
  if (sortedDoubleValues.size() < n)
  {
    std::vector< std::pair<size_t, double> >& v = const_cast<LuapeNodeCache* >(this)->sortedDoubleValues;
    v.resize(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = std::make_pair(i, trainingSamples->getElement(i).toDouble());
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  }
  return sortedDoubleValues;
}

BinaryKeyPtr LuapeNodeCache::makeKeyFromSamples(bool useTrainingSamples) const
{
  ContainerPtr samples = getSamples(useTrainingSamples);
  size_t n = samples->getNumElements();
  
  BooleanVectorPtr booleanVector = samples.dynamicCast<BooleanVector>();
  if (booleanVector)
  {
    BinaryKeyPtr res = new BinaryKey(1 + n / 8);
    for (size_t i = 0; i < n; ++i)
      res->pushBit(booleanVector->get(i));
    res->fillBits();
    return res;
  }

  DenseDoubleVectorPtr doubleVector = samples.dynamicCast<DenseDoubleVector>();
  if (doubleVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * 4);
    for (size_t i = 0; i < n; ++i)
      res->push32BitInteger((int)(doubleVector->getValue(i) * 10e6));
    return res;
  }

  ObjectVectorPtr objectVector = samples.dynamicCast<ObjectVector>();
  if (objectVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * sizeof (void* ));
    for (size_t i = 0; i < n; ++i)
      res->pushPointer(objectVector->get(i));
    return res;
  }

  GenericVectorPtr genericVector = samples.dynamicCast<GenericVector>();
  if (genericVector)
  {
    if (genericVector->getElementsType()->inheritsFrom(integerType))
    {
      BinaryKeyPtr res = new BinaryKey(n * sizeof (VariableValue));
      res->pushBytes((unsigned char* )&genericVector->getValues()[0], n * sizeof (VariableValue));
      return res;
    }

    jassert(false);
  }

  jassert(false);
  return BinaryKeyPtr();
}

/*
** LuapeNode
*/
LuapeNode::LuapeNode(const TypePtr& type, const String& name)
  : NameableObject(name), type(type), cache(new LuapeNodeCache()), indexInGraph((size_t)-1)
{
  cache->initialize(type);
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
  {return getName();}

Variable LuapeInputNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {jassert(false); return Variable();}

void LuapeInputNode::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  target.staticCast<LuapeInputNode>()->inputIndex = inputIndex;
  LuapeNode::clone(context, target);
}

/*
** LuapeFunctionNode
*/
LuapeFunctionNode::LuapeFunctionNode(const FunctionPtr& function, const std::vector<LuapeNodePtr>& arguments)
  : function(function->cloneAndCast<Function>()), arguments(arguments)
{
  initializeFunction(defaultExecutionContext());
}

LuapeFunctionNode::LuapeFunctionNode(const FunctionPtr& function, LuapeNodePtr argument)
  : function(function->cloneAndCast<Function>()), arguments(1, argument)
{
  initializeFunction(defaultExecutionContext());
}

String LuapeFunctionNode::toShortString() const
{
  String className = function->getClassName();
  if (className == T("GetVariableFunction"))
    return arguments[0]->toShortString() + "." + arguments[0]->getType()->getMemberVariableName(function->getVariable(0).getInteger());
  else if (className == T("BooleanAndFunction"))
    return arguments[0]->toShortString() + T(" && ") + arguments[1]->toShortString();
  else if (className == T("EqualsEnumValueFunction"))
    return arguments[0]->toShortString() + T(" == ") + function->getVariable(0).toShortString();
  else
  {
    String res = function->toShortString() + T("(");
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      res += arguments[i]->toShortString();
      if (i < arguments.size() - 1)
        res += T(", ");
    }
    return res + T(")");
  }
}

bool LuapeFunctionNode::initializeFunction(ExecutionContext& context)
{
  std::vector<VariableSignaturePtr> inputs(arguments.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    inputs[i] = arguments[i]->getSignature();
  if (!function->initialize(context, inputs))
    return false;
  type = function->getOutputType();
  name = function->getOutputVariable()->getName();
  cache = new LuapeNodeCache();
  cache->initialize(type);
  return true;
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  size_t n = arguments.size();
  std::vector<Variable> inputs(n);
  for (size_t i = 0; i < n; ++i)
    inputs[i] = state[arguments[i]->getIndexInGraph()];
  return function->compute(context, inputs);
}

void LuapeFunctionNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  const LuapeFunctionNodePtr& target = t.staticCast<LuapeNode>();
  target->function = function;
  target->arguments = arguments;
}

size_t LuapeFunctionNode::getDepth() const
{
  size_t maxInputDepth = 0;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    size_t d = arguments[i]->getDepth();
    if (d > maxInputDepth)
      maxInputDepth = d;
  }
  return maxInputDepth + 1;
}

void LuapeFunctionNode::updateCache(ExecutionContext& context, bool isTrainingSamples)
{
  jassert(arguments.size());
  TypePtr inputBaseType = arguments[0]->getType();
  arguments[0]->updateCache(context, isTrainingSamples);
  size_t minCacheSize = arguments[0]->getCache()->getNumSamples(isTrainingSamples);
  for (size_t i = 1; i < arguments.size(); ++i)
  {
    arguments[i]->updateCache(context, isTrainingSamples);
    size_t cacheSize = arguments[i]->getCache()->getNumSamples(isTrainingSamples);
    if (cacheSize < minCacheSize)
      minCacheSize = cacheSize;
    inputBaseType = Type::findCommonBaseType(inputBaseType, arguments[i]->getType());
  }

  size_t currentSize = cache->getNumSamples(isTrainingSamples);
  if (minCacheSize > currentSize)
  {
    cache->resizeSamples(isTrainingSamples, minCacheSize);
    std::vector<Variable> inputs(arguments.size());

    if (inputBaseType == doubleType)
    {
      std::vector<DenseDoubleVectorPtr> inputCaches(arguments.size());
      for (size_t i = 0; i < inputCaches.size(); ++i)
        inputCaches[i] = arguments[i]->getCache()->getSamples(isTrainingSamples).staticCast<DenseDoubleVector>();
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = inputCaches[j]->getValue(i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
    else if (inputBaseType == booleanType)
    {
      std::vector<BooleanVectorPtr> inputCaches(arguments.size());
      for (size_t i = 0; i < inputCaches.size(); ++i)
        inputCaches[i] = arguments[i]->getCache()->getSamples(isTrainingSamples).staticCast<BooleanVector>();
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = inputCaches[j]->get(i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
    else
    {
      for (size_t i = currentSize; i < minCacheSize; ++i)
      {
        for (size_t j = 0; j < inputs.size(); ++j)
          inputs[j] = arguments[j]->getCache()->getSample(isTrainingSamples, i);
        cache->setSample(isTrainingSamples, i, function->compute(context, inputs));
      }
    }
  }
}

/*
** LuapeYieldNode
*/
LuapeYieldNode::LuapeYieldNode(const LuapeNodePtr& argument)
  : LuapeNode(nilType, argument->getName()), argument(argument)
{
}

String LuapeYieldNode::toShortString() const
  {return T("yield(") + argument->toShortString() + T(")");}

Variable LuapeYieldNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  if (callback)
    callback->valueYielded(state[argument]);
  return state[argument];
}

size_t LuapeYieldNode::getDepth() const
  {return argument->getDepth() + 1;}

void LuapeYieldNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  t.staticCast<LuapeYieldNode>()->argument = argument;
}

/*
** LuapeGraphUniverse
*/
static void clearNode(const LuapeNodePtr& node, bool clearTrainingSamples, bool clearValidationSamples, bool clearScores)
{
  const LuapeNodeCachePtr& cache = node->getCache();
  cache->clearSamples(clearTrainingSamples, clearValidationSamples);
  if (clearScores)
    cache->clearScore();
}

void LuapeGraphUniverse::clear(bool clearTrainingSamples, bool clearValidationSamples, bool clearScores)
{
  for (size_t i = 0; i < inputNodes.size(); ++i)
    clearNode(inputNodes[i], clearTrainingSamples, clearValidationSamples, clearScores);
  for (FunctionNodesMap::const_iterator it = functionNodes.begin(); it != functionNodes.end(); ++it)
    clearNode(it->second, clearTrainingSamples, clearValidationSamples, clearScores);
}

LuapeFunctionNodePtr LuapeGraphUniverse::makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs)
{
  FunctionKey key;
  key.functionClass = functionClass;
  key.arguments = arguments;
  key.inputs = inputs;
  FunctionNodesMap::const_iterator it = functionNodes.find(key);
  if (it == functionNodes.end())
  {
    FunctionPtr function = Function::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    LuapeFunctionNodePtr res = new LuapeFunctionNode(function, inputs);
    functionNodes[key] = res;
    return res;
  }
  else
    return it->second;
}

LuapeFunctionNodePtr LuapeGraphUniverse::makeFunctionNode(const FunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
{
  std::vector<Variable> arguments(function->getNumVariables());
  for (size_t i = 0; i < arguments.size(); ++i)
    arguments[i] = function->getVariable(i);
  return makeFunctionNode(function->getClass(), arguments, inputs);
}

/*
** LuapeGraph
*/
String LuapeGraph::toShortString() const
  {return graphToString(0);}

String LuapeGraph::graphToString(size_t firstNodeIndex) const
{
  String res;
  for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
    res += String((int)i) + T(" ") + nodes[i]->toShortString() + T("\n");
  return res;
}

void LuapeGraph::clearScores()
{
  universe->clearScores();
}

LuapeNodePtr LuapeGraph::pushMissingNodes(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodesMap::const_iterator it = nodesMap.find(node);
  if (it != nodesMap.end())
    return node;

  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (functionNode)
  {
    size_t n = functionNode->getNumArguments();
    for (size_t i = 0; i < n; ++i)
      pushMissingNodes(context, functionNode->getArgument(i));
  }

  LuapeYieldNodePtr yieldNode = node.dynamicCast<LuapeYieldNode>();
  if (yieldNode)
    pushMissingNodes(context, yieldNode->getArgument());
 
  nodesMap[node] = nodes.size();
  nodes.push_back(node);
  return node;
}

LuapeNodePtr LuapeGraph::pushFunctionNode(ExecutionContext& context, const FunctionPtr& function, const LuapeNodePtr& input)
{
  LuapeNodePtr node = universe->makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));
  pushNode(context, node);
  return node;
}

LuapeNodePtr LuapeGraph::pushNode(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodesMap::const_iterator it = nodesMap.find(node);
  if (it != nodesMap.end())
    return node;

  LuapeInputNodePtr inputNode = node.dynamicCast<LuapeInputNode>();
  if (inputNode)
    universe->addInputNode(inputNode);

  node->indexInGraph = nodes.size();
  nodesMap[node] = nodes.size();
  nodes.push_back(node);
  return node;
}

void LuapeGraph::popNode()
{
  jassert(nodes.size());
  LuapeNodePtr node = nodes.back();
  nodesMap.erase(node);
  nodes.pop_back();
}

size_t LuapeGraph::getNumTrainingSamples() const
  {return nodes.size() ? nodes[0]->getCache()->getNumTrainingSamples() : 0;}

size_t LuapeGraph::getNumValidationSamples() const
  {return nodes.size() ? nodes[0]->getCache()->getNumValidationSamples() : 0;}

size_t LuapeGraph::getNumSamples(bool isTrainingSamples) const
  {return nodes.size() ? nodes[0]->getCache()->getNumSamples(isTrainingSamples) : 0;}

void LuapeGraph::resizeSamples(bool isTrainingSamples, size_t numSamples)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->getCache()->resizeSamples(isTrainingSamples, numSamples);
}

void LuapeGraph::resizeSamples(size_t numTrainingSamples, size_t numValidationSamples)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->getCache()->resizeSamples(numTrainingSamples, numValidationSamples);
}

void LuapeGraph::setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example)
{
  jassert(example.size() <= nodes.size());
  for (size_t i = 0; i < example.size(); ++i)
  {
    LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
    jassert(node);
    node->getCache()->setSample(isTrainingSample, index, example[i]);
  }
}

void LuapeGraph::setSample(bool isTrainingSample, size_t index, const ObjectPtr& example)
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
      node->getCache()->setSample(isTrainingSample, index, container->getElement(i));
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
      node->getCache()->setSample(isTrainingSample, index, example->getVariable(i));
    }
  }
}

void LuapeGraph::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  getUniverse()->clearSamples(clearTrainingSamples, clearValidationSamples);
}

void LuapeGraph::compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex, LuapeGraphCallbackPtr callback) const
{
  for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
    state[i] = nodes[i]->compute(context, state, callback);
}

void LuapeGraph::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeGraphPtr& target = t.staticCast<LuapeGraph>();
  target->nodes = nodes;
  target->universe = universe;
}

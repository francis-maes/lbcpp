/*-----------------------------------------.---------------------------------.
| Filename: LuapeUniverse.cpp              | Luape Universe                  |
| Author  : Francis Maes                   |                                 |
| Started : 19/12/2011 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeUniverse.h>
#include <lbcpp/Luape/LuapeNode.h>
using namespace lbcpp;

LuapeFunctionPtr LuapeUniverse::makeFunction(ClassPtr functionClass, const std::vector<Variable>& arguments)
{
  FunctionKey key(functionClass, arguments);
  FunctionsMap::const_iterator it = functions.find(key);
  if (it == functions.end())
  {
    LuapeFunctionPtr function = LuapeFunction::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    functions[key] = function;
    return function;
  }
  else
    return it->second;
}

LuapeNodePtr LuapeUniverse::makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
{
  FunctionNodeKey key(function, inputs);
  FunctionNodesMap::const_iterator it = functionNodes.find(key);
  if (it == functionNodes.end())
  {
    LuapeNodePtr res = canonizeNode(new LuapeFunctionNode(function, inputs));
    functionNodes[key] = res;
    return res;
  }
  else
    return it->second;
}

void LuapeUniverse::observeNodeComputingTime(const LuapeNodePtr& node, size_t numInstances, double timeInMilliseconds)
{
  nodesComputingTimeStatistics[makeNodeStatisticsKey(node)].push(timeInMilliseconds / (double)numInstances, (double)numInstances);
}

double LuapeUniverse::getExpectedComputingTime(const LuapeNodePtr& node) const // in milliseconds
{
  if (node.isInstanceOf<LuapeInputNode>() || node.isInstanceOf<LuapeConstantNode>())
    return 0.0;
  std::map<std::pair<ClassPtr, ClassPtr>, ScalarVariableStatistics>::const_iterator it 
    = nodesComputingTimeStatistics.find(makeNodeStatisticsKey(node));
  jassert(it != nodesComputingTimeStatistics.end());
  return it->second.getMean();
}

std::pair<ClassPtr, ClassPtr> LuapeUniverse::makeNodeStatisticsKey(const LuapeNodePtr& node) 
{
  if (node.isInstanceOf<LuapeFunctionNode>())
    return std::make_pair(luapeFunctionNodeClass, node.staticCast<LuapeFunctionNode>()->getFunction()->getClass());
  else
    return std::make_pair(node->getClass(), ClassPtr());
}

#if 0
/*
** LuapeNodeKeysMap
*/
void LuapeNodeKeysMap::clear()
{
  keyToNodes.clear();
  nodeToKeys.clear();
}

// return true if it is a new node
bool LuapeNodeKeysMap::addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodeToKeyMap::const_iterator it = nodeToKeys.find(node);
  if (it != nodeToKeys.end())
    return false; // we already know about this node

  // compute node key
  graph->updateNodeCache(context, node, true);
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();

  KeyToNodeMap::iterator it2 = keyToNodes.find(key);
  if (it2 == keyToNodes.end())
  {
    // this is a new node equivalence class
    nodeToKeys[node] = key;
    keyToNodes[key] = node;
    addSubNodesToCache(context, node);
    return true;
  }
  else
  {
    // existing node equivalence class
    //  see if new node is better than previous one to represent the class
    LuapeNodePtr previousNode = it2->second;
    if (node->getDepth() < previousNode->getDepth())
    {
      it2->second = node;
      context.informationCallback(T("Change computation of ") + previousNode->toShortString() + T(" into ") + node->toShortString());
      LuapeFunctionNodePtr sourceFunctionNode = node.dynamicCast<LuapeFunctionNode>();
      LuapeFunctionNodePtr targetFunctionNode = previousNode.dynamicCast<LuapeFunctionNode>();
      if (sourceFunctionNode && targetFunctionNode)
        sourceFunctionNode->clone(context, targetFunctionNode);
      addSubNodesToCache(context, node);
    }
    nodeToKeys[node] = it2->first;
    return false;
  }
}

void LuapeNodeKeysMap::addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (functionNode)
  {
    std::vector<LuapeNodePtr> arguments = functionNode->getArguments();
    for (size_t i = 0; i < arguments.size(); ++i)
      addNodeToCache(context, arguments[i]);
  }
}

bool LuapeNodeKeysMap::isNodeKeyNew(const LuapeNodePtr& node) const
{
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();
  return keyToNodes.find(key) == keyToNodes.end();
}
#endif // 0

#if 0
BinaryKeyPtr LuapeNodeCache::makeKeyFromSamples(bool useTrainingSamples) const
{
  ContainerPtr samples = getSamples(useTrainingSamples);
  size_t n = samples->getNumElements();
  
  BooleanVectorPtr booleanVector = samples.dynamicCast<BooleanVector>();
  if (booleanVector)
  {
    BinaryKeyPtr res = new BinaryKey(1 + n / 8);
    std::vector<bool>::const_iterator it = booleanVector->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      res->pushBit(*it++);
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

String LuapeNodeCache::toShortString() const
{
  size_t n = trainingSamples ? trainingSamples->getNumElements() : 0;
  if (n == 0)
    return "<no examples>";

  TypePtr elementsType = trainingSamples->getElementsType();
  if (elementsType == booleanType)
  {
    BooleanVectorPtr booleans = trainingSamples.staticCast<BooleanVector>();
    size_t countOfTrue = 0;
    for (size_t i = 0; i < n; ++i)
      if (booleans->get(i))
        ++countOfTrue;
    return String((int)countOfTrue) + T(" / ") + String((int)n);
  }
  else if (elementsType.isInstanceOf<Enumeration>())
  {
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(elementsType.staticCast<Enumeration>(), doubleType);
    double invZ = 1.0 / (double)n;
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = trainingSamples->getElement(i);
      if (!v.isMissingValue())
        probabilities->incrementValue(v.getInteger(), invZ);
    }
    return probabilities->toShortString();
  }
  else if (elementsType->isConvertibleToDouble())
  {
    ScalarVariableStatistics stats;
    for (size_t i = 0; i < n; ++i)
      stats.push(trainingSamples->getElement(i).toDouble());
    return stats.toShortString();
  }
  else
    return String((int)n);
}

#endif // 0

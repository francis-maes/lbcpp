/*-----------------------------------------.---------------------------------.
| Filename: ExpressionUniverse.cpp         | Expression Universe             |
| Author  : Francis Maes                   |                                 |
| Started : 19/12/2011 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/ExpressionUniverse.h>
#include <lbcpp-ml/Expression.h>
using namespace lbcpp;

ExpressionUniverse::ExpressionUniverse() : maxFunctionDepth((size_t)-1), maxObservedFunctionDepth(0)
{
}

ConstantExpressionPtr ExpressionUniverse::makeConstantNode(const ObjectPtr& constantValue)
{
  if (constantValue.exists())
    return new ConstantExpression(constantValue);

  // cache only "missing value" nodes for each type
  ConstantNodesMap::const_iterator it = constantNodes.find(constantValue);
  if (it == constantNodes.end())
  {
    ConstantExpressionPtr res = new ConstantExpression(constantValue);
    constantNodes[constantValue] = res;
    return res;
  }
  else
    return it->second;
}

FunctionPtr ExpressionUniverse::makeFunction(ClassPtr functionClass, const std::vector<ObjectPtr>& arguments)
{
  FunctionKey key(functionClass, arguments);
  FunctionsMap::const_iterator it = functions.find(key);
  if (it == functions.end())
  {
    FunctionPtr function = Function::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    functions[key] = function;
    return function;
  }
  else
    return it->second;
}

ExpressionPtr ExpressionUniverse::makeFunctionExpression(const FunctionPtr& function, const std::vector<ExpressionPtr>& inputs)
{
  FunctionExpressionKey key(function, inputs);
  FunctionExpressionsMap::const_iterator it = functionNodes.find(key);
  if (it == functionNodes.end())
  {
    ExpressionPtr res = canonizeNode(new FunctionExpression(function, inputs));
    cacheFunctionExpression(key, res);
    return res;
  }
  else
    return it->second;
}

ExpressionPtr ExpressionUniverse::makeFunctionExpression(const FunctionPtr& function, const ExpressionPtr& input)
  {return makeFunctionExpression(function, std::vector<ExpressionPtr>(1, input));}

ExpressionPtr ExpressionUniverse::makeFunctionExpression(const FunctionPtr& function, const ExpressionPtr& input1, const ExpressionPtr& input2)
{
  std::vector<ExpressionPtr> inputs(2);
  inputs[0] = input1;
  inputs[1] = input2;
  return makeFunctionExpression(function, inputs);
}

void ExpressionUniverse::cacheFunctionExpression(const FunctionExpressionKey& key, ExpressionPtr node)
{
  enum {maxNumCachedFunctionExpressions = 1000000}; 

  size_t depth = node->getDepth();
  if (depth > maxObservedFunctionDepth)
    maxObservedFunctionDepth = depth;

/*  {
    static int counter = 0;
    if (++counter % 1000 == 0)
      std::cout << "depth = " << depth << " maxObserved: " << maxObservedFunctionDepth << " maxDepth: " << maxFunctionDepth << " count = " << functionNodes.size() << " / " << maxNumCachedFunctionExpressions << std::endl;
  }*/
  if (depth >= maxFunctionDepth)
    return; // do not cache: too deep

  functionNodes[key] = node;
  while (functionNodes.size() >= maxNumCachedFunctionExpressions)
  {
    if (maxFunctionDepth == (size_t)-1)
      maxFunctionDepth = maxObservedFunctionDepth;
    else
    {
      jassert(maxFunctionDepth > 0);
      --maxFunctionDepth;
    }

    FunctionExpressionsMap::iterator it, nxt;
    for (it = functionNodes.begin(); it != functionNodes.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (it->second->getDepth() >= maxFunctionDepth)
        functionNodes.erase(it);
    }
    std::cout << "Too many function nodes, new maxDepth = " << maxFunctionDepth << " => " << functionNodes.size() << " functions" << std::endl;
  }
}

void ExpressionUniverse::observeNodeComputingTime(const ExpressionPtr& node, size_t numInstances, double timeInMilliseconds)
{
  nodesComputingTimeStatistics[makeNodeStatisticsKey(node)].push(timeInMilliseconds / (double)numInstances, (double)numInstances);
}

double ExpressionUniverse::getExpectedComputingTime(const ExpressionPtr& node) const // in milliseconds
{
  if (node.isInstanceOf<VariableExpression>() || node.isInstanceOf<ConstantExpression>())
    return 0.0;
  std::map<std::pair<ClassPtr, ClassPtr>, ScalarVariableStatistics>::const_iterator it 
    = nodesComputingTimeStatistics.find(makeNodeStatisticsKey(node));
  jassert(it != nodesComputingTimeStatistics.end());
  return it->second.getMean();
}

std::pair<ClassPtr, ClassPtr> ExpressionUniverse::makeNodeStatisticsKey(const ExpressionPtr& node) 
{
  if (node.isInstanceOf<FunctionExpression>())
    return std::make_pair(functionExpressionClass, node.staticCast<FunctionExpression>()->getFunction()->getClass());
  else
    return std::make_pair(node->getClass(), ClassPtr());
}

void ExpressionUniverse::clearImportances()
{
  for (ConstantNodesMap::const_iterator it = constantNodes.begin(); it != constantNodes.end(); ++it)
    clearImportances(it->second);
  for (FunctionExpressionsMap::const_iterator it = functionNodes.begin(); it != functionNodes.end(); ++it)
    clearImportances(it->second);
}

void ExpressionUniverse::getImportances(std::map<ExpressionPtr, double>& res) const
{
  for (ConstantNodesMap::const_iterator it = constantNodes.begin(); it != constantNodes.end(); ++it)
    getImportances(it->second, res);
  for (FunctionExpressionsMap::const_iterator it = functionNodes.begin(); it != functionNodes.end(); ++it)
    getImportances(it->second, res);
}

void ExpressionUniverse::getImportances(const ExpressionPtr& node, std::map<ExpressionPtr, double>& res)
{
  if (node && res.find(node) == res.end())
  {
    double importance = node->getImportance();
    jassert(isNumberValid(importance));
    if (importance > 0)
      if (!node.isInstanceOf<FunctionExpression>() || node.staticCast<FunctionExpression>()->getFunction()->getClassName() != T("StumpFunction"))
        res[node] = importance;
    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      getImportances(node->getSubNode(i), res);
  }
}

void ExpressionUniverse::clearImportances(const ExpressionPtr& node)
{
  if (!node)
    return;
  node->setImportance(0.0);
  size_t n = node->getNumSubNodes();
  for (size_t i = 0; i < n; ++i)
    clearImportances(node->getSubNode(i));
}

void ExpressionUniverse::displayMostImportantNodes(ExecutionContext& context, const std::map<ExpressionPtr, double>& importances)
{
  // create probabilities and nodes vectors
  double Z = 0.0;
  std::vector<double> probabilities(importances.size());
  std::vector<ExpressionPtr> nodes(importances.size());
  size_t index = 0;
  for (std::map<ExpressionPtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it, ++index)
  {
    Z += it->second;
    probabilities[index] = it->second;
    nodes[index] = it->first;
  }

  // display most important nodes
  std::multimap<double, ExpressionPtr> nodeImportanceMap;
  for (std::map<ExpressionPtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
    nodeImportanceMap.insert(std::make_pair(it->second, it->first));
  size_t i = 0;
  for (std::multimap<double, ExpressionPtr>::reverse_iterator it = nodeImportanceMap.rbegin(); it != nodeImportanceMap.rend() && i < 20; ++it, ++i)
  {
    if (it->first <= 0.0)
      break;
    const ExpressionPtr& node = it->second;
    context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + node->toShortString() + T(" [") + String(it->first * 100.0 / Z, 2) + T("%]"));
  }
}


#if 0
/*
** ExpressionKeysMap
*/
void ExpressionKeysMap::clear()
{
  keyToNodes.clear();
  nodeToKeys.clear();
}

// return true if it is a new node
bool ExpressionKeysMap::addNodeToCache(ExecutionContext& context, const ExpressionPtr& node)
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
    ExpressionPtr previousNode = it2->second;
    if (node->getDepth() < previousNode->getDepth())
    {
      it2->second = node;
      context.informationCallback(T("Change computation of ") + previousNode->toShortString() + T(" into ") + node->toShortString());
      FunctionExpressionPtr sourceFunctionExpression = node.dynamicCast<FunctionExpression>();
      FunctionExpressionPtr targetFunctionExpression = previousNode.dynamicCast<FunctionExpression>();
      if (sourceFunctionExpression && targetFunctionExpression)
        sourceFunctionExpression->clone(context, targetFunctionExpression);
      addSubNodesToCache(context, node);
    }
    nodeToKeys[node] = it2->first;
    return false;
  }
}

void ExpressionKeysMap::addSubNodesToCache(ExecutionContext& context, const ExpressionPtr& node)
{
  FunctionExpressionPtr functionNode = node.dynamicCast<FunctionExpression>();
  if (functionNode)
  {
    std::vector<ExpressionPtr> arguments = functionNode->getArguments();
    for (size_t i = 0; i < arguments.size(); ++i)
      addNodeToCache(context, arguments[i]);
  }
}

bool ExpressionKeysMap::isNodeKeyNew(const ExpressionPtr& node) const
{
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();
  return keyToNodes.find(key) == keyToNodes.end();
}
#endif // 0

#if 0
BinaryKeyPtr ExpressionCache::makeKeyFromSamples(bool useTrainingSamples) const
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

String ExpressionCache::toShortString() const
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

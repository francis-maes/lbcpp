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
    LuapeFunctionPtr function = LuapeFunction::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    LuapeFunctionNodePtr res = new LuapeFunctionNode(function, inputs);
    functionNodes[key] = res;
    return res;
  }
  else
    return it->second;
}

LuapeFunctionNodePtr LuapeGraphUniverse::makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
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

LuapeNodePtr LuapeGraph::pushFunctionNode(ExecutionContext& context, const LuapeFunctionPtr& function, const LuapeNodePtr& input)
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
  target->nodesMap = nodesMap;
  target->universe = universe;
}

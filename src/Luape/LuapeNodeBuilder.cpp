/*-----------------------------------------.---------------------------------.
| Filename: LuapeNodeBuilder.cpp           | Node Builder base classes       |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeNodeBuilder.h>
#include <lbcpp/Luape/LuapeInference.h>
#include "NodeBuilder/NodeBuilderTypeSearchSpace.h"
using namespace lbcpp;

/*
** StochasticNodeBuilder
*/
StochasticNodeBuilder::StochasticNodeBuilder(size_t numNodes)
  : numNodes(numNodes)
{
}

void StochasticNodeBuilder::buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
{
  size_t numFailuresAllowed = 10 * numNodes;
  size_t numFailures = 0;
  std::set<LuapeNodePtr> nodes;
  size_t limit = numNodes;
  if (maxCount && maxCount < numNodes)
    limit = maxCount;
  while (nodes.size() < limit && numFailures < numFailuresAllowed)
  {
    LuapeNodePtr node = sampleNode(context, function);
    if (node && nodes.find(node) == nodes.end())
      nodes.insert(node);
    else
      ++numFailures;
  }

  context.resultCallback(T("numSamplingFailures"), numFailures);

  size_t index = res.size();
  res.resize(index + nodes.size());
  for (std::set<LuapeNodePtr>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    //context.informationCallback(T("Candidate: ") + (*it)->toShortString());
    res[index++] = *it;
  }
}

/*
** SequentialNodeBuilder
*/
SequentialNodeBuilder::SequentialNodeBuilder(size_t numNodes, size_t complexity)
  : StochasticNodeBuilder(numNodes), complexity(complexity)
{
}

LuapeNodePtr SequentialNodeBuilder::sampleNode(ExecutionContext& context, const LuapeInferencePtr& function)
{
  RandomGeneratorPtr random = context.getRandomGenerator();
  universe = function->getUniverse();
  typeSearchSpace = function->getSearchSpace(context, complexity);

  std::vector<LuapeNodePtr> stack;
  LuapeGraphBuilderTypeStatePtr typeState;

  for (size_t i = 0; i < complexity; ++i)
  {
    // Retrieve type-state index
    LuapeGraphBuilderTypeStatePtr typeState = getTypeState(i, stack);
    jassert(typeState);

    // Sample action
    ObjectPtr action;
    size_t numFailuresAllowed = 100;
    size_t numFailures;
    for (numFailures = 0; numFailures < numFailuresAllowed; ++numFailures)
      if (sampleAction(context, typeState, action) && isActionAvailable(action, stack))
        break;
    samplingDone(context, numFailures, numFailuresAllowed);
    if (numFailures == numFailuresAllowed)
      return LuapeNodePtr();

    // Execute action
    executeAction(stack, action);
    if (!action)
    {
      //context.informationCallback(T("Candidate: ") + stack[0]->toShortString());
      return stack[0]; // yield action
    }
  }

  context.informationCallback("Failed to sample candidate weak node");
  return LuapeNodePtr();
}

bool SequentialNodeBuilder::isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack)
{
  return !action || !action.isInstanceOf<LuapeFunction>() ||
    action.staticCast<LuapeFunction>()->acceptInputsStack(stack);
}

LuapeGraphBuilderTypeStatePtr SequentialNodeBuilder::getTypeState(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const
{
  std::vector<TypePtr> typeStack(stack.size());
  for (size_t j = 0; j < typeStack.size(); ++j)
    typeStack[j] = stack[j]->getType();
  return typeSearchSpace->getState(stepNumber, typeStack);
}

void SequentialNodeBuilder::executeAction(std::vector<LuapeNodePtr>& stack, const ObjectPtr& action) const
{
  // Execute action
  if (action)
  {
    if (action.isInstanceOf<LuapeNode>())
      stack.push_back(action);   // push action
    else
    {
      // apply action
      LuapeFunctionPtr function = action.staticCast<LuapeFunction>();
      size_t n = function->getNumInputs();
      jassert(stack.size() >= n && n > 0);
      std::vector<LuapeNodePtr> inputs(n);
      for (size_t i = 0; i < n; ++i)
        inputs[i] = stack[stack.size() - n + i];
      stack.erase(stack.begin() + stack.size() - n, stack.end());
      stack.push_back(universe->makeFunctionNode(function, inputs));
    }
  }
  else
  {
    // yield action
    jassert(stack.size() == 1);
  }
}

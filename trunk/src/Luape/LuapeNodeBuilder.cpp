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
** LuapeRPNSequence
*/
LuapeRPNSequence::LuapeRPNSequence(const std::vector<ObjectPtr>& sequence)
  : sequence(sequence) {}

String LuapeRPNSequence::toShortString() const
{
  String res;
  for (size_t i = 0; i < sequence.size(); ++i)
  {
    res += sequence[i]->toShortString();
    if (i < sequence.size() - 1)
      res += " ";
  }
  return res;
}

LuapeRPNSequencePtr LuapeRPNSequence::fromNode(const LuapeNodePtr& node)
{
  LuapeRPNSequencePtr res = new LuapeRPNSequence();
  res->appendNode(node);
  return res;
}

void LuapeRPNSequence::appendNode(const LuapeNodePtr& node)
{
  if (node.isInstanceOf<LuapeInputNode>() || node.isInstanceOf<LuapeConstantNode>())
    append(node);
  else if (node.isInstanceOf<LuapeFunctionNode>())
  {
    const LuapeFunctionNodePtr& functionNode = node.staticCast<LuapeFunctionNode>();
    size_t n = functionNode->getNumArguments();
    for (size_t i = 0; i < n; ++i)
      appendNode(functionNode->getArgument(i));
    append(functionNode->getFunction());
  }
  else
    jassert(false);
}

bool LuapeRPNSequence::startsWith(const LuapeRPNSequencePtr& start) const
{
  size_t n = start->sequence.size();
  if (sequence.size() < n)
    return false;
  for (size_t i = 0; i < n; ++i)
    if (sequence[i] != start->sequence[i])
      return false;
  return true;
}

std::vector<TypePtr> LuapeRPNSequence::computeTypeState(const std::vector<TypePtr>& initialState) const
{
  std::vector<TypePtr> state(initialState);
  for (size_t i = 0; i < sequence.size(); ++i)
  {
    ObjectPtr action = sequence[i];
    if (action.isInstanceOf<LuapeNode>())
      state.push_back(action.staticCast<LuapeNode>()->getType());
    else
    {
      const LuapeFunctionPtr& function = action.staticCast<LuapeFunction>();
      jassert(state.size() >= function->getNumInputs());
      TypePtr outputType = function->initialize(&state[state.size() - function->getNumInputs()]);
      state.resize(state.size() - function->getNumInputs() + 1);
      state.back() = outputType;
    }
  }
  return state;
}

/*
** StochasticNodeBuilder
*/
StochasticNodeBuilder::StochasticNodeBuilder(size_t numNodes)
  : numNodes(numNodes)
{
}

void StochasticNodeBuilder::buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
{
  size_t numTrialsAllowed = 5 * numNodes;
  
  std::set<LuapeNodePtr> nodes;
  size_t limit = numNodes;
  if (maxCount && maxCount < numNodes)
    limit = maxCount;
  size_t numTrials;
  for (numTrials = 0; numTrials < numTrialsAllowed && nodes.size() < limit; ++numTrials)
  {
    LuapeNodePtr node = sampleNode(context, function);
    if (node)
      nodes.insert(node);
  }

  context.resultCallback(T("numSamplingFailures"), numTrials - nodes.size());

  size_t index = res.size();
  res.resize(index + nodes.size());
  //context.enterScope(String((int)nodes.size()) + T(" candidates"));
  for (std::set<LuapeNodePtr>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    //context.informationCallback(T("Candidate: ") + (*it)->toShortString());
    res[index++] = *it;
  }
  //context.leaveScope();
}

/*
** SequentialNodeBuilder
*/
SequentialNodeBuilder::SequentialNodeBuilder(size_t numNodes, size_t complexity)
  : StochasticNodeBuilder(numNodes), complexity(complexity)
{
}

void SequentialNodeBuilder::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  StochasticNodeBuilder::clone(context, target);
}

LuapeNodePtr SequentialNodeBuilder::sampleNode(ExecutionContext& context, const LuapeInferencePtr& problem)
{
  RandomGeneratorPtr random = context.getRandomGenerator();
  universe = problem->getUniverse();
  typeSearchSpace = problem->getSearchSpace(context, complexity, true);
  jassert(typeSearchSpace);

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
      if (sampleAction(context, problem, typeState, action) && isActionAvailable(action, stack))
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
  jassert(typeSearchSpace);
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

/*-----------------------------------------.---------------------------------.
| Filename: ExpressionBuilder.cpp          | Expression Builder classes      |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/ExpressionBuilder.h>
#include <lbcpp-ml/ExpressionDomain.h>
using namespace lbcpp;

/*
** StochasticNodeBuilder
*/
StochasticNodeBuilder::StochasticNodeBuilder(size_t numNodes)
  : numNodes(numNodes)
{
}

void StochasticNodeBuilder::buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<ExpressionPtr>& res)
{
  size_t numTrialsAllowed = 5 * numNodes;
  
  std::set<ExpressionPtr> nodes;
  size_t limit = numNodes;
  if (maxCount && maxCount < numNodes)
    limit = maxCount;
  size_t numTrials;
  for (numTrials = 0; numTrials < numTrialsAllowed && nodes.size() < limit; ++numTrials)
  {
    ExpressionPtr node = sampleNode(context, function);
    if (node)
      nodes.insert(node);
  }

  context.resultCallback(T("numSamplingFailures"), numTrials - nodes.size());

  size_t index = res.size();
  res.resize(index + nodes.size());
  //context.enterScope(String((int)nodes.size()) + T(" candidates"));
  for (std::set<ExpressionPtr>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
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

ExpressionPtr SequentialNodeBuilder::sampleNode(ExecutionContext& context, const ExpressionDomainPtr& problem)
{
  RandomGeneratorPtr random = context.getRandomGenerator();
  //universe = problem->getUniverse();
  jassertfalse; // broken
  typeSearchSpace = problem->getSearchSpace(context, complexity, true);
  jassert(typeSearchSpace);

  std::vector<ExpressionPtr> stack;
  PostfixExpressionTypeStatePtr typeState;

  for (size_t i = 0; i < complexity; ++i)
  {
    // Retrieve type-state index
    PostfixExpressionTypeStatePtr typeState = getTypeState(i, stack);
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
      return ExpressionPtr();

    // Execute action
    executeAction(stack, action);
    if (!action)
    {
      //context.informationCallback(T("Candidate: ") + stack[0]->toShortString());
      return stack[0]; // yield action
    }
  }

  context.informationCallback("Failed to sample candidate weak node");
  return ExpressionPtr();
}

bool SequentialNodeBuilder::isActionAvailable(ObjectPtr action, const std::vector<ExpressionPtr>& stack)
{
  return !action || !action.isInstanceOf<Function>() ||
    action.staticCast<Function>()->acceptInputsStack(stack);
}

PostfixExpressionTypeStatePtr SequentialNodeBuilder::getTypeState(size_t stepNumber, const std::vector<ExpressionPtr>& stack) const
{
  jassert(typeSearchSpace);
  std::vector<TypePtr> typeStack(stack.size());
  for (size_t j = 0; j < typeStack.size(); ++j)
    typeStack[j] = stack[j]->getType();
  return typeSearchSpace->getState(stepNumber, typeStack);
}

void SequentialNodeBuilder::executeAction(std::vector<ExpressionPtr>& stack, const ObjectPtr& action) const
{
  // Execute action
  if (action)
  {
    if (action.isInstanceOf<Expression>())
      stack.push_back(action);   // push action
    else
    {
      // apply action
      FunctionPtr function = action.staticCast<Function>();
      size_t n = function->getNumInputs();
      jassert(stack.size() >= n && n > 0);
      std::vector<ExpressionPtr> inputs(n);
      for (size_t i = 0; i < n; ++i)
        inputs[i] = stack[stack.size() - n + i];
      stack.erase(stack.begin() + stack.size() - n, stack.end());
      stack.push_back(universe->makeFunctionExpression(function, inputs));
    }
  }
  else
  {
    // yield action
    jassert(stack.size() == 1);
  }
}

/*-----------------------------------------.---------------------------------.
| Filename: PostfixExpression.cpp          | Expressions in postfix notation |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2012 15:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/PostfixExpression.h>
#include <lbcpp-ml/ExpressionDomain.h>
#include "TypedPostfixExpressionState.h"
using namespace lbcpp;

/*
** PostfixExpressionSequence
*/
PostfixExpressionSequence::PostfixExpressionSequence(const std::vector<ObjectPtr>& sequence)
  : sequence(sequence) {}

String PostfixExpressionSequence::toShortString() const
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

PostfixExpressionSequencePtr PostfixExpressionSequence::fromNode(const ExpressionPtr& node)
{
  PostfixExpressionSequencePtr res = new PostfixExpressionSequence();
  res->appendNode(node);
  return res;
}

void PostfixExpressionSequence::appendNode(const ExpressionPtr& node)
{
  if (node.isInstanceOf<VariableExpression>() || node.isInstanceOf<ConstantExpression>())
    append(node);
  else if (node.isInstanceOf<FunctionExpression>())
  {
    const FunctionExpressionPtr& functionNode = node.staticCast<FunctionExpression>();
    size_t n = functionNode->getNumArguments();
    for (size_t i = 0; i < n; ++i)
      appendNode(functionNode->getArgument(i));
    append(functionNode->getFunction());
  }
  else
    jassert(false);
}

bool PostfixExpressionSequence::startsWith(const PostfixExpressionSequencePtr& start) const
{
  size_t n = start->sequence.size();
  if (sequence.size() < n)
    return false;
  for (size_t i = 0; i < n; ++i)
    if (sequence[i] != start->sequence[i])
      return false;
  return true;
}

std::vector<TypePtr> PostfixExpressionSequence::computeTypeState(const std::vector<TypePtr>& initialState) const
{
  std::vector<TypePtr> state(initialState);
  for (size_t i = 0; i < sequence.size(); ++i)
  {
    ObjectPtr action = sequence[i];
    if (action.isInstanceOf<Expression>())
      state.push_back(action.staticCast<Expression>()->getType());
    else
    {
      const FunctionPtr& function = action.staticCast<Function>();
      jassert(state.size() >= function->getNumInputs());
      TypePtr outputType = function->initialize(&state[state.size() - function->getNumInputs()]);
      state.resize(state.size() - function->getNumInputs() + 1);
      state.back() = outputType;
    }
  }
  return state;
}

void PostfixExpressionSequence::apply(const ExpressionUniversePtr& universe, std::vector<ExpressionPtr>& stack, const ObjectPtr& element)
{
   if (element.isInstanceOf<Expression>()) // push action
    stack.push_back(element.staticCast<Expression>());
  else if (element.isInstanceOf<Function>()) // apply action
  {
    const FunctionPtr& function = element.staticCast<Function>();
    size_t numInputs = function->getNumInputs();
    jassert(stack.size() >= numInputs);
    std::vector<ExpressionPtr> inputs(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
      inputs[i] = stack[stack.size() - numInputs + i];
    stack.resize(stack.size() - numInputs + 1);
    stack.back() = new FunctionExpression(function, inputs);// universe->makeFunctionExpression(function, inputs);
  }
  else
    jassert(false);
}

ExpressionPtr PostfixExpressionSequence::toNode(const ExpressionUniversePtr& universe) const
{
  std::vector<ExpressionPtr> stack;
  for (size_t i = 0; i < sequence.size(); ++i)
    apply(universe, stack, sequence[i]);
  jassert(stack.size() == 1);
  return stack[0];
}

/*
** PostfixExpressionTypeState
*/
PostfixExpressionTypeState::PostfixExpressionTypeState(size_t depth, const std::vector<TypePtr>& stack, bool yieldable)
  : depth(depth), stack(stack), yieldable(yieldable), stateIndex((size_t)-1), numNodeTypesWhenBuilt(0), canBePruned(false), canBePrunedComputed(false) {}
PostfixExpressionTypeState::PostfixExpressionTypeState() : depth(0), stateIndex((size_t)-1), numNodeTypesWhenBuilt(0), canBePruned(false), canBePrunedComputed(false) {}

String PostfixExpressionTypeState::toShortString() const
{
  String res = T("[") + String((int)depth) + T("] {");
  for (size_t i = 0; i < stack.size(); ++i)
  {
    String name = stack[i]->getName();
    jassert(name.isNotEmpty());
    res += name;
    if (i < stack.size() - 1)
      res += T(", ");
  }
  res += T("} -> ");
  if (push.size())
    res += String((int)push.size()) + T(" push actions");
  if (apply.size())
  {
    if (push.size()) res += T(", ");
    res += String((int)apply.size()) + T(" apply actions");
  }
  if (hasYieldAction())
  {
    if (push.size() || apply.size()) res += T(", ");
    res += T("yield action");
  }
  return res;
}

bool PostfixExpressionTypeState::hasPushAction(const TypePtr& type) const
{
  for (size_t i = 0; i < push.size(); ++i)
    if (push[i].first == type)
      return true;
  return false;
}

PostfixExpressionTypeStatePtr PostfixExpressionTypeState::getPushTransition(const TypePtr& type) const
{
  for (size_t i = 0; i < push.size(); ++i)
    if (push[i].first == type)
      return push[i].second;
  return PostfixExpressionTypeStatePtr();
}

bool PostfixExpressionTypeState::hasApplyAction(const FunctionPtr& function) const
{
  for (size_t i = 0; i < apply.size(); ++i)
    if (apply[i].first == function)
      return true;
  return false;
}

void PostfixExpressionTypeState::setPushTransition(const TypePtr& type, const PostfixExpressionTypeStatePtr& nextState)
{
  for (size_t i = 0; i < push.size(); ++i)
    if (push[i].first == type)
    {
      jassert(push[i].second == nextState);
      return;
    }
  push.push_back(std::make_pair(type, nextState));
}
  
void PostfixExpressionTypeState::setApplyTransition(const FunctionPtr& function, const PostfixExpressionTypeStatePtr& nextState)
{
  for (size_t i = 0; i < apply.size(); ++i)
    if (apply[i].first == function)
    {
      jassert(apply[i].second == nextState);
      return;
    }
  apply.push_back(std::make_pair(function, nextState));
}

/*
** PostfixExpressionTypeSpace
*/
PostfixExpressionTypeSpace::PostfixExpressionTypeSpace(const ExpressionDomainPtr& domain, const std::vector<TypePtr>& initialState, size_t maxDepth)
{
  jassert(domain->getNumFunctions());
  std::vector<TypePtr> nodeTypes;
  for (size_t i = 0; i < domain->getNumInputs(); ++i)
    insertType(nodeTypes, domain->getInput(i)->getType());
  for (size_t i = 0; i < domain->getNumConstants(); ++i)
    insertType(nodeTypes, domain->getConstant(i)->getType());
  //insertType(nodeTypes, booleanType); // automatically included since boosting methods may create stump nodes that output booleans

  this->initialState = getOrCreateState(domain, 0, initialState);
  size_t numTypes = nodeTypes.size();
  while (true)
  {
    buildSuccessors(domain, this->initialState, nodeTypes, maxDepth + 1); // add 1 to account for the "yield action"
    size_t newNumTypes = nodeTypes.size();
    if (newNumTypes != numTypes)
    { 
      jassert(newNumTypes > numTypes);
      numTypes = newNumTypes;
    }
    else
      break;
  }
}

void PostfixExpressionTypeSpace::pruneStates(ExecutionContext& context, bool verbose)
{
//    context.informationCallback(T("Num states before pruning: ") + String((int)states.size()));
  prune(initialState);
  //jassert(!isRootPrunable);

  StateMap::iterator it, nxt;
  for (it = states.begin(); it != states.end(); it = nxt)
  {
    nxt = it; ++nxt;
    jassert(it->second->canBePrunedComputed);
    if (it->second->canBePruned)
      states.erase(it);
  }
  if (verbose)
  {
    context.enterScope(T("Type states"));
    for (it = states.begin(); it != states.end(); ++it)
      context.informationCallback(it->second->toShortString());
    context.leaveScope(states.size());
  }
}

void PostfixExpressionTypeSpace::assignStateIndices(ExecutionContext& context)
{
  size_t index = 0;
  for (StateMap::iterator it = states.begin(); it != states.end(); ++it)
    it->second->stateIndex = index++;
}

PostfixExpressionTypeStatePtr PostfixExpressionTypeSpace::getState(size_t depth, const std::vector<TypePtr>& stack) const
{
  StateMap::const_iterator it = states.find(std::make_pair(depth, stack));
  return it == states.end() ? PostfixExpressionTypeStatePtr() : it->second;
}

PostfixExpressionTypeStatePtr PostfixExpressionTypeSpace::getOrCreateState(const ExpressionDomainPtr& domain, size_t depth, const std::vector<TypePtr>& stack)
{
  StateKey key(depth, stack);
  StateMap::const_iterator it = states.find(key);
  if (it == states.end())
  { 
    bool yieldable = (stack.size() == 1 && domain->isTargetTypeAccepted(stack[0]));
    PostfixExpressionTypeStatePtr res = new PostfixExpressionTypeState(depth, stack, yieldable);
    states[key] = res;
    return res;
  }
  else
    return it->second;
}

void PostfixExpressionTypeSpace::insertType(std::vector<TypePtr>& types, const TypePtr& type)
{
  for (size_t i = 0; i < types.size(); ++i)
    if (types[i] == type)
      return;
  types.push_back(type);
}

void PostfixExpressionTypeSpace::buildSuccessors(const ExpressionDomainPtr& domain, const PostfixExpressionTypeStatePtr& state, std::vector<TypePtr>& nodeTypes, size_t maxDepth)
{
  if (maxDepth == state->getDepth())
  {
    state->stack.clear(); // thanks to this, the state will be pruned
    state->yieldable = false;
    return;
  }

  if (nodeTypes.size() == state->numNodeTypesWhenBuilt)
    return;
  state->numNodeTypesWhenBuilt = nodeTypes.size();

  size_t numRemainingSteps = maxDepth - state->getDepth();
  if (numRemainingSteps > state->getStackSize())
  {
    for (size_t i = 0; i < nodeTypes.size(); ++i)
    {
      TypePtr type = nodeTypes[i];
      PostfixExpressionTypeStatePtr nextState = state->getPushTransition(type);
      if (nextState)
        buildSuccessors(domain, nextState, nodeTypes, maxDepth);
      else
      {
        // compute new stack
        std::vector<TypePtr> newStack = state->getStack();
        newStack.push_back(type);

        // create successor state and call recursively
        PostfixExpressionTypeStatePtr newState = getOrCreateState(domain, state->getDepth() + 1, newStack);
        state->setPushTransition(type, newState);
        buildSuccessors(domain, newState, nodeTypes, maxDepth);
      }
    }
  }

  for (size_t i = 0; i < domain->getNumFunctions(); ++i)
  {
    FunctionPtr function = domain->getFunction(i);
    if (acceptInputTypes(function, state->getStack()))
    {
      size_t numInputs = function->getNumInputs();
      std::vector<ObjectPtr> tmp(function->getNumVariables());

      std::vector<TypePtr> inputTypes(numInputs);
      for (size_t i = 0; i < numInputs; ++i)
        inputTypes[i] = state->stack[state->getStackSize() - numInputs + i];

      std::vector<FunctionPtr> functions;
      enumerateFunctionVariables(domain->getUniverse(), function, inputTypes, tmp, 0, functions);

      for (size_t j = 0; j < functions.size(); ++j)
        applyFunctionAndBuildSuccessor(domain, state, functions[j], nodeTypes, maxDepth);
    }
  }
}

void PostfixExpressionTypeSpace::enumerateFunctionVariables(const ExpressionUniversePtr& universe, const FunctionPtr& function, const std::vector<TypePtr>& inputTypes, std::vector<ObjectPtr>& variables, size_t variableIndex, std::vector<FunctionPtr>& res)
{
  if (variableIndex == variables.size())
    res.push_back(universe->makeFunction(function->getClass(), variables));
  else
  {
    ContainerPtr values = function->getVariableCandidateValues(variableIndex, inputTypes);
    if (values)
    {
      size_t n = values->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        variables[variableIndex] = values->getElement(i).getObject();
        enumerateFunctionVariables(universe, function, inputTypes, variables, variableIndex + 1, res);
      }
    }
  }
}

void PostfixExpressionTypeSpace::applyFunctionAndBuildSuccessor(const ExpressionDomainPtr& domain, const PostfixExpressionTypeStatePtr& state, const FunctionPtr& function, std::vector<TypePtr>& nodeTypes, size_t maxDepth)
{
  // compute output type given input types
  size_t numInputs = function->getNumInputs();
  TypePtr* inputs = new TypePtr[numInputs];
  for (size_t i = 0; i < numInputs; ++i)
    inputs[i] = state->stack[state->getStackSize() - numInputs + i];
  TypePtr outputType = function->initialize(inputs);
  delete [] inputs;

  insertType(nodeTypes, outputType); // the function output type can become a graph node in later episodes

  // compute new stack and new graph node types
  std::vector<TypePtr> newStack(state->getStackSize() - numInputs + 1);
  for (size_t i = 0; i < state->getStackSize() - numInputs; ++i)
    newStack[i] = state->stack[i];
  newStack.back() = outputType;

  // create successor state and call recursively
  PostfixExpressionTypeStatePtr newState = getOrCreateState(domain, state->getDepth() + 1, newStack);
  state->setApplyTransition(function, newState);
  buildSuccessors(domain, newState, nodeTypes, maxDepth);
}

bool PostfixExpressionTypeSpace::acceptInputTypes(const FunctionPtr& function, const std::vector<TypePtr>& stack) const
{
  size_t numInputs = function->getNumInputs();
  if (numInputs > stack.size())
    return false;
  size_t stackFirstIndex = stack.size() - numInputs;
  for (size_t i = 0; i < numInputs; ++i)
    if (!function->doAcceptInputType(i, stack[stackFirstIndex + i]))
      return false;
  return true;
}

bool PostfixExpressionTypeSpace::prune(PostfixExpressionTypeStatePtr state) // return true if state is prunable
{
  if (state->canBePrunedComputed)
    return state->canBePruned;

  {
    std::vector<std::pair<TypePtr, PostfixExpressionTypeStatePtr> > remainingTransitions;
    for (size_t i = 0; i < state->push.size(); ++i)
      if (!prune(state->push[i].second))
        remainingTransitions.push_back(state->push[i]);
    state->push.swap(remainingTransitions);
  }
  {
    std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> > remainingTransitions;
    for (size_t i = 0; i < state->apply.size(); ++i)
      if (!prune(state->apply[i].second))
        remainingTransitions.push_back(state->apply[i]);
    state->apply.swap(remainingTransitions);
  }
  state->canBePruned = !state->hasAnyAction();
  state->canBePrunedComputed = true;
  return state->canBePruned;
}

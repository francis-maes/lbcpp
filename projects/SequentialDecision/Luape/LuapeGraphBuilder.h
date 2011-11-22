/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphBuilder.h            | Luape Graph Builder             |
| Author  : Francis Maes                   |  Decision Problem               |
| Started : 25/10/2011 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_BUILDER_H_
# define LBCPP_LUAPE_GRAPH_BUILDER_H_

# include "LuapeGraphBuilderTypeSearchSpace.h"
# include "../Core/DecisionProblem.h"

namespace lbcpp
{

class LuapeRPNGraphBuilderState : public DecisionProblemState
{
public:
  LuapeRPNGraphBuilderState(const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, size_t maxSteps)
    : problem(problem), graph(graph), initialGraphNumNodes(graph->getNumNodes()), maxSteps(maxSteps), numSteps(0)
  {
    state.numNodes = initialGraphNumNodes;
    state.setStaticAllocationFlag();
  }
  LuapeRPNGraphBuilderState() : maxSteps(0), numSteps(0) {}

  virtual String toShortString() const
  {
    if (isFinalState())
      return graph->getLastNode()->toShortString();

    String res = T("{");
    for (size_t i = 0; i < state.stack.size(); ++i)
    {
      res += state.stack[i]->toShortString();
      if (i < state.stack.size() - 1)
        res += T(", ");
    }
    res += T("}");
    if (state.function)
    {
      std::vector<LuapeNodePtr> arguments(state.functionNumArguments);
      for (size_t i = 0; i < state.functionNumArguments; ++i)
        arguments[i] = state.stack[state.stack.size() - state.functionNumArguments + i];
      res += T(" ") + state.function->toShortString(arguments);
    }
    return res;
  }

  virtual TypePtr getActionType() const
    {return variableType;}

  size_t getMinNumStepsToFinalState() const
  {
    if (state.function)
      return state.function->getNumVariables() + state.stack.size();
    else
      return state.stack.size();
  }

  virtual ContainerPtr getAvailableActions() const
  {
    VariableVectorPtr res = new VariableVector();
    if (numSteps >= maxSteps)
      return res;

    if (state.function)
    {
      std::vector<TypePtr> inputTypes(state.function->getNumInputs());
      for (size_t i = 0; i < inputTypes.size(); ++i)
        inputTypes[i] = state.stack[state.stack.size() - state.function->getNumInputs() + i]->getType();
      ContainerPtr candidateValues = state.function->getVariableCandidateValues(state.functionNumArguments, inputTypes);
      size_t n = candidateValues->getNumElements();
      for (size_t i = 0; i < n; ++i)
        res->append(candidateValues->getElement(i));
    }
    else
    {
      size_t numRemainingSteps = maxSteps - numSteps;
      if (numRemainingSteps > state.stack.size())
      {
        size_t n = graph->getNumNodes();
        for (size_t i = 0; i < n; ++i)
          if (!graph->getNode(i).isInstanceOf<LuapeYieldNode>())
            res->append(graph->getNode(i)); // push node action
      }

      if (numRemainingSteps > 1)
      {
        for (size_t i = 0; i < problem->getNumFunctions(); ++i)
        {
          LuapeFunctionPtr function = problem->getFunction(i);
          if (isFunctionAvailable(function) && numRemainingSteps >= 1 + function->getNumVariables() + (state.stack.size() - function->getNumInputs() + 1))
            res->append(function);
        }
      }

      if (isYieldAvailable())
        res->append(new LuapeYieldNode(state.stack.back()));
    }
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    if (stateBackup)
      *stateBackup = Variable(new State(state), objectClass);

    if (state.function)
    {
      state.function->setVariable(state.functionNumArguments, action);
      ++state.functionNumArguments;
      tryToResolveFunction(context);
    }
    else
    { 
      if (action.getType() == luapeYieldNodeClass)
      {
        const LuapeNodePtr& node = action.getObjectAndCast<LuapeNode>();
        graph->pushNode(context, node);
        state.stack.pop_back();
      }
      else if (action.getType()->inheritsFrom(luapeNodeClass))
      {
        state.stack.push_back(action.getObjectAndCast<LuapeNode>());
      }
      else if (action.getType()->inheritsFrom(luapeFunctionClass))
      {
        state.function = action.getObjectAndCast<LuapeFunction>();
        state.functionNumArguments = 0;
        tryToResolveFunction(context);
      }
      else
        jassert(false);
    }

    state.numNodes = graph->getNumNodes();
    reward = 0.0;
    ++numSteps;
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    --numSteps;
    state = *stateBackup.getObjectAndCast<State>();
    while (graph->getNumNodes() > state.numNodes)
      graph->popNode();
    return true;
  }

  virtual bool isFinalState() const
    {return numSteps >= maxSteps || (graph->getNumNodes() > initialGraphNumNodes && graph->getLastNode().isInstanceOf<LuapeYieldNode>());}

  LuapeGraphPtr getGraph() const
    {return graph;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<LuapeRPNGraphBuilderState>& target = t.staticCast<LuapeRPNGraphBuilderState>();
    target->problem = problem;
    target->graph = graph->cloneAndCast<LuapeGraph>(context);
    target->initialGraphNumNodes = initialGraphNumNodes;
    target->maxSteps = maxSteps;
    target->numSteps = numSteps;
    target->state = state;
  }

protected:
  friend class LuapeRPNGraphBuilderStateClass;

  LuapeProblemPtr problem;
  LuapeGraphPtr graph;
  size_t initialGraphNumNodes;
  size_t maxSteps;
  size_t numSteps;

  struct State : public Object
  {
    State() : functionNumArguments(0), numNodes(0) {}

    std::vector<LuapeNodePtr> stack;
    LuapeFunctionPtr function;
    size_t functionNumArguments;
    size_t numNodes;
  };
  State state;
  
  bool isFunctionAvailable(const LuapeFunctionPtr& function) const
    {return function->acceptInputsStack(state.stack);}

  bool isYieldAvailable() const
    {return state.stack.size() && state.stack.back()->getType()->inheritsFrom(booleanType);}

  void tryToResolveFunction(ExecutionContext& context)
  {
    if (state.functionNumArguments >= state.function->getClass()->getNumMemberVariables())
    {
      // add function node into graph
      size_t numInputs = state.function->getNumInputs();
      std::vector<LuapeNodePtr> inputs(numInputs);
      for (size_t i = 0; i < numInputs; ++i)
        inputs[i] = state.stack[state.stack.size() - numInputs + i];
      LuapeNodePtr node = graph->getUniverse()->makeFunctionNode(state.function, inputs);
      bool ok = graph->pushNode(context, node);
      jassert(ok);

      // remove arguments from stack
      state.stack.erase(state.stack.begin() + state.stack.size() - numInputs, state.stack.end());
      state.function = LuapeFunctionPtr();
      state.functionNumArguments = 0;

      // push result onto stack
      state.stack.push_back(graph->getLastNode());
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeRPNGraphBuilderState> LuapeRPNGraphBuilderStatePtr;

class LuapeGraphBuilderAction;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderAction> LuapeGraphBuilderActionPtr;

class LuapeGraphBuilderAction : public Object
{
public:
  LuapeGraphBuilderAction(size_t numNodesToRemove, const LuapeNodePtr& nodeToAdd, bool isNewNode)
    : numNodesToRemove(numNodesToRemove), nodeToAdd(nodeToAdd), newNode(isNewNode), hasAddedNodeToGraph(false) {}
  LuapeGraphBuilderAction() : numNodesToRemove(0), newNode(false), hasAddedNodeToGraph(false) {}

  static LuapeGraphBuilderActionPtr push(const LuapeNodePtr& node)
    {return new LuapeGraphBuilderAction(0, node, false);}

  static LuapeGraphBuilderActionPtr apply(const LuapeGraphPtr& graph, const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
    {return new LuapeGraphBuilderAction(inputs.size(), graph->getUniverse()->makeFunctionNode(function, inputs), true);}

  static LuapeGraphBuilderActionPtr yield(const LuapeNodePtr& node)
    {return new LuapeGraphBuilderAction(1, new LuapeYieldNode(node), true);}

  bool isNewNode() const
    {return newNode;}

  const LuapeNodePtr& getNodeToAdd() const
    {return nodeToAdd;}

  bool isUseless(const LuapeGraphPtr& graph) const
    {return nodeToAdd && isNewNode() && graph->containsNode(nodeToAdd);}

  void perform(std::vector<LuapeNodePtr>& stack, const LuapeGraphPtr& graph)
  {
    if (numNodesToRemove)
    {
      jassert(stack.size() >= numNodesToRemove);
      removedNodes.resize(numNodesToRemove);
      size_t firstIndex = stack.size() - numNodesToRemove;
      for (size_t i = 0; i < numNodesToRemove; ++i)
        removedNodes[i] = stack[firstIndex + i];
      stack.erase(stack.begin() + firstIndex, stack.end());
    }
    if (nodeToAdd)
    {
      stack.push_back(nodeToAdd);
      if (isNewNode() && !graph->containsNode(nodeToAdd))
      {
        graph->pushNode(defaultExecutionContext(), nodeToAdd);
        hasAddedNodeToGraph = true;
      }
    }
  }

  void undo(std::vector<LuapeNodePtr>& stack, const LuapeGraphPtr& graph)
  {
    if (nodeToAdd)
    {
      if (hasAddedNodeToGraph)
        graph->popNode();
      jassert(stack.size() && stack.back() == nodeToAdd);
      stack.pop_back();
    }
    if (numNodesToRemove)
    {
      jassert(removedNodes.size() == numNodesToRemove);
      for (size_t i = 0; i < numNodesToRemove; ++i)
        stack.push_back(removedNodes[i]);
    }
  }

private:
  size_t numNodesToRemove;
  LuapeNodePtr nodeToAdd;
  bool newNode;

  std::vector<LuapeNodePtr> removedNodes; // use in state backup only
  bool hasAddedNodeToGraph;
};

extern ClassPtr luapeGraphBuilderActionClass;

class LuapeGraphBuilderState : public DecisionProblemState
{
public:
  LuapeGraphBuilderState(const LuapeGraphPtr& graph, LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace)
    : graph(graph), typeSearchSpace(typeSearchSpace), typeState(typeSearchSpace->getInitialState()), numSteps(0), isAborted(false), isYielded(false)
  {
    /*nodeKeys = new LuapeNodeKeysMap();
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
      if (!graph->getNode(i).isInstanceOf<LuapeYieldNode>())
        nodeKeys->addNodeToCache(defaultExecutionContext(), graph->getNode(i));
        */

  }
  LuapeGraphBuilderState() : numSteps(0), isAborted(false), isYielded(false) {}

  virtual String toShortString() const
  {
    String seps = isFinalState() ? T("[]") : T("{}");

    String res;
    if (isAborted)
      res += T("canceled - ");
    res += seps[0];
    for (size_t i = 0; i < stack.size(); ++i)
    {
      res += stack[i]->toShortString();
      if (i < stack.size() - 1)
        res += T(", ");
    }
    res += seps[1];
    return res;
  }

  virtual TypePtr getActionType() const
    {return luapeGraphBuilderActionClass;}

  virtual ContainerPtr getAvailableActions() const
  {
    if (availableActions)
      return availableActions;

    if (!typeState)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(luapeGraphBuilderActionClass, 0);
    const_cast<LuapeGraphBuilderState* >(this)->availableActions = res;

    if (typeState->hasPushActions())
    {
      size_t n = graph->getNumNodes();
      for (size_t i = 0; i < n; ++i)
      {
        LuapeNodePtr node = graph->getNode(i);
        if (!node.isInstanceOf<LuapeYieldNode>() && typeState->canTypeBePushed(node->getType()))
          res->append(LuapeGraphBuilderAction::push(graph->getNode(i)));
      }
    }
    if (typeState->hasApplyActions())
    {
      const std::map<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr>& apply = typeState->getApplyActions();
      for (std::map<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr>::const_iterator it = apply.begin(); it != apply.end(); ++it)
      {
        LuapeFunctionPtr function = it->first;
        if (function->acceptInputsStack(stack))
        {
          size_t numInputs = function->getNumInputs();
          std::vector<LuapeNodePtr> inputs(numInputs);
          for (size_t i = 0; i < numInputs; ++i)
            inputs[i] = stack[stack.size() - numInputs + i];

          LuapeGraphBuilderActionPtr action = LuapeGraphBuilderAction::apply(graph, function, inputs);
          if (!action->isUseless(graph)) // useless == the created node already exists in the graph
            res->append(action);
        }
      }
    }

    if (typeState->hasYieldAction())
      res->append(LuapeGraphBuilderAction::yield(stack.back()));
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& a, double& reward, Variable* stateBackup = NULL)
  {
    const LuapeGraphBuilderActionPtr& action = a.getObjectAndCast<LuapeGraphBuilderAction>();
    if (stateBackup)
      *stateBackup = action;
    action->perform(stack, graph);
    LuapeNodePtr nodeToAdd = action->getNodeToAdd();
    if (action->isNewNode())
    {
      /*nodeToAdd->updateCache(context, true);
      if (nodeToAdd.isInstanceOf<LuapeFunctionNode>() && !nodeKeys->isNodeKeyNew(nodeToAdd))
      {
        context.informationCallback(T("Aborded: ") + nodeToAdd->toShortString());
        isAborted = true; // create a node that has the same key as an already existing node in the graph is forbidden 
        // note that in the current implementation, we only compare to the nodes existing in the current initial graph
        //  (not intermediary nodes created along the builder trajectory)
      }*/
    }
    reward = 0.0;
    ++numSteps;
    availableActions = ContainerPtr();
    if (nodeToAdd && nodeToAdd.isInstanceOf<LuapeYieldNode>())
    {
      isYielded = true;
      typeState = LuapeGraphBuilderTypeStatePtr();
    }
    else
      updateTypeState();
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    const LuapeGraphBuilderActionPtr& action = stateBackup.getObjectAndCast<LuapeGraphBuilderAction>();
    isAborted = isYielded = false;
    --numSteps;
    action->undo(stack, graph);
    availableActions = ContainerPtr();
    updateTypeState();
    return true;
  }

  virtual bool isFinalState() const
    {return isAborted || isYielded || !typeState->hasAnyAction();}

  LuapeGraphPtr getGraph() const
    {return graph;}

  size_t getCurrentStep() const
    {return numSteps;}

  size_t getStackSize() const
    {return stack.size();}

  const LuapeNodePtr& getStackElement(size_t index) const
    {jassert(index < stack.size()); return stack[index];}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraphBuilderStateClass;

  LuapeGraphPtr graph;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
  LuapeGraphBuilderTypeStatePtr typeState;
  ContainerPtr availableActions;
  //LuapeNodeKeysMapPtr nodeKeys;

  std::vector<LuapeNodePtr> stack;
  size_t numSteps;
  bool isAborted;
  bool isYielded;

  void updateTypeState()
  {
    std::vector<TypePtr> types(stack.size());
    for (size_t i = 0; i < types.size(); ++i)
      types[i] = stack[i]->getType();
    typeState = typeSearchSpace->getState(numSteps, types);
    jassert(typeState);
  }

/*
  bool isYieldAvailable() const
    {return stack.size() == 1 && stack[0]->getType()->inheritsFrom(booleanType);}

  void enumerateFunctionVariables(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs, ObjectVectorPtr res) const
  {
    std::vector<TypePtr> inputTypes(inputs.size());
    for (size_t i = 0; i < inputTypes.size(); ++i)
      inputTypes[i] = inputs[i]->getType();

    std::vector<Variable> variables(function->getNumVariables());
    enumerateFunctionVariables(function, inputs, inputTypes, variables, 0, res);
  }

  // enumerate function parameters recursively
  void enumerateFunctionVariables(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs, const std::vector<TypePtr>& inputTypes,
                                  std::vector<Variable>& variables, size_t variableIndex, ObjectVectorPtr res) const
  {
    if (variableIndex == variables.size())
    {
      LuapeFunctionPtr f = function->cloneAndCast<LuapeFunction>();
      for (size_t i = 0; i < variables.size(); ++i)
        f->setVariable(i, variables[i]);

      LuapeGraphBuilderActionPtr action = LuapeGraphBuilderAction::apply(graph, f, inputs);
      if (!action->isUseless(graph))
        res->append(action);
    }
    else
    {
      ContainerPtr values = function->getVariableCandidateValues(variableIndex, inputTypes);
      size_t n = values->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        variables[variableIndex] = values->getElement(i);
        enumerateFunctionVariables(function, inputs, inputTypes, variables, variableIndex + 1, res);
      }
    }
  }*/
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderState> LuapeGraphBuilderStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_BUILDER_H_

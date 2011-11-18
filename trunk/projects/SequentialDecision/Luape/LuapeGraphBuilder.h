/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphBuilder.h            | Luape Graph Builder             |
| Author  : Francis Maes                   |  Decision Problem               |
| Started : 25/10/2011 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_BUILDER_H_
# define LBCPP_LUAPE_GRAPH_BUILDER_H_

# include "LuapeGraph.h"
# include "LuapeProblem.h"
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
      std::vector<LuapeNodePtr> functionInputs(state.function->getNumInputs());
      for (size_t i = 0; i < functionInputs.size(); ++i)
        functionInputs[i] = state.stack[state.stack.size() - state.function->getNumInputs() + i];
      ContainerPtr candidateValues = state.function->getVariableCandidateValues(state.functionNumArguments, functionInputs);
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
  {
    size_t n = function->getNumInputs();
    if (n > state.stack.size())
      return false;

    if (function->getClassName() == T("StumpFunction"))
    {
      TypePtr inputType = state.stack.back()->getType();
      return inputType->inheritsFrom(doubleType) || inputType->inheritsFrom(integerType);
    }

    if (function->getClassName() == T("BooleanAndFunction"))
    {
       size_t firstStackIndex = state.stack.size() - 2;
       if (state.stack[firstStackIndex] == state.stack[firstStackIndex + 1])
         return false; // remove x && x
    }

    size_t firstStackIndex = state.stack.size() - n;
    for (size_t i = 0; i < n; ++i)
      if (!function->doAcceptInputType(i, state.stack[firstStackIndex + i]->getType()))
        return false;
    return true;
  }

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


class LuapeGraphBuilderState : public DecisionProblemState
{
public:
  LuapeGraphBuilderState(const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, size_t maxSteps)
    : problem(problem), graph(graph), maxSteps(maxSteps), numSteps(0) {}
  LuapeGraphBuilderState() : maxSteps(0), numSteps(0) {}

  virtual String toShortString() const
    {return graph->getLastNode()->toShortString();}

  virtual TypePtr getActionType() const
    {return luapeNodeClass;}

  virtual ContainerPtr getAvailableActions() const
  {
    if (numSteps >= maxSteps)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(luapeNodeClass, 0);

    // accessor actions
    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      TypePtr nodeType = graph->getNodeType(i);
      if (nodeType->inheritsFrom(objectClass))
      {
        size_t nv = nodeType->getNumMemberVariables();
        for (size_t j = 0; j < nv; ++j)
          res->append(graph->getUniverse()->makeFunctionNode(getVariableLuapeFunction(j), graph->getNode(i)));
      }
    }
    
    // function actions
    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      LuapeFunctionPtr function = problem->getFunction(i);
      std::vector<LuapeNodePtr> arguments;
      enumerateFunctionActionsRecursively(function, arguments, res);
    }

    // yield actions
    for (size_t i = 0; i < n; ++i)
      if (graph->getNodeType(i) == booleanType)
        res->append(new LuapeYieldNode(graph->getNode(i)));
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    const LuapeNodePtr& node = action.getObjectAndCast<LuapeNode>();
    bool ok = graph->pushNode(context, node);
    jassert(ok);
    reward = 0.0;
    ++numSteps;
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    --numSteps;
    graph->popNode();
    return true;
  }

  virtual bool isFinalState() const
    {return numSteps >= maxSteps || (numSteps && graph->getLastNode().isInstanceOf<LuapeYieldNode>());}

  LuapeGraphPtr getGraph() const
    {return graph;}

protected:
  friend class LuapeGraphBuilderStateClass;

  LuapeProblemPtr problem;
  LuapeGraphPtr graph;
  size_t maxSteps;

  size_t numSteps;

  void enumerateFunctionActionsRecursively(const LuapeFunctionPtr& function, std::vector<LuapeNodePtr>& arguments, const ObjectVectorPtr& res) const
  {
    size_t expectedNumArguments = function->getNumInputs();
    if (arguments.size() == expectedNumArguments)
      res->append(graph->getUniverse()->makeFunctionNode(function, arguments));
    else
    {
      jassert(arguments.size() < expectedNumArguments);
      size_t n = graph->getNumNodes();
      for (size_t i = 0; i < n; ++i)
        if (function->doAcceptInputType(arguments.size(), graph->getNodeType(i)))
        {
          arguments.push_back(graph->getNode(i));
          enumerateFunctionActionsRecursively(function, arguments, res);
          arguments.pop_back();
        }
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderState> LuapeGraphBuilderStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_BUILDER_H_

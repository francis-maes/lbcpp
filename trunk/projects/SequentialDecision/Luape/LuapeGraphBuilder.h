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
      res += graph->getNode(state.stack[i])->toShortString();
      if (i < state.stack.size() - 1)
        res += T(", ");
    }
    res += T("}");
    if (state.function)
    {
      res += T(" ") + state.function->toShortString();
      for (size_t i = 0; i < state.functionNumArguments; ++i)
        res += T(" ") + state.stack[state.stack.size() - state.functionNumArguments + i];
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
      String className = state.function->getClassName();
      if (className == T("GetVariableFunction"))
      {
        jassert(state.stack.size());
        TypePtr objectClass = graph->getNodeType(state.stack.back());
        for (size_t i = 0; i < objectClass->getNumMemberVariables(); ++i)
          res->append(i);
      }
      else if (className == T("StumpFunction"))
      {
        jassert(state.stack.size());
        LuapeNodeCachePtr cache = graph->getNode(state.stack.back())->getCache();
        jassert(cache->isConvertibleToDouble());
        
        const std::vector< std::pair<size_t, double> >& sortedDoubleValues = cache->getSortedDoubleValues();
        if (sortedDoubleValues.size())
        {
          jassert(sortedDoubleValues.size());
          double previousThreshold = sortedDoubleValues[0].second;
          for (size_t i = 0; i < sortedDoubleValues.size(); ++i)
          {
            double threshold = sortedDoubleValues[i].second;
            jassert(threshold >= previousThreshold);
            if (threshold > previousThreshold)
            {
              res->append((threshold + previousThreshold) / 2.0);
              previousThreshold = threshold;
            }
          }
        }
        else
          jassert(false); // no training data, cannot choose thresholds
      }
      else
      {
        jassert(false);
      }
    }
    else
    {
      size_t numRemainingSteps = maxSteps - numSteps;
      if (numRemainingSteps > state.stack.size())
      {
        size_t n = graph->getNumNodes();
        for (size_t i = 0; i < n; ++i)
          res->append(Variable(i)); // push node action
      }

      if (numRemainingSteps > 1)
      {
        for (size_t i = 0; i < problem->getNumFunctions(); ++i)
        {
          FunctionPtr function = problem->getFunction(i);
          if (isFunctionAvailable(function) && numRemainingSteps >= 1 + function->getNumVariables() + (state.stack.size() - function->getNumRequiredInputs() + 1))
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
      if (action.getType() == positiveIntegerType)
      {
        state.stack.push_back((size_t)action.getInteger());
      }
      else if (action.getType()->inheritsFrom(functionClass))
      {
        state.function = action.getObjectAndCast<Function>();
        state.functionNumArguments = 0;
        tryToResolveFunction(context);
      }
      else if (action.getType() == luapeYieldNodeClass)
      {
        const LuapeNodePtr& node = action.getObjectAndCast<LuapeNode>();
        graph->pushNode(context, node);
        state.stack.pop_back();
      }
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

    std::vector<size_t> stack;
    FunctionPtr function;
    size_t functionNumArguments;
    size_t numNodes;
  };
  State state;

  bool isFunctionAvailable(const FunctionPtr& function) const
  {
    size_t n = function->getNumRequiredInputs();
    if (n > state.stack.size())
      return false;

    if (function->getClassName() == T("StumpFunction"))
    {
      TypePtr inputType = graph->getNodeType(state.stack.back());
      return inputType->inheritsFrom(doubleType) || inputType->inheritsFrom(integerType);
    }

    size_t firstStackIndex = state.stack.size() - n;
    for (size_t i = 0; i < n; ++i)
      if (!graph->getNodeType(state.stack[firstStackIndex + i])->inheritsFrom(function->getRequiredInputType(i, n)))
        return false;
    return true;
  }

  bool isYieldAvailable() const
    {return state.stack.size() && graph->getNodeType(state.stack.back())->inheritsFrom(booleanType);}

  void tryToResolveFunction(ExecutionContext& context)
  {
    if (state.functionNumArguments >= state.function->getClass()->getNumMemberVariables())
    {
      // add function node into graph
      size_t numInputs = state.function->getNumRequiredInputs();
      std::vector<size_t> inputs(numInputs);
      if (numInputs)
        memcpy(&inputs[0], &state.stack[state.stack.size() - numInputs], sizeof (size_t) * numInputs);
      LuapeNodePtr node = new LuapeFunctionNode(state.function, inputs);
      bool ok = graph->pushNode(context, node);
      jassert(ok);

      // remove arguments from stack
      state.stack.erase(state.stack.begin() + state.stack.size() - numInputs, state.stack.end());
      state.function = FunctionPtr();
      state.functionNumArguments = 0;

      // push result onto stack
      state.stack.push_back(graph->getNumNodes() - 1);
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
        std::vector<size_t> arguments(1, i);
        size_t nv = nodeType->getNumMemberVariables();
        for (size_t j = 0; j < nv; ++j)
          res->append(new LuapeFunctionNode(getVariableFunction(j), arguments));
      }
    }
    
    // function actions
    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      FunctionPtr function = problem->getFunction(i);
      std::vector<size_t> arguments;
      enumerateFunctionActionsRecursively(function, arguments, res);
    }

    // yield actions
    for (size_t i = 0; i < n; ++i)
      if (graph->getNodeType(i) == booleanType)
        res->append(new LuapeYieldNode(i));
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

  void enumerateFunctionActionsRecursively(const FunctionPtr& function, std::vector<size_t>& arguments, const ObjectVectorPtr& res) const
  {
    size_t expectedNumArguments = function->getNumRequiredInputs();
    if (arguments.size() == expectedNumArguments)
      res->append(new LuapeFunctionNode(function, arguments));
    else
    {
      jassert(arguments.size() < expectedNumArguments);
      TypePtr expectedType = function->getRequiredInputType(arguments.size(), expectedNumArguments);
      size_t n = graph->getNumNodes();
      for (size_t i = 0; i < n; ++i)
        if (graph->getNodeType(i)->inheritsFrom(expectedType))
        {
          arguments.push_back(i);
          enumerateFunctionActionsRecursively(function, arguments, res);
          arguments.pop_back();
        }
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderState> LuapeGraphBuilderStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_BUILDER_H_

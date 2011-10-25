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

class LuapeGraphBuilderState : public DecisionProblemState
{
public:
  LuapeGraphBuilderState(const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeGraphCachePtr& cache, size_t maxSteps)
    : problem(problem), graph(graph), cache(cache), maxSteps(maxSteps), numSteps(0) {}
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

    // todo: stump actions

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
  LuapeGraphCachePtr cache;
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

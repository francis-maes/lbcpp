/*-----------------------------------------.---------------------------------.
| Filename: NodeBuilderDecisionProblem.h   | Luape Node Builder              |
| Author  : Francis Maes                   |  Decision Problem               |
| Started : 25/10/2011 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_
# define LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_

# include "NodeBuilderTypeSearchSpace.h"
# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

class LuapeGraphBuilderAction;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderAction> LuapeGraphBuilderActionPtr;

class LuapeGraphBuilderAction : public Object
{
public:
  LuapeGraphBuilderAction(size_t numNodesToRemove, const LuapeNodePtr& nodeToAdd)
    : numNodesToRemove(numNodesToRemove), nodeToAdd(nodeToAdd) {}
  LuapeGraphBuilderAction() : numNodesToRemove(0) {}

  static LuapeGraphBuilderActionPtr push(const LuapeNodePtr& node)
    {return new LuapeGraphBuilderAction(0, node);}

  static LuapeGraphBuilderActionPtr apply(const LuapeUniversePtr& universe, const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
    {return new LuapeGraphBuilderAction(inputs.size(), universe->makeFunctionNode(function, inputs));}

  static LuapeGraphBuilderActionPtr yield()
    {return new LuapeGraphBuilderAction(0, LuapeNodePtr());}

  const LuapeNodePtr& getNodeToAdd() const
    {return nodeToAdd;}

  bool isYield() const
    {return numNodesToRemove == 0 && !nodeToAdd;}
  
  virtual String toShortString() const
  {
    if (numNodesToRemove == 0)
      return nodeToAdd ? T("push(") + nodeToAdd->toShortString() + T(")") : T("yield");
    else
      return T("apply(") + nodeToAdd.staticCast<LuapeFunctionNode>()->getFunction()->toShortString() + T(")");
  }

  //bool isUseless(const LuapeGraphPtr& graph) const
  //  {return nodeToAdd && isNewNode() && graph->containsNode(nodeToAdd);}

  void perform(std::vector<LuapeNodePtr>& stack)
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
      stack.push_back(nodeToAdd);
  }

  void undo(std::vector<LuapeNodePtr>& stack)
  {
    if (nodeToAdd)
    {
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
  std::vector<LuapeNodePtr> removedNodes; // use in state backup only
};

extern ClassPtr luapeGraphBuilderActionClass;

class LuapeGraphBuilderState : public DecisionProblemState
{
public:
  LuapeGraphBuilderState(const LuapeInferencePtr& function, LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace)
    : function(function), typeSearchSpace(typeSearchSpace), typeState(typeSearchSpace->getInitialState()), numSteps(0), isAborted(false), isYielded(false)
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
      for (size_t i = 0; i < function->getNumInputs(); ++i)
      {
        LuapeNodePtr node = function->getInput(i);
        if (typeState->hasPushAction(node->getType()))
          res->append(LuapeGraphBuilderAction::push(node));
      }
      const std::set<LuapeNodePtr>& activeVariables = function->getActiveVariables();
      for (std::set<LuapeNodePtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
      {
        LuapeNodePtr node = *it;
        if (typeState->hasPushAction(node->getType()))
          res->append(LuapeGraphBuilderAction::push(node));
      }
    }
    if (typeState->hasApplyActions())
    {
      const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& apply = typeState->getApplyActions();
      for (size_t i = 0; i < apply.size(); ++i)
      {
        LuapeFunctionPtr function = apply[i].first;
        if (function->acceptInputsStack(stack))
        {
          size_t numInputs = function->getNumInputs();
          std::vector<LuapeNodePtr> inputs(numInputs);
          for (size_t i = 0; i < numInputs; ++i)
            inputs[i] = stack[stack.size() - numInputs + i];

          LuapeGraphBuilderActionPtr action = LuapeGraphBuilderAction::apply(this->function->getUniverse(), function, inputs);
          // if (!action->isUseless(graph)) // useless == the created node already exists in the graph
            res->append(action);
        }
      }
    }

    if (typeState->hasYieldAction())
      res->append(LuapeGraphBuilderAction::yield());
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& a, double& reward, Variable* stateBackup = NULL)
  {
    const LuapeGraphBuilderActionPtr& action = a.getObjectAndCast<LuapeGraphBuilderAction>();
    if (stateBackup)
      *stateBackup = action;
    action->perform(stack);
    LuapeNodePtr nodeToAdd = action->getNodeToAdd();
    /*
    if (action->isNewNode())
    {
      nodeToAdd->updateCache(context, true);
      if (nodeToAdd.isInstanceOf<LuapeFunctionNode>() && !nodeKeys->isNodeKeyNew(nodeToAdd))
      {
        context.informationCallback(T("Aborded: ") + nodeToAdd->toShortString());
        isAborted = true; // create a node that has the same key as an already existing node in the graph is forbidden 
        // note that in the current implementation, we only compare to the nodes existing in the current initial graph
        //  (not intermediary nodes created along the builder trajectory)
      }
    }*/
    reward = 0.0;
    ++numSteps;
    availableActions = ContainerPtr();
    if (action->isYield())
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
    action->undo(stack);
    availableActions = ContainerPtr();
    updateTypeState();
    return true;
  }

  virtual bool isFinalState() const
    {return isAborted || isYielded || !typeState || !typeState->hasAnyAction();}

  LuapeInferencePtr getFunction() const
    {return function;}

  size_t getCurrentStep() const
    {return numSteps;}

  size_t getStackSize() const
    {return stack.size();}

  const LuapeNodePtr& getStackElement(size_t index) const
    {jassert(index < stack.size()); return stack[index];}

  void setStackElement(size_t index, const LuapeNodePtr& node)
    {jassert(index < stack.size()); stack[index] = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraphBuilderStateClass;

  LuapeInferencePtr function;
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
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderState> LuapeGraphBuilderStatePtr;
extern ClassPtr luapeGraphBuilderStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_

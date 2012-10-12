/*-----------------------------------------.---------------------------------.
| Filename: NodeBuilderDecisionProblem.h   | Luape Node Builder              |
| Author  : Francis Maes                   |  Decision Problem               |
| Started : 25/10/2011 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_
# define LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_

# include <lbcpp-ml/PostfixExpression.h>
# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

class ExpressionBuilderState : public DecisionProblemState
{
public:
  ExpressionBuilderState(const ExpressionDomainPtr& function, PostfixExpressionTypeSpacePtr typeSearchSpace, const PostfixExpressionSequencePtr& subSequence = PostfixExpressionSequencePtr())
    : function(function), typeSearchSpace(typeSearchSpace), typeState(typeSearchSpace->getInitialState()), numSteps(0), isAborted(false), isYielded(false)
  {
    if (subSequence)
    {
      for (size_t i = 0; i < subSequence->getLength(); ++i)
        PostfixExpressionSequence::apply(function->getUniverse(), stack, subSequence->getElement(i));
    }
  }
  ExpressionBuilderState() : numSteps(0), isAborted(false), isYielded(false) {}

  virtual String toShortString() const
  {
    String seps = isFinalState() ? T("[]") : T("{}");

    String res = isYielded ? "yielded - " : String::empty;
    if (isAborted)
      res += T("canceled - ");
    res += seps[0];
    res += String((int)numSteps) + T(": ");
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
    {return objectClass;}
      
  virtual ContainerPtr getAvailableActions() const
  {
    if (availableActions)
      return availableActions;

    if (!typeState)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(objectClass, 0);
    const_cast<ExpressionBuilderState* >(this)->availableActions = res;

    if (typeState->hasPushActions())
    {
      // constants
      for (size_t i = 0; i < function->getNumConstants(); ++i)
        addPushActionIfAvailable(res, function->getConstant(i));

      // inputs
      for (size_t i = 0; i < function->getNumInputs(); ++i)
        addPushActionIfAvailable(res, function->getInput(i));

      // active variables
      const std::set<ExpressionPtr>& activeVariables = function->getActiveVariables();
      for (std::set<ExpressionPtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
        addPushActionIfAvailable(res, *it);
    }
    if (typeState->hasApplyActions())
    {
      const std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> >& apply = typeState->getApplyActions();
      for (size_t i = 0; i < apply.size(); ++i)
      {
        FunctionPtr function = apply[i].first;
        if (function->acceptInputsStack(stack))
        {
          res->append(function);
          /*size_t numInputs = function->getNumInputs();
          std::vector<ExpressionPtr> inputs(numInputs);
          for (size_t i = 0; i < numInputs; ++i)
            inputs[i] = stack[stack.size() - numInputs + i];

          ExpressionBuilderActionPtr action = ExpressionBuilderAction::apply(this->function->getUniverse(), function, inputs);
          // if (!action->isUseless(graph)) // useless == the created node already exists in the graph
            res->append(action);*/
        }
      }
    }

    if (typeState->hasYieldAction())
      res->append(ObjectPtr());
    return res;
  }

  struct Backup : public Object
  {
    Backup(const std::vector<ExpressionPtr>& stack)
      : stack(stack) {}

    std::vector<ExpressionPtr> stack;
  };

  virtual void performTransition(ExecutionContext& context, const Variable& a, double& reward, Variable* stateBackup = NULL)
  {
    const ObjectPtr& action = a.getObject();
    if (stateBackup)
      *stateBackup = Variable(new Backup(stack), objectClass);
    if (action)
      PostfixExpressionSequence::apply(function->getUniverse(), stack, action);

    reward = 0.0;
    ++numSteps;
    availableActions = ContainerPtr();
    if (action == ObjectPtr()) // yield
    {
      isYielded = true;
      typeState = PostfixExpressionTypeStatePtr();
    }
    else
      updateTypeState();
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    isAborted = isYielded = false;
    --numSteps;
    stack = stateBackup.getObjectAndCast<Backup>()->stack;
    availableActions = ContainerPtr();
    updateTypeState();
    return true;
  }

  virtual bool isFinalState() const
    {return isAborted || isYielded || !typeState || !typeState->hasAnyAction();}

  ExpressionDomainPtr getFunction() const
    {return function;}

  size_t getCurrentStep() const
    {return numSteps;}

  size_t getStackSize() const
    {return stack.size();}

  const ExpressionPtr& getStackElement(size_t index) const
    {jassert(index < stack.size()); return stack[index];}

  void setStackElement(size_t index, const ExpressionPtr& node)
    {jassert(index < stack.size()); stack[index] = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExpressionBuilderStateClass;

  ExpressionDomainPtr function;
  PostfixExpressionTypeSpacePtr typeSearchSpace;
  PostfixExpressionTypeStatePtr typeState;
  ContainerPtr availableActions;
  //ExpressionKeysMapPtr nodeKeys;

  std::vector<ExpressionPtr> stack;
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

  void addPushActionIfAvailable(const ObjectVectorPtr& availableActions, const ExpressionPtr& node) const
  {
    if (typeState->hasPushAction(node->getType()))
      availableActions->append(node);
  }
};

typedef ReferenceCountedObjectPtr<ExpressionBuilderState> ExpressionBuilderStatePtr;
extern ClassPtr luapeNodeBuilderStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_

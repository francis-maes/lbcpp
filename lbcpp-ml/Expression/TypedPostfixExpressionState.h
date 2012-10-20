/*-----------------------------------------.---------------------------------.
| Filename: TypedPostfixExpressionState.h  | Postfix Expression Search State |
| Author  : Francis Maes                   | With typing constraints         |
| Started : 08/10/2012 15:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_TYPED_POSTFIX_STATE_H_
# define LBCPP_ML_EXPRESSION_TYPED_POSTFIX_STATE_H_

# include "PostfixExpressionState.h"

namespace lbcpp
{

class TypedPostfixExpressionState : public PostfixExpressionStateBase
{
public:
  TypedPostfixExpressionState(const ExpressionDomainPtr& domain, size_t maxSize)
    : PostfixExpressionStateBase(domain, maxSize)
  {
    typeSearchSpace = domain->getSearchSpace(defaultExecutionContext(), maxSize);
    typeState = typeSearchSpace->getInitialState();
  }
  TypedPostfixExpressionState() {}

  virtual DomainPtr getActionDomain() const
  {
    if (availableActions)
      return availableActions;

    if (!typeState)
      return DomainPtr();

    DiscreteDomainPtr res = new DiscreteDomain();
    const_cast<TypedPostfixExpressionState* >(this)->availableActions = res;

    // push actions
    if (typeState->hasPushActions())
    {
      // constants
      for (size_t i = 0; i < domain->getNumConstants(); ++i)
        addPushActionIfAvailable(res, domain->getConstant(i));

      // inputs
      for (size_t i = 0; i < domain->getNumInputs(); ++i)
        addPushActionIfAvailable(res, domain->getInput(i));

      // active variables
      const std::set<ExpressionPtr>& activeVariables = domain->getActiveVariables();
      for (std::set<ExpressionPtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
        addPushActionIfAvailable(res, *it);
    }

    // apply actions
    if (typeState->hasApplyActions())
    {
      const std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> >& apply = typeState->getApplyActions();
      for (size_t i = 0; i < apply.size(); ++i)
      {
        FunctionPtr function = apply[i].first;
        jassert(function->acceptInputsStack(stack));
        res->addElement(function);
      }
    }

    // yield action
    if (typeState->hasYieldAction())
      res->addElement(ObjectPtr());
    return res;
  }
  
  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL)
  {
    PostfixExpressionStateBase::performTransition(context, action, stateBackup);
    availableActions = DiscreteDomainPtr();
    if (action == ObjectPtr())
      typeState = PostfixExpressionTypeStatePtr();
    else
      updateTypeState();
  }

  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    PostfixExpressionStateBase::undoTransition(context, stateBackup);
    availableActions = DiscreteDomainPtr();
    updateTypeState();
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    PostfixExpressionStateBase::clone(context, target);

    const ReferenceCountedObjectPtr<TypedPostfixExpressionState>& t = target.staticCast<TypedPostfixExpressionState>();
    t->typeSearchSpace = typeSearchSpace;
    t->typeState = typeState;
    t->availableActions = availableActions;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TypedPostfixExpressionStateClass;

  PostfixExpressionTypeSpacePtr typeSearchSpace;
  PostfixExpressionTypeStatePtr typeState;
  DiscreteDomainPtr availableActions;
  
  void updateTypeState()
  {
    std::vector<TypePtr> types(stack.size());
    for (size_t i = 0; i < types.size(); ++i)
      types[i] = stack[i]->getType();
    typeState = typeSearchSpace->getState(trajectory.size(), types);
    jassert(typeState && typeState->hasAnyAction());
  }

  void addPushActionIfAvailable(DiscreteDomainPtr availableActions, const ExpressionPtr& expression) const
  {
    if (typeState->hasPushAction(expression->getType()))
      availableActions->addElement(expression);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_TYPED_POSTFIX_STATE_H_

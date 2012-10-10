/*-----------------------------------------.---------------------------------.
| Filename: ExpressionRPNSearchState.h     | Expression RPN Search State     |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2012 15:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_RPN_SEARCH_STATE_H_
# define LBCPP_ML_EXPRESSION_RPN_SEARCH_STATE_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/ExpressionRPN.h>
# include <lbcpp-ml/Search.h>

namespace lbcpp
{

class ExpressionRPNSearchState : public SearchState
{
public:
  ExpressionRPNSearchState(const ExpressionDomainPtr& domain, ExpressionRPNTypeSpacePtr typeSearchSpace, const ExpressionRPNSequencePtr& subSequence = ExpressionRPNSequencePtr())
    : domain(domain), typeSearchSpace(typeSearchSpace), typeState(typeSearchSpace->getInitialState()), numSteps(0), isYielded(false)
  {
    if (subSequence)
    {
      for (size_t i = 0; i < subSequence->getLength(); ++i)
        ExpressionRPNSequence::apply(domain->getUniverse(), stack, subSequence->getElement(i));
    }
  }
  ExpressionRPNSearchState() : numSteps(0), isYielded(false) {}

  virtual String toShortString() const
  {
    String seps = isFinalState() ? T("[]") : T("{}");

    String res = isYielded ? "yielded - " : String::empty;
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

  virtual DomainPtr getActionDomain() const
  {
    if (availableActions)
      return availableActions;

    if (!typeState)
      return DomainPtr();

    DiscreteDomainPtr res = new DiscreteDomain();
    const_cast<ExpressionRPNSearchState* >(this)->availableActions = res;

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
    if (typeState->hasApplyActions())
    {
      const std::vector<std::pair<FunctionPtr, ExpressionRPNTypeStatePtr> >& apply = typeState->getApplyActions();
      for (size_t i = 0; i < apply.size(); ++i)
      {
        FunctionPtr function = apply[i].first;
        if (function->acceptInputsStack(stack))
        {
          res->addElement(function);
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
      res->addElement(ObjectPtr());
    return res;
  }
  
  virtual size_t getActionCode(const ObjectPtr& action) const
  {
    ActionCodeMap::const_iterator it = actionCodes.find(action);
    if (it == actionCodes.end())
    {
      size_t res = actionCodes.size();
      const_cast<ExpressionRPNSearchState* >(this)->actionCodes[action] = res;
      return res * 10 + getCurrentStep();
    }
    else
      return it->second * 10 + getCurrentStep();
    /*
    std::vector<ExpressionPtr> stack = getStack();
    if (action)
      ExpressionRPNSequence::apply(domain->getUniverse(), stack, action);
    return stack.back()->getAllocationIndex();
    */
  }

  struct Backup : public Object
  {
    Backup(const std::vector<ExpressionPtr>& stack)
      : stack(stack) {}

    std::vector<ExpressionPtr> stack;
  };
  
  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL)
  {
    if (stateBackup)
      *stateBackup = Variable(new Backup(stack), objectClass);
    if (action)
      ExpressionRPNSequence::apply(domain->getUniverse(), stack, action);

    ++numSteps;
    availableActions = DiscreteDomainPtr();
    if (action == ObjectPtr()) // yield
    {
      isYielded = true;
      typeState = ExpressionRPNTypeStatePtr();
    }
    else
      updateTypeState();
  }

  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    isYielded = false;
    --numSteps;
    stack = stateBackup.getObjectAndCast<Backup>()->stack;
    availableActions = DiscreteDomainPtr();
    updateTypeState();
  }

  virtual bool isFinalState() const
    {return isYielded || !typeState || !typeState->hasAnyAction();}

  ExpressionDomainPtr getDomain() const
    {return domain;}

  size_t getCurrentStep() const
    {return numSteps;}
    
  const std::vector<ExpressionPtr>& getStack() const
    {return stack;}

  size_t getStackSize() const
    {return stack.size();}

  const ExpressionPtr& getStackElement(size_t index) const
    {jassert(index < stack.size()); return stack[index];}

  void setStackElement(size_t index, const ExpressionPtr& node)
    {jassert(index < stack.size()); stack[index] = node;}

  virtual ObjectPtr getConstructedObject() const
    {jassert(stack.size() == 1); return stack[0];}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExpressionRPNSearchStateClass;

  ExpressionDomainPtr domain;
  ExpressionRPNTypeSpacePtr typeSearchSpace;
  ExpressionRPNTypeStatePtr typeState;
  DiscreteDomainPtr availableActions;
  //ExpressionKeysMapPtr nodeKeys;

  std::vector<ExpressionPtr> stack;
  size_t numSteps;
  bool isYielded;
  
  typedef std::map<ObjectPtr, size_t, ObjectComparator> ActionCodeMap;
  ActionCodeMap actionCodes;

  void updateTypeState()
  {
    std::vector<TypePtr> types(stack.size());
    for (size_t i = 0; i < types.size(); ++i)
      types[i] = stack[i]->getType();
    typeState = typeSearchSpace->getState(numSteps, types);
    jassert(typeState);
  }

  void addPushActionIfAvailable(DiscreteDomainPtr availableActions, const ExpressionPtr& expression) const
  {
    if (typeState->hasPushAction(expression->getType()))
      availableActions->addElement(expression);
  }
};

typedef ReferenceCountedObjectPtr<ExpressionRPNSearchState> ExpressionRPNSearchStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_RPN_SEARCH_STATE_H_

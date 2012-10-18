/*-----------------------------------------.---------------------------------.
| Filename: PostfixExpressionState.h       | Postfix Expression Search State |
| Author  : Francis Maes                   |                                 |
| Started : 12/10/2012 11:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_POSTFIX_STATE_H_
# define LBCPP_ML_EXPRESSION_POSTFIX_STATE_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/PostfixExpression.h>
# include <lbcpp-ml/Search.h>
# include "ExpressionActionDomainsCache.h"

namespace lbcpp
{

class PostfixExpressionStateBase : public ExpressionState
{
public:
  PostfixExpressionStateBase(const ExpressionDomainPtr& domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator)
    : ExpressionState(domain, maxSize, codeGenerator), numSteps(0), isYielded(false) {}
  PostfixExpressionStateBase() : numSteps(0), isYielded(false) {}
  
  virtual String toShortString() const
  {
    String res;
    if (!isFinalState())
      res += "{";
    for (size_t i = 0; i < stack.size(); ++i)
    {
      res += stack[i]->toShortString();
      if (i < stack.size() - 1)
        res += T(", ");
    }
    if (!isFinalState())
      res += "}";
    return res;
  }
  
  virtual size_t getActionCode(const ObjectPtr& action) const
    {return actionCodeGenerator->getActionCode(domain, action, numSteps, stack.size(), maxSize);}
  
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
      PostfixExpressionSequence::apply(domain->getUniverse(), stack, action);
    else
      isYielded = true;
    ++numSteps;
  }

  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    isYielded = false;
    --numSteps;
    stack = stateBackup.getObjectAndCast<Backup>()->stack;
  }

  virtual bool isFinalState() const
  {
    jassert(numSteps < maxSize || stack.size() == 1);
    return isYielded || numSteps == maxSize;
  }
  
  virtual ObjectPtr getConstructedObject() const
    {jassert(stack.size() == 1); return stack[0];}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    const ReferenceCountedObjectPtr<PostfixExpressionStateBase>& t = target.staticCast<PostfixExpressionStateBase>();
    ExpressionState::clone(context, target);
    t->stack = stack;
    t->numSteps = numSteps;
    t->isYielded = isYielded;
  }

protected:
  std::vector<ExpressionPtr> stack;
  size_t numSteps;
  bool isYielded;
};

class PostfixExpressionState : public PostfixExpressionStateBase
{
public:
  PostfixExpressionState(const ExpressionDomainPtr& domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator)
    : PostfixExpressionStateBase(domain, maxSize, codeGenerator)
  {
    ExpressionActionDomainsCachePtr actionsCache = new ExpressionActionDomainsCache(domain);
    maxFunctionArity = actionsCache->getMaxFunctionArity();
    actionsCacheByCase.resize((maxFunctionArity + 1) * (maxFunctionArity + 1) * 2);
    for (size_t minArity = 0; minArity <= maxFunctionArity; ++minArity)
      for (size_t maxArity = 0; maxArity <= maxFunctionArity; ++maxArity)
        for (size_t isYieldable = 0; isYieldable <= 1; ++isYieldable)
        {
          size_t caseIndex = makeActionCaseIndex(minArity, maxArity, isYieldable == 1);
          std::vector<size_t> actionSubsets;
          for (size_t a = minArity; a <= maxArity; ++a)
            actionSubsets.push_back(a);
          if (isYieldable)
            actionSubsets.push_back(maxFunctionArity + 1); // refers to the singleton containing the "yield" action
          actionsCacheByCase[caseIndex] = actionsCache->getActions(actionSubsets);
        }
  }
  PostfixExpressionState() {}

  virtual DomainPtr getActionDomain() const
  {
    jassert(numSteps < maxSize);
    size_t maxArity = (size_t)juce::jmin((int)maxFunctionArity, (int)stack.size()); // cannot apply a n-ary operator if there are no n elements on the stack
    size_t minArity = (size_t)juce::jlimit(0, (int)maxFunctionArity, (int)stack.size() - (int)((maxSize - numSteps - 1) * (maxFunctionArity - 1)));
    bool isYieldable = stack.size() == 1;
    jassert(numSteps < maxSize || isYieldable); // if the budget has been exhausted, the result must be yieldable
    return actionsCacheByCase[makeActionCaseIndex(minArity, maxArity, isYieldable)];
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    PostfixExpressionStateBase::clone(context, target);
    const ReferenceCountedObjectPtr<PostfixExpressionState>& t = target.staticCast<PostfixExpressionState>();
    t->maxFunctionArity = maxFunctionArity;
    t->actionsCacheByCase = actionsCacheByCase;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PostfixExpressionStateClass;

  size_t maxFunctionArity;
  std::vector<DiscreteDomainPtr> actionsCacheByCase;

  size_t makeActionCaseIndex(size_t minArity, size_t maxArity, bool isYieldable) const
    {return (minArity * (maxFunctionArity + 1) + maxArity) * 2 + (isYieldable ? 1 : 0);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_TYPED_POSTFIX_STATE_H_

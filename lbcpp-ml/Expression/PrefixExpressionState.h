/*-----------------------------------------.---------------------------------.
| Filename: PrefixExpressionState.h        | Prefix Expression Search State  |
| Author  : Francis Maes                   |                                 |
| Started : 12/10/2012 10:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_PREFIX_STATE_H_
# define LBCPP_ML_EXPRESSION_PREFIX_STATE_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/Search.h>
# include "ExpressionActionDomainsCache.h"

namespace lbcpp
{

class PrefixExpressionState : public ExpressionState
{
public:
  PrefixExpressionState(ExpressionDomainPtr domain, size_t maxSize)
    : ExpressionState(domain, maxSize), numLeafs(1)
  {
    ExpressionActionDomainsCachePtr actionsCache = new ExpressionActionDomainsCache(domain);
    actionsByMaxArity.resize(actionsCache->getMaxFunctionArity() + 1);
    std::vector<size_t> actionsSubset;
    for (size_t i = 0; i < actionsByMaxArity.size(); ++i)
    {
      actionsSubset.push_back(i);
      actionsByMaxArity[i] = actionsCache->getActions(actionsSubset);
    }
  }
  PrefixExpressionState() {}

  virtual DomainPtr getActionDomain() const
  {
    size_t maxArity = maxSize - sequence.size() - numLeafs;
    jassert(maxArity >= 0);
    return maxArity < actionsByMaxArity.size() ? actionsByMaxArity[maxArity] : actionsByMaxArity.back();
  }

  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL)
  {
    sequence.push_back(action);
    FunctionPtr function = action.dynamicCast<Function>();
    numLeafs += (function ? function->getNumInputs() : 0) - 1;
  }

  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    jassert(sequence.size());
    FunctionPtr function = sequence.back().dynamicCast<Function>();
    numLeafs -= (function ? function->getNumInputs() : 0) - 1;
    sequence.pop_back();
  }

  virtual bool isFinalState() const
    {return numLeafs == 0;}
  
  virtual size_t getActionCode(const ObjectPtr& action) const
    {return actionCodeGenerator->getActionCode(action, sequence.size(), maxSize);}

  virtual ObjectPtr getConstructedObject() const
  {
    size_t position = 0;
    ExpressionPtr res = makeExpression(sequence, position);
    jassert(position == sequence.size());
    return res;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ExpressionState::clone(context, target);

    const ReferenceCountedObjectPtr<PrefixExpressionState>& t = target.staticCast<PrefixExpressionState>();
    t->sequence = sequence;
    t->numLeafs = numLeafs;
    t->actionsByMaxArity = actionsByMaxArity;
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class PrefixExpressionStateClass;

  std::vector<ObjectPtr> sequence;
  size_t numLeafs;

  // actionsByMaxArity[i] = all actions up to arity i (first: constants, second: constants + unary functions, third: constants + unary functions + binary functions ...
  std::vector<DiscreteDomainPtr> actionsByMaxArity;

  ExpressionPtr makeExpression(const std::vector<ObjectPtr>& sequence, size_t& position) const
  {
    jassert(position < sequence.size());
    ObjectPtr symbol = sequence[position];
    ++position;
    ExpressionPtr expression = symbol.dynamicCast<Expression>();
    if (expression)
      return expression;
    else
    {
      FunctionPtr function = symbol.staticCast<Function>();
      std::vector<ExpressionPtr> arguments(function->getNumInputs());
      for (size_t i = 0; i < arguments.size(); ++i)
        arguments[i] = makeExpression(sequence, position);
      return new FunctionExpression(function, arguments); //domain->getUniverse()->makeFunctionExpression(function, arguments);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_PREFIX_STATE_H_

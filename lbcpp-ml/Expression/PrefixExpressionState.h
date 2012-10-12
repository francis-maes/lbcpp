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

namespace lbcpp
{

class PrefixExpressionState : public ExpressionState
{
public:
  PrefixExpressionState(ExpressionDomainPtr domain, size_t maxSize)
    : ExpressionState(domain, maxSize), numLeafs(1)
  {
    terminalActions = new DiscreteDomain();

    // constants
    for (size_t i = 0; i < domain->getNumConstants(); ++i)
      terminalActions->addElement(domain->getConstant(i));

    // inputs
    for (size_t i = 0; i < domain->getNumInputs(); ++i)
      terminalActions->addElement(domain->getInput(i));
    
    // active variables
    const std::set<ExpressionPtr>& activeVariables = domain->getActiveVariables();
    for (std::set<ExpressionPtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
      terminalActions->addElement(*it);

    allActions = terminalActions->cloneAndCast<DiscreteDomain>();
    for (size_t i = 0; i < domain->getNumFunctions(); ++i)
    {
      FunctionPtr function = domain->getFunction(i);
      jassert(function->getNumVariables() == 0); // parameterized functions are not supported yet
      allActions->addElement(function);
    }

    actionCodeGenerator = new ExpressionActionCodeGenerator();
  }
  PrefixExpressionState() {}

  virtual DomainPtr getActionDomain() const
    {return sequence.size() + numLeafs < maxSize ? allActions : terminalActions;}

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
    const ReferenceCountedObjectPtr<PrefixExpressionState>& t = target.staticCast<PrefixExpressionState>();
    t->domain = domain;
    t->maxSize = maxSize;
    t->sequence = sequence;
    t->numLeafs = numLeafs;
    t->terminalActions = terminalActions;
    t->allActions = allActions;
    t->actionCodeGenerator = actionCodeGenerator;
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class PrefixExpressionStateClass;

  std::vector<ObjectPtr> sequence;
  size_t numLeafs;
  DiscreteDomainPtr terminalActions;
  DiscreteDomainPtr allActions;

  ExpressionActionCodeGeneratorPtr actionCodeGenerator;

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
      return domain->getUniverse()->makeFunctionExpression(function, arguments);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_PREFIX_STATE_H_

/*-----------------------------------------.---------------------------------.
| Filename: ExpressionSampler.h            | Expression Sampler              |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2012 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_SAMPLER_H_
# define LBCPP_ML_EXPRESSION_SAMPLER_H_

# include "ExpressionDomain.h"
# include "Sampler.h"

namespace lbcpp
{

class ExpressionSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<ExpressionDomain>();}

protected:
  ExpressionDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<ExpressionSampler> ExpressionSamplerPtr;

class RPNExpressionSampler : public ExpressionSampler
{
public:
  RPNExpressionSampler(size_t expressionSize = 0)
    : expressionSize(expressionSize) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionSampler::initialize(context, domain);
    typeSearchSpace = this->domain->getSearchSpace(context, expressionSize, true);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionUniversePtr universe = domain->getUniverse();
    jassert(typeSearchSpace);

    std::vector<ExpressionPtr> stack;
    PostfixExpressionTypeStatePtr typeState;

    for (size_t i = 0; i < expressionSize; ++i)
    {
      // Retrieve type-state index
      PostfixExpressionTypeStatePtr typeState = getTypeState(i, stack);
      jassert(typeState);

      // Sample action
      ObjectPtr action;
      const size_t numFailuresAllowed = 100;
      size_t numFailures;
      for (numFailures = 0; numFailures < numFailuresAllowed; ++numFailures)
        if (sampleAction(context, typeState, action) && isActionAvailable(action, stack))
          break;
      if (numFailures == numFailuresAllowed)
        return ExpressionPtr();

      // Execute action
      executeAction(stack, action);
      if (!action)
      {
        //context.informationCallback(T("Candidate: ") + stack[0]->toShortString());
        return stack[0]; // yield action
      }
    }

    context.informationCallback("Failed to sample candidate weak node");
    return ExpressionPtr();
  }

protected:
  friend class RPNExpressionSamplerClass;

  size_t expressionSize;
  PostfixExpressionTypeSpacePtr typeSearchSpace;

  virtual bool sampleAction(ExecutionContext& context, PostfixExpressionTypeStatePtr typeState, ObjectPtr& res) const = 0;

  static bool isActionAvailable(ObjectPtr action, const std::vector<ExpressionPtr>& stack)
    {return !action || !action.isInstanceOf<Function>() || action.staticCast<Function>()->acceptInputsStack(stack);}

  PostfixExpressionTypeStatePtr getTypeState(size_t stepNumber, const std::vector<ExpressionPtr>& stack) const
  {
    jassert(typeSearchSpace);
    std::vector<TypePtr> typeStack(stack.size());
    for (size_t j = 0; j < typeStack.size(); ++j)
      typeStack[j] = stack[j]->getType();
    return typeSearchSpace->getState(stepNumber, typeStack);
  }

  void executeAction(std::vector<ExpressionPtr>& stack, const ObjectPtr& action) const
  {
    // Execute action
    if (action)
    {
      if (action.isInstanceOf<Expression>())
        stack.push_back(action);   // push action
      else
      {
        // apply action
        FunctionPtr function = action.staticCast<Function>();
        size_t n = function->getNumInputs();
        jassert(stack.size() >= n && n > 0);
        std::vector<ExpressionPtr> inputs(n);
        for (size_t i = 0; i < n; ++i)
          inputs[i] = stack[stack.size() - n + i];
        stack.erase(stack.begin() + stack.size() - n, stack.end());
        stack.push_back(domain->getUniverse()->makeFunctionExpression(function, inputs));
      }
    }
    else
    {
      // yield action
      jassert(stack.size() == 1);
    }
  }
};

class RandomRPNExpressionSampler : public RPNExpressionSampler
{
public:
  RandomRPNExpressionSampler(size_t expressionSize = 0)
    : RPNExpressionSampler(expressionSize) {}

  virtual bool sampleAction(ExecutionContext& context, PostfixExpressionTypeStatePtr typeState, ObjectPtr& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (!typeState)
      return false;

    std::vector<double> probabilities(3, 0.0);
    double Z = 0.0;
    if (typeState->hasPushActions())
      probabilities[0] = 1.0, ++Z;
    if (typeState->hasApplyActions())
      probabilities[1] = 1.0, ++Z;
    if (typeState->hasYieldAction())
      probabilities[2] = 1.0, ++Z;
    jassert(Z > 0.0);
    size_t actionKind = random->sampleWithProbabilities(probabilities, Z);

    switch (actionKind)
    {
    case 0: // push
      {
        static const size_t numTrials = 10;
        size_t numVariables = domain->getNumInputs() + domain->getNumActiveVariables();
        for (size_t trial = 0; trial < numTrials; ++trial)
        {
          size_t variableIndex = random->sampleSize(numVariables);
          ExpressionPtr variable = variableIndex < domain->getNumInputs()
            ? (ExpressionPtr)domain->getInput(variableIndex)
            : domain->getActiveVariable(variableIndex - domain->getNumInputs());
          if (typeState->hasPushAction(variable->getType()))
          {
            res = variable;
            return true;
          }
        }
        return false;
      }

    case 1: // apply
      {
        const std::vector<std::pair<FunctionPtr, PostfixExpressionTypeStatePtr> >& apply = typeState->getApplyActions();
        jassert(apply.size());
        if (apply.empty())
          return false;
        res = apply[random->sampleSize(apply.size())].first;
        return true;
      }

    case 2: // yield
      res = ObjectPtr();
      return true;
    };

    return false;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_SAMPLER_H_

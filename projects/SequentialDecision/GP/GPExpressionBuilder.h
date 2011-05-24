/*-----------------------------------------.---------------------------------.
| Filename: GPExpressionBuilder.h          | GP Expression Builder           |
| Author  : Francis Maes                   |                                 |
| Started : 23/05/2011 16:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_EXPRESSION_BUILDER_H_
# define LBCPP_GENETIC_PROGRAMMING_EXPRESSION_BUILDER_H_

# include "GPExpression.h"
# include "../Core/DecisionProblem.h"

namespace lbcpp
{

extern ClassPtr gpExpressionBuilderActionClass;

class GPExpressionBuilderAction : public Object
{
public:
  virtual GPExpressionPtr makeExpression(const std::vector<GPExpressionPtr>& currentExpressions) const = 0;
};

typedef ReferenceCountedObjectPtr<GPExpressionBuilderAction> GPExpressionBuilderActionPtr;

class VariableGPExpressionBuilderAction : public GPExpressionBuilderAction
{
public:
  VariableGPExpressionBuilderAction(Variable index)
    : index(index) {}
  VariableGPExpressionBuilderAction() {}
  
  size_t getIndex() const
    {return (size_t)index.getInteger();}

  virtual GPExpressionPtr makeExpression(const std::vector<GPExpressionPtr>& currentExpressions) const
    {return new VariableGPExpression(index);}

protected:
  friend class VariableGPExpressionBuilderActionClass;

  Variable index;
};

class UnaryGPExpressionBuilderAction : public GPExpressionBuilderAction
{
public:
  UnaryGPExpressionBuilderAction(GPPre op, size_t index)
    : op(op), index(index) {}
  UnaryGPExpressionBuilderAction() {}

  virtual GPExpressionPtr makeExpression(const std::vector<GPExpressionPtr>& currentExpressions) const
    {return new UnaryGPExpression(op, currentExpressions[index]);}

protected:
  friend class UnaryGPExpressionBuilderActionClass;

  GPPre op;
  size_t index;
};

class BinaryGPExpressionBuilderAction : public GPExpressionBuilderAction
{
public:
  BinaryGPExpressionBuilderAction(GPOperator op, size_t left, size_t right)
    : op(op), left(left), right(right) {}
  BinaryGPExpressionBuilderAction() {}

  virtual GPExpressionPtr makeExpression(const std::vector<GPExpressionPtr>& currentExpressions) const
    {return new BinaryGPExpression(currentExpressions[left], op, currentExpressions[right]);}

protected:
  friend class BinaryGPExpressionBuilderActionClass;

  GPOperator op;
  size_t left;
  size_t right;
};

class GPExpressionBuilderState : public DecisionProblemState
{
public:
  GPExpressionBuilderState(const String& name, EnumerationPtr inputVariables, FunctionPtr objectiveFunction)
    : DecisionProblemState(name), inputVariables(inputVariables), objectiveFunction(objectiveFunction)
  {
    areVariableUsed.resize(inputVariables->getNumElements(), false);
  }
  GPExpressionBuilderState() {}

  virtual String toShortString() const
  {
    if (expressions.empty())
      return T("<initial>");
    return description;
    //GPExpressionPtr expr = expressions.back();
    //return expr->toShortString() + T(" (") + String((int)expressions.size()) + T(" steps)");
  }

  virtual TypePtr getActionType() const
    {return gpExpressionBuilderActionClass;}

  virtual ContainerPtr getAvailableActions() const
  {
    ObjectVectorPtr res = new ObjectVector(gpExpressionBuilderActionClass);

    size_t numExpressions = expressions.size();

    // variables
    for (size_t i = 0; i < areVariableUsed.size(); ++i)
      if (!areVariableUsed[i])
        res->append(new VariableGPExpressionBuilderAction(Variable(i, inputVariables)));

    if (numExpressions)
    {
      // unary expressions
      size_t n = gpPreEnumeration->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        //for (size_t j = 0; j < numExpressions; ++j)
          res->append(new UnaryGPExpressionBuilderAction((GPPre)i, numExpressions - 1));
      }

      // binary expressions
      n = gpOperatorEnumeration->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        bool isCommutative = (i == 0) || (i == 2); // addition and multiplication
        size_t j = numExpressions - 1;
        for (size_t k = 0; k < numExpressions; ++k)
          res->append(new BinaryGPExpressionBuilderAction((GPOperator)i, j, k));
        if (!isCommutative)
        {
          size_t k = numExpressions - 1;
          for (j = 0; j < numExpressions - 1; ++j)
            res->append(new BinaryGPExpressionBuilderAction((GPOperator)i, j, k));
        }
      }
    }
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward)
  {
    const GPExpressionBuilderActionPtr& builderAction = action.getObjectAndCast<GPExpressionBuilderAction>();
    GPExpressionPtr expression = builderAction->makeExpression(expressions);

    if (builderAction.dynamicCast<VariableGPExpressionBuilderAction>())
    {
      size_t index = builderAction.staticCast<VariableGPExpressionBuilderAction>()->getIndex();
      jassert(!areVariableUsed[index]);
      areVariableUsed[index] = true;
    }

    expressions.push_back(expression);
    double previousScore = expressionScores.size() ? expressionScores.back() : objectiveFunction->compute(context, new ConstantGPExpression(0.0)).toDouble();
    double score = objectiveFunction->compute(context, expression).toDouble();
    expressionScores.push_back(score);
    reward = previousScore - score; // score must be minimized, reward must be maximized

    if (description.isNotEmpty())
      description += T(" -> ");
    description += action.toShortString();
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& action)
  {
    const GPExpressionBuilderActionPtr& builderAction = action.getObjectAndCast<GPExpressionBuilderAction>();
    if (builderAction.dynamicCast<VariableGPExpressionBuilderAction>())
    {
      size_t index = builderAction.staticCast<VariableGPExpressionBuilderAction>()->getIndex();
      jassert(areVariableUsed[index]);
      areVariableUsed[index] = false;
    }

    expressions.pop_back();
    expressionScores.pop_back();
    int i = description.lastIndexOf(T(" -> "));
    description = (i >= 0 ? description.substring(0, i) : String::empty);
    return true;
  }

  virtual bool isFinalState() const
    {return false;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<GPExpressionBuilderState>& target = t.staticCast<GPExpressionBuilderState>();
    target->name = name;
    target->inputVariables = inputVariables;
    target->objectiveFunction = objectiveFunction;
    target->areVariableUsed = areVariableUsed;
    target->expressions.resize(expressions.size());
    for (size_t i = 0; i < expressions.size(); ++i)
      target->expressions[i] = expressions[i]->cloneAndCast<GPExpression>(context);
    target->expressionScores = expressionScores;
    target->description = description;
  }

  double getScore() const
    {return expressionScores.size() ? expressionScores.back() : DBL_MAX;}

  GPExpressionPtr getExpression() const
    {return expressions.size() ? expressions.back() : GPExpressionPtr();}

protected:
  friend class GPExpressionBuilderStateClass;

  EnumerationPtr inputVariables;
  FunctionPtr objectiveFunction;
  std::vector<bool> areVariableUsed;
  std::vector<GPExpressionPtr> expressions;
  std::vector<double> expressionScores;
  String description;
};

extern ClassPtr gpExpressionBuilderStateClass;

typedef ReferenceCountedObjectPtr<GPExpressionBuilderState> GPExpressionBuilderStatePtr;

class GPExpressionBuilderInitialStateSampler : public SimpleUnaryFunction
{
public:
  GPExpressionBuilderInitialStateSampler(EnumerationPtr inputVariables, FunctionPtr objectiveFunction)
    : SimpleUnaryFunction(randomGeneratorClass, gpExpressionBuilderStateClass), inputVariables(inputVariables), objectiveFunction(objectiveFunction) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return new GPExpressionBuilderState(T("toto"), inputVariables, objectiveFunction);}

protected:
  EnumerationPtr inputVariables;
  FunctionPtr objectiveFunction;
};


class GPExpressionBuilderProblem : public DecisionProblem
{
public:
  GPExpressionBuilderProblem(EnumerationPtr inputVariables, FunctionPtr objectiveFunction)
    : DecisionProblem(new GPExpressionBuilderInitialStateSampler(inputVariables, objectiveFunction), 1.0) {}
  GPExpressionBuilderProblem() {}

  virtual TypePtr getActionType() const
    {return gpExpressionBuilderActionClass;}

  virtual double getMaxReward() const
    {return DBL_MAX;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_EXPRESSION_BUILDER_H_

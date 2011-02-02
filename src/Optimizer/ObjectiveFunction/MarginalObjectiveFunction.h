/*-----------------------------------------.---------------------------------.
| Filename: MarginalObjectiveFunction.h    | Marginal Objective Function     |
| Author  : Francis Maes                   |                                 |
| Started : 22/12/2010 01:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_MARGINAL_H_
# define LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_MARGINAL_H_

# include <lbcpp/Optimizer/ObjectiveFunction.h>

namespace lbcpp
{

class MarginalObjectiveFunction : public ObjectiveFunction
{
public:
  MarginalObjectiveFunction(ObjectiveFunctionPtr objective, const ObjectPtr& referenceValue, size_t variableIndex)
    : objective(objective), referenceValue(referenceValue), variableIndex(variableIndex)
    {inputType = referenceValue->getVariableType(variableIndex);}

  MarginalObjectiveFunction() : variableIndex(0) {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual double compute(ExecutionContext& context, const Variable& input) const
    {return objective->compute(context, makeValue(context, input));}

  virtual String getDescription(const Variable& input) const
    {return objective->getDescription(makeValue(defaultExecutionContext(), input));}

  String toString() const
    {return objective->toString() + T(" (") + referenceValue->getVariableName(variableIndex) + T(")");}

protected:
  friend class MarginalObjectiveFunctionClass;

  ObjectiveFunctionPtr objective;
  ObjectPtr referenceValue;
  size_t variableIndex;
  TypePtr inputType;

  ObjectPtr makeValue(ExecutionContext& context, const Variable& input) const
  {
    ObjectPtr res = referenceValue->clone(context);
    res->setVariable(variableIndex, input);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_MARGINAL_H_

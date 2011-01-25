/*-----------------------------------------.---------------------------------.
| Filename: EvaluateObjectiveFunctionWor..h| Evaluate Objective Function WU  |
| Author  : Francis Maes                   |                                 |
| Started : 22/12/2010 01:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_WORK_UNIT_H_
# define LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_WORK_UNIT_H_

# include <lbcpp/Optimizer/ObjectiveFunction.h>

namespace lbcpp
{

class EvaluateObjectiveFunctionWorkUnit : public WorkUnit
{
public:
  EvaluateObjectiveFunctionWorkUnit(const String& description, ObjectiveFunctionPtr objective, const Variable& input, double& result)
    : description(description), objective(objective), input(input), result(result)
  {
  }

  EvaluateObjectiveFunctionWorkUnit() : result(*(double* )0) {}

  virtual String toString() const
    {return description;}

protected:
  virtual bool run(ExecutionContext& context)
  {
    result = objective->compute(context, input);
    context.resultCallback(T("Evaluation"), result);
    return true;
  }

  friend class EvaluateObjectiveFunctionWorkUnitClass;

  ObjectiveFunctionPtr objective;
  Variable input;
  double& result;
  String description;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_WORK_UNIT_H_

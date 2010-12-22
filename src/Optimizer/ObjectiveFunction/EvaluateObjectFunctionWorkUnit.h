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
  EvaluateObjectiveFunctionWorkUnit(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result)
    : WorkUnit(name), objective(objective), input(input), result(result)
  {
  }

  EvaluateObjectiveFunctionWorkUnit() : result(*(double* )0) {}

protected:
  virtual bool run(ExecutionContext& context)
  {
    result = objective->compute(context, input);
    context.resultCallback(name, result);
    return true;
  }

  friend class EvaluateObjectiveFunctionWorkUnitClass;

  ObjectiveFunctionPtr objective;
  Variable input;
  double& result;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_WORK_UNIT_H_

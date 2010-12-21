/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveFunction.cpp          | Objective Function Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 22:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Function/ObjectiveFunction.h>
using namespace lbcpp;


/*
** EvaluateObjectiveFunctionWorkUnit
*/
EvaluateObjectiveFunctionWorkUnit::EvaluateObjectiveFunctionWorkUnit(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result)
  : WorkUnit(name), objective(objective), input(input), result(result)
{
}

EvaluateObjectiveFunctionWorkUnit::EvaluateObjectiveFunctionWorkUnit() : result(*(double* )0)
{
}

bool EvaluateObjectiveFunctionWorkUnit::run(ExecutionContext& context)
{
  result = objective->compute(context, input);
  context.resultCallback(name, result);
  return true;
}

/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveFunction.h            | Objective Function Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 01/11/2010 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OBJECTIVE_H_
# define LBCPP_FUNCTION_OBJECTIVE_H_

# include "Function.h"
# include "../Execution/ThreadPool.h"
# include "../Execution/WorkUnit.h"

namespace lbcpp
{

class ObjectiveFunction : public Function
{
public:
  virtual double compute(const Variable& input) const = 0;

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return compute(input);}
};

typedef ReferenceCountedObjectPtr<ObjectiveFunction> ObjectiveFunctionPtr;

extern ClassPtr objectiveFunctionClass;

class EvaluateObjectiveFunctionWorkUnit : public WorkUnit
{
public:
  EvaluateObjectiveFunctionWorkUnit(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result)
    : WorkUnit(name), objective(objective), input(input), result(result) {}
  EvaluateObjectiveFunctionWorkUnit() : result(*(double* )0) {}

protected:
  virtual bool run(ExecutionContext& context)
  {
    result = objective->compute(input);
    return true;
  }

  friend class EvaluateObjectiveFunctionWorkUnitClass;

  ObjectiveFunctionPtr objective;
  Variable input;
  double& result;
};

extern WorkUnitPtr evaluateObjectiveFunctionWorkUnit(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OBJECTIVE_H_

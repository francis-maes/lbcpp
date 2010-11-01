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
# include "ThreadPool.h"

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

class EvaluateObjectiveFunctionJob : public Job
{
public:
  EvaluateObjectiveFunctionJob(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result)
    : Job(name), objective(objective), input(input), result(result) {}
  EvaluateObjectiveFunctionJob() : result(*(double* )0) {}

  virtual String getCurrentStatus() const
    {return T("Evaluating ") + input.toShortString();}

  virtual bool runJob(String& failureReason)
  {
    result = objective->compute(input);
    return true;
  }

protected:
  friend class EvaluateObjectiveFunctionJobClass;

  ObjectiveFunctionPtr objective;
  Variable input;
  double& result;
};

extern JobPtr evaluateObjectiveFunctionJob(const String& name, ObjectiveFunctionPtr objective, const Variable& input, double& result);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OBJECTIVE_H_

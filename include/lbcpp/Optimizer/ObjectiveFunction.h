/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveFunction.h            | Objective Function Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 01/11/2010 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OBJECTIVE_H_
# define LBCPP_FUNCTION_OBJECTIVE_H_

# include "../Function/Function.h"
# include "../Execution/WorkUnit.h"

namespace lbcpp
{

class ObjectiveFunction : public Function
{
public:
  virtual double compute(ExecutionContext& context, const Variable& input) const = 0;

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return compute(context, input);}
};

typedef ReferenceCountedObjectPtr<ObjectiveFunction> ObjectiveFunctionPtr;

extern ClassPtr objectiveFunctionClass;

extern ObjectiveFunctionPtr marginalObjectiveFunction(const ObjectiveFunctionPtr& objective, const ObjectPtr& referenceValue, size_t variableIndex);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OBJECTIVE_H_

/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveFunction.h            | Objective Function Base Class   |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 01/11/2010 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OBJECTIVE_H_
# define LBCPP_FUNCTION_OBJECTIVE_H_

# include "../Core/Function.h"
# include "../Execution/WorkUnit.h"

namespace lbcpp
{

// Variable -> double
class ObjectiveFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? variableType : anyType;} // TODO arnaud : if (index != 0) --> error ?
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  
// TODO arnaud : necessary ?
//  virtual double compute(ExecutionContext& context, const Variable& input) const = 0;   
protected:  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return compute(context, input);}

};

typedef ReferenceCountedObjectPtr<ObjectiveFunction> ObjectiveFunctionPtr;

extern ClassPtr objectiveFunctionClass;

extern ObjectiveFunctionPtr marginalObjectiveFunction(const ObjectiveFunctionPtr& objective, const ObjectPtr& referenceValue, size_t variableIndex);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OBJECTIVE_H_

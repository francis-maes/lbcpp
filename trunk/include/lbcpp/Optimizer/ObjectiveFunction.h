/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveFunction.h            | Objective Function Base Class   |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 01/11/2010 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECTIVE_FUNCTION_H_
# define LBCPP_OBJECTIVE_FUNCTION_H_

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

};

typedef ReferenceCountedObjectPtr<ObjectiveFunction> ObjectiveFunctionPtr;

extern ClassPtr objectiveFunctionClass;

extern ObjectiveFunctionPtr marginalObjectiveFunction(const ObjectiveFunctionPtr& objective, const ObjectPtr& referenceValue, size_t variableIndex);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECTIVE_FUNCTION_H_

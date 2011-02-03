/*-----------------------------------------.---------------------------------.
| Filename: Operator.h                     | Base class for Operators        |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_H_
# define LBCPP_OPERATOR_H_

# include "../Core/Variable.h"
# include "../Core/Vector.h"
# include "../Function/Function.h"

namespace lbcpp
{

class ProxyFunction : public Function
{
protected:
  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    implementation = createImplementation(inputVariables);
    if (!implementation)
    {
      context.errorCallback(T("Could not create implementation in proxy operator"));
      return VariableSignaturePtr();
    }
    if (!implementation->initialize(context, inputVariables))
      return VariableSignaturePtr();

    return implementation->getOutputVariable();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(implementation); return implementation->computeFunction(context, input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(implementation); return implementation->computeFunction(context, inputs);}

  FunctionPtr implementation;
};

extern FunctionPtr accumulateOperator();
extern FunctionPtr discretizeOperator(bool sampleBest = true);
extern FunctionPtr segmentContainerOperator();
extern FunctionPtr applyOnContainerOperator(const FunctionPtr& function);

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_H_

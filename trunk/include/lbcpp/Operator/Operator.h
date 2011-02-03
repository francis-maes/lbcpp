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
/*
class Operator : public Object
{
public:
  bool initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables);
  
  size_t getNumInputs() const
    {return inputVariables.size();}

  const VariableSignaturePtr& getInputVariable(size_t index) const
    {jassert(index < inputVariables.size()); return inputVariables[index];}

  const TypePtr& getInputType(size_t index) const
    {return getInputVariable(index)->getType();}

  const VariableSignaturePtr& getOutputVariable() const
    {return outputVariable;}

  const TypePtr& getOutputType() const
    {return outputVariable->getType();}

  Variable compute(const Variable& input) const
    {jassert(inputVariables.size() == 1); return computeOperator(&input);}

  Variable compute(const Variable* inputs) const
    {return computeOperator(inputs);}

public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context) = 0;
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const = 0;

protected:
  friend class OperatorClass;

  std::vector<VariableSignaturePtr> inputVariables;
  VariableSignaturePtr outputVariable;

  bool checkNumInputs(ExecutionContext& context, size_t numInputs) const;
  bool checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const;
  TypePtr getTemplateArgument(ExecutionContext& context, TypePtr type, size_t templateArgumentIndex, TypePtr requestedType = anyType) const;
  TypePtr getContainerElementsType(ExecutionContext& context, TypePtr type) const;
  TypePtr getDistributionElementsType(ExecutionContext& context, TypePtr type) const;
};

typedef ReferenceCountedObjectPtr<Operator> FunctionPtr;
*/

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

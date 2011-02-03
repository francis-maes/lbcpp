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

namespace lbcpp
{

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
  virtual VariableSignaturePtr initializeOperator(ExecutionContext& context) = 0;
  virtual Variable computeOperator(const Variable* inputs) const = 0;

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

typedef ReferenceCountedObjectPtr<Operator> OperatorPtr;

class ProxyOperator : public Operator
{
protected:
  virtual OperatorPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;

  virtual VariableSignaturePtr initializeOperator(ExecutionContext& context)
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

  virtual Variable computeOperator(const Variable* inputs) const
    {jassert(implementation); return implementation->computeOperator(inputs);}

  OperatorPtr implementation;
};

extern OperatorPtr accumulateOperator();
extern OperatorPtr discretizeOperator(bool sampleBest = true);
extern OperatorPtr segmentContainerOperator();
extern OperatorPtr applyOnContainerOperator(const OperatorPtr& function);

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_H_

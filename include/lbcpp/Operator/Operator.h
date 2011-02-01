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
  bool initialize(ExecutionContext& context, const std::vector<TypePtr>& inputs);
  
  size_t getNumInputs() const
    {return inputTypes.size();}

  TypePtr getInputType(size_t index) const
    {jassert(index < inputTypes.size()); return inputTypes[index];}

  TypePtr getOutputType() const
    {return outputType;}

  Variable compute(const Variable& input) const
    {jassert(inputTypes.size() == 1); return computeOperator(&input);}

  Variable compute(const Variable* inputs) const
    {return computeOperator(inputs);}

public:
  virtual TypePtr initializeOperator(ExecutionContext& context) = 0;
  virtual Variable computeOperator(const Variable* inputs) const = 0;

protected:
  friend class OperatorClass;

  std::vector<TypePtr> inputTypes;
  TypePtr outputType;

  bool checkNumInputsEquals(ExecutionContext& context, size_t numInputs) const;
  bool checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const;
  TypePtr getTemplateArgument(ExecutionContext& context, TypePtr type, size_t templateArgumentIndex, TypePtr requestedType = anyType) const;
  TypePtr getContainerElementsType(ExecutionContext& context, TypePtr type) const;
  TypePtr getDistributionElementsType(ExecutionContext& context, TypePtr type) const;
};

typedef ReferenceCountedObjectPtr<Operator> OperatorPtr;

class ProxyOperator : public Operator
{
protected:
  virtual OperatorPtr createImplementation(const std::vector<TypePtr>& inputTypes) const = 0;

  virtual TypePtr initializeOperator(ExecutionContext& context)
  {
    implementation = createImplementation(inputTypes);
    if (!implementation)
    {
      context.errorCallback(T("Could not create implementation in proxy operator"));
      return TypePtr();
    }
    if (!implementation->initialize(context, inputTypes))
      return TypePtr();

    return implementation->getOutputType();
  }

  virtual Variable computeOperator(const Variable* inputs) const
    {jassert(implementation); return implementation->computeOperator(inputs);}

  OperatorPtr implementation;
};

extern OperatorPtr accumulateOperator();
extern OperatorPtr discretizeOperator(bool sampleBest = true);
extern OperatorPtr segmentContainerOperator();

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_H_

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
  size_t getNumInputs() const
    {return inputTypes.size();}

  TypePtr getInputType(size_t index) const
    {jassert(index < inputTypes.size()); return inputTypes[index];}

  TypePtr getOutputType() const
    {return outputType;}

  bool initialize(ExecutionContext& context, const std::vector<TypePtr>& inputs);
  
  Variable compute(const Variable& input) const
    {jassert(inputTypes.size() == 1); return computeOperator(&input);}

  Variable compute(const Variable* inputs) const
    {return computeOperator(inputs);}

protected:
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

extern OperatorPtr accumulateOperator(TypePtr inputType);
extern OperatorPtr discretizeOperator(TypePtr inputType, bool sampleBest = true);
extern OperatorPtr segmentOperator(TypePtr inputType);

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_H_

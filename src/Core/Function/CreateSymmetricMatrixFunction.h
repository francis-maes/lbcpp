/*-----------------------------------------.---------------------------------.
| Filename: CreateSymmetricMatrixFunction.h| Create Symmetric Matrix Function|
| Author  : Julien Becker                  |                                 |
| Started : 15/02/2011 16:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_CREATE_SYMMETRIC_MATRIX_H_
# define LBCPP_CORE_FUNCTION_CREATE_SYMMETRIC_MATRIX_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/SymmetricMatrix.h>

namespace lbcpp
{

// generates a symmetric matrix M(i, j) = f(i, j, x) where i,j in [0,n[ from input n,x
class CreateSymmetricMatrixFunction : public Function
{
public:
  CreateSymmetricMatrixFunction(FunctionPtr elementGeneratorFunction = FunctionPtr())
    : elementGeneratorFunction(elementGeneratorFunction) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (index == 0) ? positiveIntegerType : elementGeneratorFunction->getRequiredInputType(index, numInputs);}

  virtual String getOutputPostFix() const
    {return T("Generated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> inputVars = inputVariables;
    inputVars.insert(inputVars.begin(), inputVars.front());
    if (!elementGeneratorFunction->initialize(context, inputVars))
      return TypePtr();
    
    VariableSignaturePtr elementsSignature = elementGeneratorFunction->getOutputVariable();
    outputName = elementsSignature->getName() + T("SymmetricMatrix");
    outputShortName = elementsSignature->getShortName() + T("sm");
    return symmetricMatrixClass(elementsSignature->getType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    size_t n = (size_t)inputs[0].getInteger();
    SymmetricMatrixPtr res = symmetricMatrix(elementGeneratorFunction->getOutputType(), n);

    std::vector<Variable> subInputs(numInputs + 1);
    for (size_t i = 2; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i - 1];

    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = Variable(i);
      for (size_t j = i; j < n; ++j)
      {
        subInputs[1] = Variable(j);
        res->setElement(i, j, elementGeneratorFunction->compute(context, subInputs));
      }
    }
    return res;
  }

protected:
  friend class CreateSymmetricMatrixFunctionClass;

  FunctionPtr elementGeneratorFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_CREATE_SYMMETRIC_MATRIX_H_
/*-----------------------------------------.---------------------------------.
| Filename: GenerateVectorFunction.h       | Generate Vector Function        |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 17:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_GENERATE_VECTOR_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_GENERATE_VECTOR_FUNCTION_H_

# include <lbcpp/Core/Vector.h>
# include <lbcpp/Function/Function.h>

namespace lbcpp
{

// generates a vector V(f(x,0); f(x,1); ... f(x,n-1)) from input x,n
class GenerateVectorFunction : public Function
{
public:
  GenerateVectorFunction(FunctionPtr elementGeneratorFunction = FunctionPtr())
    : elementGeneratorFunction(elementGeneratorFunction) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == (numInputs - 1) ? positiveIntegerType : elementGeneratorFunction->getRequiredInputType(index, numInputs);}

  virtual String getOutputPostFix() const
    {return T("Generated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!elementGeneratorFunction->initialize(context, inputVariables))
      return TypePtr();

    VariableSignaturePtr elementsSignature = elementGeneratorFunction->getOutputVariable();
    outputName = elementsSignature->getName() + T("Vector");
    outputShortName = elementsSignature->getShortName() + T("v");
    return vectorClass(elementsSignature->getType());
  }

  // at each iteration, replaces the last input (n) by the loop counter (i in [0,n[)
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 0; i < subInputs.size() - 1; ++i)
      subInputs[i] = inputs[i];

    size_t n = (size_t)inputs[numInputs - 1].getInteger();
    VectorPtr res = vector(elementGeneratorFunction->getOutputType(), n);
    for (size_t i = 0; i < n; ++i)
    {
      subInputs[numInputs - 1] = Variable(i);
      res->setElement(i, elementGeneratorFunction->computeFunction(context, &subInputs[0]));
    }
    return res;
  }

protected:
  friend class GenerateVectorFunctionClass;

  FunctionPtr elementGeneratorFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_GENERATE_VECTOR_FUNCTION_H_

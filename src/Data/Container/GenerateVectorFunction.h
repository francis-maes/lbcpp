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

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!elementGeneratorFunction->initialize(context, inputVariables))
      return VariableSignaturePtr();
    VariableSignaturePtr elementsSignature = elementGeneratorFunction->getOutputVariable();
    return new VariableSignature(vectorClass(elementsSignature->getType()), elementsSignature->getName() + T("Vector"), elementsSignature->getShortName() + T("v"));
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

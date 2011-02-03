/*-----------------------------------------.---------------------------------.
| Filename: WindowVariableGenerator.h      | Window Variable Generator       |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_VARIABLE_GENERATOR_WINDOW_H_
# define LBCPP_OPERATOR_VARIABLE_GENERATOR_WINDOW_H_

# include <lbcpp/Operator/VariableGenerator.h>

namespace lbcpp
{

class WindowVariableGenerator : public VariableGenerator
{
public:
  WindowVariableGenerator(size_t windowSize = 0)
    : windowSize(windowSize) {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 2) || !checkInputType(context, 0, containerClass(anyType)) || !checkInputType(context, 1, positiveIntegerType))
      return false;
    TypePtr elementsType = getContainerElementsType(context, getInputType(0));
    if (!elementsType)
      return false;
    DynamicClassPtr outputType = new UnnamedDynamicClass(T("Window"));
    int startPosition = - (int)(windowSize / 2);
    for (size_t i = 0; i < windowSize; ++i)
    {
      String pos = String((int)i + startPosition);
      outputType->addMemberVariable(context, elementsType, T("[") + pos + T("]"), pos);
    }
    return new VariableSignature(outputType, T("window"));
  }

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
  {
    ContainerPtr container = inputs[0].getObjectAndCast<Container>();
    int position = inputs[1].getInteger();
   
    int startPosition = position - (int)(windowSize / 2);
    
    int n = (int)container->getNumElements();
    for (size_t i = 0; i < windowSize; ++i)
    {
      int position = startPosition + (int)i;
      if (position >= 0 && position < n)
      {
        Variable variable = container->getElement(position);
        callback.sense(i, variable);
      }
    }
  }

protected:
  friend class WindowVariableGeneratorClass;

  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_VARIABLE_GENERATOR_WINDOW_H_

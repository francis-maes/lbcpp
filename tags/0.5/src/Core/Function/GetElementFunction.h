/*-----------------------------------------.---------------------------------.
| Filename: GetElementFunction.h           | Get Element Function            |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2011 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_GET_ELEMENT_H_
# define LBCPP_CORE_FUNCTION_GET_ELEMENT_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

// Container<T>, PositiveInteger -> T
class GetElementFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)containerClass(anyType);}

  virtual String getOutputPostFix() const
    {return T("Element");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return Container::getTemplateParameter(inputVariables[0]->getType());}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    if (container)
    {
      int index = inputs[1].getInteger();
      if (index >= 0 && index < (int)container->getNumElements())
        return container->getElement((size_t)index);
    }

    return Variable::missingValue(getOutputType());
  }
};

// object, position -> element
class GetElementInVariableFunction : public CompositeFunction
{
public:
  GetElementInVariableFunction(const String& variableName = String::empty)
    : variableName(variableName) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(objectClass);
    size_t position = builder.addInput(positiveIntegerType);
    size_t container = builder.addFunction(getVariableFunction(variableName), input);
    builder.addFunction(getElementFunction(), container, position);
  }

protected:
  friend class GetElementInVariableFunctionClass;

  String variableName;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_GET_ELEMENT_H_

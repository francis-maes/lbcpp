/*-----------------------------------------.---------------------------------.
| Filename: ApplyFunctionContainer.h       | Apply a function in a lazy way  |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_

# include <lbcpp/Core/Container.h>
# include <lbcpp/Function/Function.h>

namespace lbcpp
{

class ApplyFunctionContainer : public DecoratorContainer
{
public:
  ApplyFunctionContainer(ContainerPtr target, FunctionPtr function)
    : DecoratorContainer(target), function(function)
    {checkInheritance(target->getElementsType(), function->getInputType());}

  ApplyFunctionContainer() {}
    
  virtual TypePtr getElementsType() const
    {return function->getOutputType(target->getElementsType());}

  virtual Variable getElement(size_t index) const
    {return function->computeFunction(*(ExecutionContext* )0, target->getElement(index));} // FIXME: Context

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  friend class ApplyFunctionContainerClass;

  FunctionPtr function;
};

class ApplyOnContainerFunction : public Function
{
public:
  ApplyOnContainerFunction(FunctionPtr function = FunctionPtr())
    : function(function) {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!getNumInputs())
    {
      context.errorCallback(T("No inputs"));
      return false;
    }

    std::vector<VariableSignaturePtr> subInputVariables(getNumInputs());
    for (size_t i = 0; i < subInputVariables.size(); ++i)
    {
      VariableSignaturePtr inputVariable = getInputVariable(0);
      TypePtr elementsType;
      if (!Container::getTemplateParameter(context, inputVariable->getType(), elementsType))
        return false;

      subInputVariables[i] = new VariableSignature(elementsType, inputVariable->getName() + T("Element"), inputVariable->getShortName() + T("e"));
    }

    if (!function->initialize(context, subInputVariables))
      return false;

    return new VariableSignature(vectorClass(function->getOutputType()), function->getOutputVariable()->getName(), function->getOutputVariable()->getShortName());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();

    if (numInputs == 1)
    {
      ContainerPtr input = inputs[0].getObjectAndCast<Container>();
      size_t n = input->getNumElements();
      VectorPtr res = vector(function->getOutputType(), n);
      for (size_t i = 0; i < n; ++i)
      {
        Variable element = input->getElement(i);
        res->setElement(i, function->computeFunction(context, element));
      }
      return res;
    }
    else
    {
      std::vector<ContainerPtr> containers(numInputs);
      for (size_t i = 0; i < numInputs; ++i)
      {
        containers[i] = inputs[i].getObjectAndCast<Container>();
        jassert(containers[i]);
        jassert(i == 0 || containers[i]->getNumElements() == containers[0]->getNumElements());
      }

      size_t n = containers[0]->getNumElements();
      VectorPtr res = vector(function->getOutputType(), n);
      std::vector<Variable> elements(numInputs);
      for (size_t i = 0; i < n; ++i)
      {
        for (size_t j = 0; j < numInputs; ++j)
          elements[j] = containers[j]->getElement(i);
        res->setElement(i, function->computeFunction(context, &elements[0]));
      }
      return res; 
    }
  }

protected:
  friend class ApplyOnContainerFunctionClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_

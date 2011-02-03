/*-----------------------------------------.---------------------------------.
| Filename: ApplyOnContainerOperator.h     | Apply on container operator     |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 20:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_CONTAINER_APPLY_H_
# define LBCPP_OPERATOR_CONTAINER_APPLY_H_

# include <lbcpp/Operator/Operator.h>

namespace lbcpp
{

class ApplyOnContainerOperator : public Function
{
public:
  ApplyOnContainerOperator(FunctionPtr function = FunctionPtr())
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
  friend class ApplyOnContainerOperatorClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_CONTAINER_APPLY_H_

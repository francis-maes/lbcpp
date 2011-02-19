/*-----------------------------------------.---------------------------------.
| Filename: MapContainerFunction.h         | Map containers                  |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2010 15:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_MAP_CONTAINER_H_
# define LBCPP_CORE_FUNCTION_MAP_CONTAINER_H_

# include <lbcpp/Core/Container.h>
# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class MapContainerFunction : public Function
{
public:
  MapContainerFunction(FunctionPtr function)
    : function(function)
  {
    setBatchLearner(mapContainerFunctionBatchLearner());
  }
  MapContainerFunction() {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}

  virtual String getOutputPostFix() const
    {return function->getOutputPostFix();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputVariables(inputVariables.size());
    for (size_t i = 0; i < subInputVariables.size(); ++i)
    {
      const VariableSignaturePtr& inputVariable = inputVariables[i];
      TypePtr elementsType = Container::getTemplateParameter(inputVariable->getType());
      subInputVariables[i] = new VariableSignature(elementsType, inputVariable->getName() + T("Element"), inputVariable->getShortName() + T("e"));
    }
    if (!function->initialize(context, subInputVariables))
      return TypePtr();

    outputName = function->getOutputVariable()->getName() + T("Container");
    outputShortName = T("[") + function->getOutputVariable()->getShortName() + T("]");
    TypePtr outputType = function->getOutputType();
    if (outputType->inheritsFrom(objectClass))
      return objectVectorClass(outputType);
    else if (outputType->inheritsFrom(doubleType))
    {
      EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
      if (!enumeration)
        enumeration = positiveIntegerEnumerationEnumeration;
      return doubleVectorClass(enumeration, outputType);
    }
    else
      return vectorClass(outputType);
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();

    if (numInputs == 1)
    {
      ContainerPtr input = inputs[0].getObjectAndCast<Container>();
      if (!input)
        return Variable::missingValue(getOutputType());
      size_t n = input->getNumElements();
      VectorPtr res = vector(function->getOutputType(), n);
      for (size_t i = 0; i < n; ++i)
      {
        Variable element = input->getElement(i);
        res->setElement(i, function->compute(context, element));
      }
      return res;
    }
    else
    {
      std::vector<ContainerPtr> containers(numInputs);
      size_t numElements = (size_t)-1;
      for (size_t i = 0; i < numInputs; ++i)
      {
        containers[i] = inputs[i].getObjectAndCast<Container>();
        if (containers[i])
        {
          jassert(numElements == (size_t)-1 || numElements == containers[i]->getNumElements());
          numElements = containers[i]->getNumElements();
        }
      }

      VectorPtr res = vector(function->getOutputType(), numElements);
      std::vector<Variable> subInputs(numInputs);
      for (size_t i = 0; i < numElements; ++i)
      {
        for (size_t j = 0; j < numInputs; ++j)
        {
          const ContainerPtr& container = containers[j];
          if (container)
            subInputs[j] = container->getElement(i);
          else
            subInputs[j] = Variable::missingValue(function->getInputsClass()->getMemberVariableType(j));
        }
        res->setElement(i, function->compute(context, subInputs));
      }
      return res; 
    }
  }

protected:
  friend class MapContainerFunctionClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_CONTAINER_H_

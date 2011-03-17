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
# include <lbcpp/Data/SymmetricMatrix.h>

namespace lbcpp
{

// ((x1..xT)) -> (f(x1) .. f(xT))
// ((x1...xT), additional-inputs) -> (f(x1, additional-inputs), ... f(xT, additional-inputs) )
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
    {return index == 0 ? containerClass(anyType) : anyType;}

  virtual String getOutputPostFix() const
    {return function->getOutputPostFix();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputVariables(inputVariables);
    subInputVariables[0] = new VariableSignature(Container::getTemplateParameter(inputVariables[0]->getType()), T("element"));
    if (!function->initialize(context, subInputVariables))
      return TypePtr();
    
    outputName = function->getOutputVariable()->getName() + T("Container");
    outputShortName = T("[") + function->getOutputVariable()->getShortName() + T("]");
    return makeOutputType(inputVariables[0]->getType(), function->getOutputType());
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();

    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 1; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i];

    ContainerPtr input = inputs[0].getObjectAndCast<Container>();
    if (!input)
      return Variable::missingValue(getOutputType());

    size_t n = input->getNumElements();

    ContainerPtr res = createOutputContainer(input);
    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = input->getElement(i);
      res->setElement(i, function->compute(context, subInputs));
    }
    return res;
  }

  const FunctionPtr& getSubFunction() const
    {return function;}

protected:
  friend class MapContainerFunctionClass;

  FunctionPtr function;

  TypePtr makeOutputType(TypePtr inputContainerType, TypePtr outputElementType) const
  {
    bool isSymmetricMatrix = inputContainerType->inheritsFrom(symmetricMatrixClass(anyType));
    bool isMatrix = inputContainerType->inheritsFrom(matrixClass(anyType));

    if (isSymmetricMatrix)
      return symmetricMatrixClass(outputElementType);
    else if (isMatrix)
    {
      if (outputElementType->inheritsFrom(objectClass))
        return objectMatrixClass(outputElementType);
      else
        return matrixClass(outputElementType);
    }
    else if (outputElementType->inheritsFrom(objectClass))
      return objectVectorClass(outputElementType);
    else if (outputElementType->inheritsFrom(doubleType))
    {
      EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(inputContainerType);
      if (!enumeration)
        enumeration = positiveIntegerEnumerationEnumeration;
      return denseDoubleVectorClass(enumeration, outputElementType);
    }
    else
      return vectorClass(outputElementType);
  }

  ContainerPtr createOutputContainer(const ContainerPtr& input) const
  {
    TypePtr outputElementType = function->getOutputType();
    if (getOutputType()->inheritsFrom(matrixClass()))
    {
      MatrixPtr inputMatrix = input.dynamicCast<Matrix>();
      jassert(inputMatrix);
      return matrix(outputElementType, inputMatrix->getNumRows(), inputMatrix->getNumColumns());
    }
    else if (getOutputType()->inheritsFrom(symmetricMatrixClass()))
    {
      SymmetricMatrixPtr inputSymmetricMatrix = input.dynamicCast<SymmetricMatrix>();
      jassert(inputSymmetricMatrix);
      return symmetricMatrix(outputElementType, inputSymmetricMatrix->getDimension());
    }
    else
      return vector(outputElementType, input->getNumElements());
  }
};

typedef ReferenceCountedObjectPtr<MapContainerFunction> MapContainerFunctionPtr;

// ((x1..xT)) -> (f(x1) .. f(xT))
// ((x1..xT), (y1..yT)) -> (f(x1, y1) .. f(xT,yY))
class MapNContainerFunction : public MapContainerFunction
{
public:
  MapNContainerFunction(FunctionPtr function = FunctionPtr())
    : MapContainerFunction(function) {}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}

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
    return makeOutputType(inputVariables[0]->getType(), function->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();

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

    ContainerPtr res = createOutputContainer(containers[0]);
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
};


}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_CONTAINER_H_

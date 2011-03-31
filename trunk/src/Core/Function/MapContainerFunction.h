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
class MapContainerFunction : public UnaryHigherOrderFunction
{
public:
  MapContainerFunction(FunctionPtr baseFunction)
    : UnaryHigherOrderFunction(baseFunction) {}
  MapContainerFunction() {}

  /*
  ** UnaryHigherOrderFunction
  */
  virtual size_t getNumSubInputs(const ObjectPtr& inputsObject) const
  {
    ContainerPtr container = inputsObject->getVariable(0).getObjectAndCast<Container>();
    return container ? container->getNumElements() : 0;
  }
  
  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    ContainerPtr container = example->getVariable(0).getObjectAndCast<Container>();
    if (!container)
      return;

    size_t numSubInputs = example->getNumVariables();
    jassert(inputsClass->getNumMemberVariables() == numSubInputs);

    size_t n = container->getNumElements();
    for (size_t position = 0; position < n; ++position)
    {
      ObjectPtr subExample = Object::create(baseFunction->getInputsClass());
      subExample->setVariable(0, container->getElement(position));
      for (size_t input = 1; input < numSubInputs; ++input)
        subExample->setVariable(input, example->getVariable(input));
      res[index++] = subExample;
    }
  }

  /*
  ** Function
  */
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass(anyType) : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputVariables(inputVariables);
    subInputVariables[0] = new VariableSignature(Container::getTemplateParameter(inputVariables[0]->getType()), T("element"));
    if (!baseFunction->initialize(context, subInputVariables))
      return TypePtr();
    
    outputName = baseFunction->getOutputVariable()->getName() + T("Container");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return makeOutputType(inputVariables[0]->getType(), baseFunction->getOutputType());
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();

    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 1; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i];

    const ContainerPtr& input = inputs[0].getObjectAndCast<Container>();
    if (!input)
      return Variable::missingValue(getOutputType());

    ContainerPtr res = createOutputContainer(input);
    size_t n = input->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = input->getElement(i);
      res->setElement(i, baseFunction->compute(context, subInputs));
    }
    return res;
  }

protected:
  bool isMatrix;

  TypePtr makeOutputType(TypePtr inputContainerType, TypePtr outputElementType)
  {
    bool isSymmetricMatrix = inputContainerType->inheritsFrom(symmetricMatrixClass(anyType));
    isMatrix = inputContainerType->inheritsFrom(matrixClass(anyType));

    if (isSymmetricMatrix)
    {
      if (outputElementType->inheritsFrom(objectClass))
        return objectSymmetricMatrixClass(outputElementType);
      else if (outputElementType->inheritsFrom(doubleType))
        return doubleSymmetricMatrixClass(outputElementType);
      else
        return symmetricMatrixClass(outputElementType);
    }
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
    ContainerPtr res = Container::create(getOutputType());
    if (isMatrix)
    {
      const MatrixPtr& inputMatrix = input.staticCast<Matrix>();
      const MatrixPtr& resMatrix = res.staticCast<Matrix>();
      resMatrix->setSize(inputMatrix->getNumRows(), inputMatrix->getNumColumns());
    }
    else
    {
      const VectorPtr& resVector = res.staticCast<Vector>();
      resVector->resize(input->getNumElements());
    }
    return res;
  }
};

typedef ReferenceCountedObjectPtr<MapContainerFunction> MapContainerFunctionPtr;

// ((x1..xT)) -> (f(x1) .. f(xT))
// ((x1..xT), (y1..yT)) -> (f(x1, y1) .. f(xT,yT))
class MapNContainerFunction : public MapContainerFunction
{
public:
  MapNContainerFunction(FunctionPtr baseFunction = FunctionPtr())
    : MapContainerFunction(baseFunction) {}

  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    ContainerPtr container = example->getVariable(0).getObjectAndCast<Container>();
    if (!container)
      return;

    std::vector<ContainerPtr> containers;
    size_t n = getInputContainers(example, containers);
    jassert(getNumInputs() == containers.size());
    
    for (size_t position = 0; position < n; ++position)
    {
      ObjectPtr subExample = Object::create(baseFunction->getInputsClass());
      for (size_t input = 0; input < containers.size(); ++input)
        if (containers[input])
        {
          Variable element = containers[input]->getElement(position);
          subExample->setVariable(input, element);
        }

      res[index++] = subExample;
    }
  }

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
    if (!baseFunction->initialize(context, subInputVariables))
      return TypePtr();
    
    outputName = baseFunction->getOutputVariable()->getName() + T("Container");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return makeOutputType(inputVariables[0]->getType(), baseFunction->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    std::vector<ContainerPtr> containers;
    size_t numElements = getInputContainers(inputs, containers);

    ContainerPtr res = createOutputContainer(containers[0]);
    std::vector<Variable> subInputs(getNumInputs());
    for (size_t i = 0; i < numElements; ++i)
    {
      for (size_t j = 0; j < subInputs.size(); ++j)
      {
        const ContainerPtr& container = containers[j];
        if (container)
          subInputs[j] = container->getElement(i);
        else
          subInputs[j] = Variable::missingValue(baseFunction->getInputsClass()->getMemberVariableType(j));
      }
      res->setElement(i, baseFunction->compute(context, subInputs));
    }
    return res; 
  }

protected:
  size_t getInputContainers(const Variable* inputs, std::vector<ContainerPtr>& res) const
  {
    size_t numInputs = getNumInputs();
    res.resize(numInputs);
    size_t numElements = (size_t)-1;
    for (size_t i = 0; i < numInputs; ++i)
    {
      res[i] = inputs[i].getObjectAndCast<Container>();
      if (res[i])
      {
        jassert(numElements == (size_t)-1 || numElements == res[i]->getNumElements());
        numElements = res[i]->getNumElements();
      }
    }
    return numElements;
  }

  size_t getInputContainers(const ObjectPtr& inputsObject, std::vector<ContainerPtr>& res) const
  {
    size_t numInputs = getNumInputs();
    res.resize(numInputs);
    size_t numElements = (size_t)-1;
    for (size_t i = 0; i < numInputs; ++i)
    {
      res[i] = inputsObject->getVariable(i).getObjectAndCast<Container>();
      if (res[i])
      {
        jassert(numElements == (size_t)-1 || numElements == res[i]->getNumElements());
        numElements = res[i]->getNumElements();
      }
    }
    return numElements;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_CONTAINER_H_

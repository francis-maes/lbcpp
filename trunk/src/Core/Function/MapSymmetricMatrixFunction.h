/*-----------------------------------------.---------------------------------.
| Filename: MapMatrixFunction.h            | Map matrices                    |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 10:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_MAP_MATRIX_H_
# define LBCPP_CORE_FUNCTION_MAP_MATRIX_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include "MapContainerFunction.h"

namespace lbcpp
{

class MapSymmetricMatrixFunction : public UnaryHigherOrderFunction
{
public:
  MapSymmetricMatrixFunction(FunctionPtr baseFunction, size_t minimumDistanceFromDiagonal)
    : UnaryHigherOrderFunction(baseFunction), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}
  MapSymmetricMatrixFunction() {}

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

    jassert(false);
    // TODO !! see MapNContainerFunction::appendSubInputs and UnaryHigherOrderFunctionBatchLearner
  }

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return matrixClass(anyType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    for (size_t i = 0; i < inputVariables.size(); ++i)
      if (!inputVariables[i]->getType()->inheritsFrom(symmetricMatrixClass(anyType)))
        return TypePtr();

    if (!baseFunction->initialize(context, inputVariables))
      return TypePtr();

    return baseFunction->getOutputType();
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!minimumDistanceFromDiagonal)
    {
      return baseFunction->compute(context, inputs, getNumInputs());
    }
    
    size_t numInputs = getNumInputs();
    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      SymmetricMatrixPtr input = inputs[i].getObjectAndCast<SymmetricMatrix>();
      if (!input || input->getDimension() <= minimumDistanceFromDiagonal)
        subInputs[i] = Variable::missingValue(baseFunction->getInputsClass()->getMemberVariableType(i));
      else
      {
        size_t dimension = input->getDimension() - minimumDistanceFromDiagonal;
        SymmetricMatrixPtr res = symmetricMatrix(input->getElementsType(), dimension);
        for (size_t x = 0; x < dimension; ++x)
          for (size_t y = x; y < dimension; ++y)
            res->setElement(x, y, input->getElement(x, y + minimumDistanceFromDiagonal));
        subInputs[i] = res;
      }
    }

    SymmetricMatrixPtr output = baseFunction->compute(context, subInputs).getObjectAndCast<SymmetricMatrix>();
    if (!output)
      return Variable::missingValue(getOutputType());

    size_t outputDimension = output->getDimension();
    size_t dimension = outputDimension + minimumDistanceFromDiagonal;
    SymmetricMatrixPtr res = symmetricMatrix(output->getElementsType(), dimension);
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i; j < dimension; ++j)
      {
        if (i >= outputDimension || j < i + minimumDistanceFromDiagonal)
          res->setElement(i, j, Variable::missingValue(res->getElementsType()));
        else
          res->setElement(i, j, output->getElement(i, j - minimumDistanceFromDiagonal));
      }
    return res;
  }

protected:
  friend class MapSymmetricMatrixFunctionClass;

  size_t minimumDistanceFromDiagonal;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_MATRIX_H_

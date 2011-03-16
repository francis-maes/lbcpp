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

class MapSymmetricMatrixFunction : public Function
{
public:
  MapSymmetricMatrixFunction(FunctionPtr function, size_t minimumDistanceFromDiagonal)
    : function(mapContainerFunction(function)), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}
  MapSymmetricMatrixFunction() {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return matrixClass(anyType);}

  virtual String getOutputPostFix() const
    {return function->getOutputPostFix();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    for (size_t i = 0; i < inputVariables.size(); ++i)
      if (!inputVariables[i]->getType()->inheritsFrom(symmetricMatrixClass(anyType)))
        return TypePtr();

    if (!function->initialize(context, inputVariables))
      return TypePtr();

    return function->getOutputType();
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!minimumDistanceFromDiagonal)
      return function->compute(context, inputs, getNumInputs());
    
    size_t numInputs = getNumInputs();
    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      SymmetricMatrixPtr input = inputs[i].getObjectAndCast<SymmetricMatrix>();
      if (!input || input->getDimension() <= minimumDistanceFromDiagonal)
        subInputs[i] = Variable::missingValue(function->getInputsClass()->getMemberVariableType(i));
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

    SymmetricMatrixPtr output = function->compute(context, subInputs).getObjectAndCast<SymmetricMatrix>();
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

  const FunctionPtr& getSubFunction() const
    {return function;}

protected:
  friend class MapSymmetricMatrixFunctionClass;

  FunctionPtr function;
  size_t minimumDistanceFromDiagonal;
};

typedef ReferenceCountedObjectPtr<MapContainerFunction> MapContainerFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_MATRIX_H_

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

  /* UnaryHigherOrderFunction */
   virtual size_t getNumSubInputs(const ObjectPtr& inputsObject) const
  {
    SymmetricMatrixPtr matrix = inputsObject->getVariable(0).getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return 0;
    const size_t dimension = matrix->getDimension() - minimumDistanceFromDiagonal;
    return dimension * (dimension + 1) / 2;
  }

  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    SymmetricMatrixPtr matrix = example->getVariable(0).getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;

    const size_t numSubInputs = example->getNumVariables();
    jassert(inputsClass->getNumMemberVariables() == numSubInputs);

    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        ObjectPtr subExample = Object::create(baseFunction->getInputsClass());
        subExample->setVariable(0, matrix->getElement(i, j));

        for (size_t input = 1; input < numSubInputs; ++input)
          subExample->setVariable(input, example->getVariable(input));
        res[index++] = subExample;
      }
  }

  /* Function */
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)symmetricMatrixClass(anyType) : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputs(inputVariables);
    subInputs[0] = new VariableSignature(Container::getTemplateParameter(inputVariables[0]->getType()), T("element"));
    if (!baseFunction->initialize(context, subInputs))
      return TypePtr();

    outputName = baseFunction->getOutputVariable()->getName() + T("SymmetricMatrix");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return symmetricMatrixClass(baseFunction->getOutputType());
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const size_t numInputs = getNumInputs();
    const SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return Variable::missingValue(getOutputType());

    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 1; i < numInputs; ++i)
      subInputs[i] = inputs[i];
    
    const size_t dimension = matrix->getDimension();
    SymmetricMatrixPtr res = symmetricMatrix(Container::getTemplateParameter(getOutputType()), dimension);
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        subInputs[0] = matrix->getElement(i, j);
        res->setElement(i, j, baseFunction->compute(context, subInputs));
      }
    return res;
  }

protected:
  friend class MapSymmetricMatrixFunctionClass;

  size_t minimumDistanceFromDiagonal;
};

class MapNSymmetricMatrixFunction : public MapSymmetricMatrixFunction
{
public:
  MapNSymmetricMatrixFunction(FunctionPtr baseFunction, size_t minimumDistanceFromDiagonal)
    : MapSymmetricMatrixFunction(baseFunction, minimumDistanceFromDiagonal) {}
  MapNSymmetricMatrixFunction() {}

  /* UnaryHigherOrderFunction */
  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    SymmetricMatrixPtr matrix = example->getVariable(0).getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;

    const size_t dimension = matrix->getDimension();
    const size_t numInputs = getNumInputs();

    std::vector<SymmetricMatrixPtr> matrices(numInputs);
    matrices[0] = matrix;
    for (size_t i = 1; i < numInputs; ++i)
    {
      matrices[i] = example->getVariable(i).getObjectAndCast<SymmetricMatrix>();
      jassert(!matrices[i] || matrices[i]->getDimension() == dimension);
    }

    if (minimumDistanceFromDiagonal > dimension)
      return;
    
    const size_t numRows = dimension - minimumDistanceFromDiagonal;
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        ObjectPtr subExample = Object::create(baseFunction->getInputsClass());
        for (size_t input = 0; input < numInputs; ++input)
          if (matrices[input])
            subExample->setVariable(input, matrices[input]->getElement(i, j));
        res[index++] = subExample;
      }
  }
  
  /* Function */
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (TypePtr)symmetricMatrixClass(anyType);}

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

    outputName = baseFunction->getOutputVariable()->getName() + T("SymmetricMatrix");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return symmetricMatrixClass(baseFunction->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return Variable::missingValue(getOutputType());
    
    const size_t dimension = matrix->getDimension();
    const size_t numInputs = getNumInputs();

    std::vector<SymmetricMatrixPtr> matrices(numInputs);
    matrices[0] = matrix;
    for (size_t i = 1; i < numInputs; ++i)
    {
      matrices[i] = inputs[i].getObjectAndCast<SymmetricMatrix>();
      jassert(!matrices[i] || matrices[i]->getDimension() == dimension);
    }

    SymmetricMatrixPtr res = symmetricMatrix(baseFunction->getOutputType(), dimension);
    
    if (minimumDistanceFromDiagonal > dimension)
      return res;

    const size_t numRows = dimension - minimumDistanceFromDiagonal;
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
      {
        std::vector<Variable> subInputs(numInputs);
        for (size_t input = 0; input < numInputs; ++input)
          subInputs[input] = matrices[input] ? matrices[input]->getElement(i, j) : Variable::missingValue(baseFunction->getInputsClass()->getMemberVariableType(input));
        res->setElement(i, j, baseFunction->compute(context, subInputs));
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_MAP_MATRIX_H_

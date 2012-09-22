/*-----------------------------------------.---------------------------------.
| Filename: MapMatrixFunction.h            | Map matrices                    |
| Author  : Julien Becker                  |                                 |
| Started : 25/07/2011 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_MAP_MATRIX_H_
# define LBCPP_CORE_FUNCTION_MAP_MATRIX_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/Matrix.h>

namespace lbcpp
{

// Do not take into account elements on diagonal
class MapMatrixFunction : public UnaryHigherOrderFunction
{
public:
  MapMatrixFunction(FunctionPtr baseFunction)
    : UnaryHigherOrderFunction(baseFunction) {}
  MapMatrixFunction() {}

  /* UnaryHigherOrderFunction */
   virtual size_t getNumSubInputs(const ObjectPtr& inputsObject) const
  {
    MatrixPtr matrix = inputsObject->getVariable(0).getObjectAndCast<Matrix>();
    if (!matrix)
      return 0;

    const size_t numRows = matrix->getNumRows();
    const size_t numColumns = matrix->getNumColumns();

    const size_t diagonalSize = (numRows < numColumns) ? numRows : numColumns;
    return numRows * numColumns - diagonalSize;
  }

  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    MatrixPtr matrix = example->getVariable(0).getObjectAndCast<Matrix>();
    if (!matrix)
      return;

    const size_t numSubInputs = example->getNumVariables();
    jassert(inputsClass->getNumMemberVariables() == numSubInputs);

    const size_t numRows = matrix->getNumRows();
    const size_t numColumns = matrix->getNumColumns();

    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numColumns; ++j)
      {
        if (i == j)
          continue;

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
    {return index == 0 ? (TypePtr)matrixClass(anyType) : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputs(inputVariables);
    subInputs[0] = new VariableSignature(Container::getTemplateParameter(inputVariables[0]->getType()), T("element"));
    if (!baseFunction->initialize(context, subInputs))
      return TypePtr();

    outputName = baseFunction->getOutputVariable()->getName() + T("Matrix");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return matrixClass(baseFunction->getOutputType());
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const size_t numInputs = getNumInputs();
    const MatrixPtr matrix = inputs[0].getObjectAndCast<Matrix>();
    if (!matrix)
      return Variable::missingValue(getOutputType());

    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 1; i < numInputs; ++i)
      subInputs[i] = inputs[i];
    
    const size_t numRows = matrix->getNumRows();
    const size_t numColumns = matrix->getNumColumns();
    MatrixPtr res = lbcpp::matrix(Container::getTemplateParameter(getOutputType()), numRows, numColumns);

    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numColumns; ++j)
      {
        if (i == j)
        {
          res->setElement(i, j, Variable::missingValue(baseFunction->getOutputType()));
          continue;
        }

        subInputs[0] = matrix->getElement(i, j);
        res->setElement(i, j, baseFunction->compute(context, subInputs));
      }
    return res;
  }

protected:
  friend class MapMatrixFunctionClass;
};

class MapNMatrixFunction : public MapMatrixFunction
{
public:
  MapNMatrixFunction(FunctionPtr baseFunction)
    : MapMatrixFunction(baseFunction) {}
  MapNMatrixFunction() {}

  /* UnaryHigherOrderFunction */
  virtual void appendSubInputs(const ObjectPtr& example, std::vector<ObjectPtr>& res, size_t& index) const
  {
    MatrixPtr matrix = example->getVariable(0).getObjectAndCast<Matrix>();
    if (!matrix)
      return;

    const size_t numRows = matrix->getNumRows();
    const size_t numColumns = matrix->getNumColumns();
    const size_t numInputs = getNumInputs();

    std::vector<MatrixPtr> matrices(numInputs);
    matrices[0] = matrix;
    for (size_t i = 1; i < numInputs; ++i)
    {
      matrices[i] = example->getVariable(i).getObjectAndCast<Matrix>();
      jassert(!matrices[i] || (matrices[i]->getNumRows() == numRows && matrices[i]->getNumColumns() == numColumns));
    }

    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numColumns; ++j)
      {
        if (i == j)
          continue;

        ObjectPtr subExample = Object::create(baseFunction->getInputsClass());
        for (size_t input = 0; input < numInputs; ++input)
          if (matrices[input])
            subExample->setVariable(input, matrices[input]->getElement(i, j));
        res[index++] = subExample;
      }
  }
  
  /* Function */
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (TypePtr)matrixClass(anyType);}

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

    outputName = baseFunction->getOutputVariable()->getName() + T("Matrix");
    outputShortName = T("[") + baseFunction->getOutputVariable()->getShortName() + T("]");
    return matrixClass(baseFunction->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    MatrixPtr matrix = inputs[0].getObjectAndCast<Matrix>();
    if (!matrix)
      return Variable::missingValue(getOutputType());
    
    const size_t numRows = matrix->getNumRows();
    const size_t numColumns = matrix->getNumColumns();
    const size_t numInputs = getNumInputs();

    std::vector<MatrixPtr> matrices(numInputs);
    matrices[0] = matrix;
    for (size_t i = 1; i < numInputs; ++i)
    {
      matrices[i] = inputs[i].getObjectAndCast<Matrix>();
      jassert(!matrices[i] || (matrices[i]->getNumRows() == numRows && matrices[i]->getNumColumns() == numColumns));
    }

    MatrixPtr res = lbcpp::matrix(baseFunction->getOutputType(), numRows, numColumns);
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numColumns; ++j)
      {
        if (i == j)
        {
          res->setElement(i, j, Variable::missingValue(baseFunction->getOutputType()));
          continue;
        }

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

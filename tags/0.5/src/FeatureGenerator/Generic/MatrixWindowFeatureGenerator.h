/*-----------------------------------------.---------------------------------.
| Filename: MatrixWindowFeatureGenerator.h | Matrix window Feature Generator |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2011 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_MATRIX_WINDOW_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_MATRIX_WINDOW_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Data/Matrix.h>

namespace lbcpp
{

// Matrix[DoubleVector[.]], PositiveInteger, PositiveInteger -> DoubleVector[.]
class MatrixWindowFeatureGenerator : public FeatureGenerator
{
public:
  MatrixWindowFeatureGenerator(size_t windowRows = 0, size_t windowColumns = 0)
    : windowRows(windowRows), windowColumns(windowColumns)  {}

  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)matrixClass(doubleVectorClass());}
  
  virtual String getOutputPostFix() const
    {return T("Window");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr doubleVectorType = Container::getTemplateParameter(inputVariables[0]->getType());
    EnumerationPtr subFeaturesEnumeration;
    if (!DoubleVector::getTemplateParameters(context, doubleVectorType, subFeaturesEnumeration, elementsType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("MatrixWindowFeatures"));
    int startRow = - (int)(windowRows / 2);
    int startColumn = - (int)(windowColumns / 2);
    numFeaturesPerPosition = subFeaturesEnumeration->getNumElements();
    for (size_t i = 0; i < windowRows; ++i)
    {
      String row = String((int)i + startRow);
      for (size_t j = 0; j < windowColumns; ++j)
      {
        String col((int)j + startColumn);
        String pos = row + T(", ") + col;
        res->addElementsWithPrefix(context, subFeaturesEnumeration, T("[") + pos + T("]."), pos + T("."));
      }
    }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const MatrixPtr& matrix = inputs[0].getObjectAndCast<Matrix>();
    if (!matrix)
      return;
    ObjectMatrixPtr objectMatrix = matrix.dynamicCast<ObjectMatrix>();
    if (!inputs[1].exists() || !inputs[2].exists())
      return;
    int row = inputs[1].getInteger();
    int column = inputs[2].getInteger();

    int startRow = row - (int)(windowRows / 2);
    int startColumn = column - (int)(windowColumns / 2);
    int numRows = (int)matrix->getNumRows();
    int numColumns = (int)matrix->getNumColumns();
    size_t index = 0;
    for (size_t i = 0; i < windowRows; ++i)
    {
      int r = startRow + i;
      for (size_t j = 0; j < windowColumns; ++j, ++index)
      {
        int c = startColumn + j;
        if (r >= 0 && c >= 0 && r < numRows && c < numColumns)
        {
          if (objectMatrix)
          {
            // faster access on object matrices
            const DoubleVectorPtr& variable = objectMatrix->getObjectAndCast<DoubleVector>(r, c);
            if (variable)
              callback.sense(index * numFeaturesPerPosition, variable, 1.0);
          }
          else
          {
            DoubleVectorPtr variable = matrix->getElement(r, c).getObjectAndCast<DoubleVector>();
            if (variable)
              callback.sense(index * numFeaturesPerPosition, variable, 1.0);
          }
        }
      }
    }
  }

protected:
  friend class MatrixWindowFeatureGeneratorClass;

  size_t windowRows;
  size_t windowColumns;
  size_t numFeaturesPerPosition;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_WINDOW_H_

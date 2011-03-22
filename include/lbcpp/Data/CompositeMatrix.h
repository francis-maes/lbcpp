/*-----------------------------------------.---------------------------------.
| Filename: CompositeMatrix.h              | Composite Matrix                |
| Author  : Julien Becker                  |                                 |
| Started : 22/03/2011 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_COMPOSITE_MATRIX_H_
# define LBCPP_DATA_COMPOSITE_MATRIX_H_

# include "../Data/Matrix.h"
# include "SymmetricMatrix.h"

namespace lbcpp
{

class CompositeMatrix : public Matrix
{
protected:
  CompositeMatrix(TypePtr thisClass)
    : Matrix(thisClass) {}
  CompositeMatrix() {}
};

typedef ReferenceCountedObjectPtr<CompositeMatrix> CompositeMatrixPtr;

class SymmetricCompositeMatrix : public CompositeMatrix
{
public:
  SymmetricCompositeMatrix(TypePtr thisClass, SymmetricMatrixPtr lowerPart, SymmetricMatrixPtr upperPart)
    : CompositeMatrix(thisClass), lowerPart(lowerPart), upperPart(upperPart) 
  {jassert(lowerPart && upperPart && lowerPart->getDimension() == upperPart->getDimension());}
  SymmetricCompositeMatrix() {}

  /* Matrix */
  virtual size_t getNumRows() const
    {return lowerPart->getNumRows();}

  virtual size_t getNumColumns() const
    {return lowerPart->getNumColumns();}

  virtual void setSize(size_t numRows, size_t numColumns)
  {
    lowerPart->setSize(numRows, numColumns);
    upperPart->setSize(numRows, numColumns);
  }

  virtual Variable getElement(size_t row, size_t column) const
  {
    jassert(row < getNumRows());
    jassert(column < getNumColumns());
    if (row < column)
      return lowerPart->getElement(row, column);
    return upperPart->getElement(row, column);
  }

  /* Container */
  virtual void setElement(size_t row, size_t column, const Variable& value)
  {
    jassert(row < getNumRows());
    jassert(column < getNumColumns());
    if (row < column)
      lowerPart->setElement(row, column, value);
    else
      upperPart->setElement(row, column, value);
  }

  virtual Variable getElement(size_t index) const
  {
    jassert(index < getNumElements());
    const size_t numElementInUpperPart = upperPart->getNumElements();
    if (index < numElementInUpperPart)
      return upperPart->getElement(index);
    return lowerPart->getElement(index - numElementInUpperPart);
  }
  
  virtual void setElement(size_t index, const Variable& value)
  {
    jassert(index < getNumElements());
    const size_t numElementInUpperPart = upperPart->getNumElements();
    if (index < numElementInUpperPart)
      upperPart->setElement(index, value);
    else
      lowerPart->setElement(index - numElementInUpperPart, value);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SymmetricCompositeMatrixClass;

  SymmetricMatrixPtr lowerPart;
  SymmetricMatrixPtr upperPart;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_COMPOSITE_MATRIX_H_

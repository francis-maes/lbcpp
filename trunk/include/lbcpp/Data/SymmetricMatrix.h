/*-----------------------------------------.---------------------------------.
| Filename: SymmetricMatrix.h              | Symmetric Matrix                |
| Author  : Julien Becker                  |                                 |
| Started : 15/03/2011 12:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_SYMMETRIC_MATRIX_H_
# define LBCPP_DATA_SYMMETRIC_MATRIX_H_

# include "../Data/Matrix.h"

namespace lbcpp
{

class SymmetricMatrix : public Matrix
{
public:
  SymmetricMatrix(ClassPtr thisClass)
    : Matrix(thisClass) {}
  SymmetricMatrix() {}
  
  virtual size_t getDimension() const = 0;
  
  /* Matrix */
  virtual size_t getNumRows() const
    {return getDimension();}

  virtual size_t getNumColumns() const
    {return getDimension();}
  
  virtual Variable getElement(size_t row, size_t column) const = 0;
  virtual void setElement(size_t row, size_t column, const Variable& value) = 0;
  
  /* Container */
  virtual size_t getNumElements() const
  {
    const size_t n = getDimension();
    return n * (n + 1) / 2;
  }

  virtual Variable getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const Variable& value) = 0;
  
  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

extern ClassPtr symmetricMatrixClass(TypePtr type);

template<class ElementsType>
class BuiltinTypeSymmetricMatrix : public SymmetricMatrix
{
public:
  BuiltinTypeSymmetricMatrix(TypePtr elementsType, size_t dimension, const ElementsType& initialValue)
    : elementsType(elementsType), dimension(dimension), elements(dimension * (dimension + 1) / 2, initialValue) {}
  BuiltinTypeSymmetricMatrix() : dimension(0) {}
  
  void setElement(size_t row, size_t column, const ElementsType& value)
    {setElement(makeIndex(row, column), value);}
  
  void setElement(size_t index, const ElementsType& value)
    {jassert(index < elements.size()); elements[index] = value;}
  
  /* SymmetricMatrix */
  size_t getDimension() const
    {return dimension;}
  
  /* Matrix */
  virtual Variable getElement(size_t row, size_t column) const
    {return getElement(makeIndex(row, column));}
  
  /* Container */
  virtual TypePtr getElementsType() const
    {return elementsType;}
  
  virtual Variable getElement(size_t index) const
    {jassert(index < elements.size()); return Variable(elements[index], elementsType);}

protected:
  TypePtr elementsType;
  size_t dimension;
  std::vector<ElementsType> elements;
  
  size_t makeIndex(size_t row, size_t column) const
  {
    jassert(row < dimension && column < dimension);
    if (row > column)
    {
      size_t swap = row;
      row = column;
      column = swap;
    }

    return (column - row) + (row * dimension) - ((row * (row - 1)) / 2);
  }
};

class DoubleSymmetricMatrix : public BuiltinTypeSymmetricMatrix<double>
{
public:
  DoubleSymmetricMatrix(TypePtr elementsType, size_t dimension, double defaultValue)
    : BuiltinTypeSymmetricMatrix<double>(elementsType, dimension, defaultValue) {}
  DoubleSymmetricMatrix() {}
  
  /* Matrix */
  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeSymmetricMatrix<double>::setElement(row, column, value.getDouble());}
  
  /* Container */
  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeSymmetricMatrix<double>::setElement(index, value.getDouble());}
};

class ObjectSymmetricMatrix : public BuiltinTypeSymmetricMatrix<ObjectPtr>
{
public:
  ObjectSymmetricMatrix(TypePtr elementsType, size_t dimension, ObjectPtr defaultValue)
    : BuiltinTypeSymmetricMatrix<ObjectPtr>(elementsType, dimension, defaultValue) {}
  ObjectSymmetricMatrix() {}
  
  /* Matrix */
  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeSymmetricMatrix<ObjectPtr>::setElement(row, column, value.getObject());}
  
  /* Container */
  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeSymmetricMatrix<ObjectPtr>::setElement(index, value.getObject());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_SYMMETRIC_MATRIX_H_

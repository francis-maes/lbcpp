/*-----------------------------------------.---------------------------------.
| Filename: Matrix.h                       | Matrix class                    |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2011 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_MATRIX_H_
# define LBCPP_DATA_MATRIX_H_

# include "../Core/Container.h"

namespace lbcpp
{

class Matrix : public Container
{
public:
  Matrix(ClassPtr thisClass)
    : Container(thisClass) {}
  Matrix() {}
  
  virtual size_t getNumRows() const = 0;
  virtual size_t getNumColumns() const = 0;

  virtual size_t getNumElements() const
    {return getNumRows() * getNumColumns();}

  virtual Variable getElement(size_t row, size_t column) const = 0;
  virtual void setElement(size_t row, size_t column, const Variable& value) = 0;

  virtual Variable getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const Variable& value) = 0;

  /* Object */
  String toString() const
  {
    String res;
    for (size_t i = 0; i < getNumRows(); ++i)
    {
      for (size_t j = 0; j < getNumColumns(); ++j)
        res += getElement(i, j).toString() + T(" ");
      res += T("\n");
    }
    return res;
  }
  
  String toShortString() const
    {return String((int)getNumRows()) + T(" x ") + String((int)getNumColumns()) + T(" matrix");}

protected:
  size_t makeIndex(size_t row, size_t column) const
    {return row * getNumColumns() + column;}
};

typedef ReferenceCountedObjectPtr<Matrix> MatrixPtr;

extern ClassPtr matrixClass(TypePtr type = anyType);

template<class ElementsType>
class BuiltinTypeMatrix : public Matrix
{
public:
  BuiltinTypeMatrix(TypePtr thisClass, size_t numRows, size_t numColumns, const ElementsType& initialValue)
    : Matrix(thisClass), elementsType(thisClass->getTemplateArgument(0)), elements(numRows * numColumns, initialValue), numRows(numRows), numColumns(numColumns) {}
  BuiltinTypeMatrix() : numRows(0), numColumns(0) {}

  virtual size_t getNumRows() const
    {return numRows;}

  virtual size_t getNumColumns() const
    {return numColumns;}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual Variable getElement(size_t index) const
    {jassert(index < elements.size()); return Variable(elements[index], elementsType);}

  virtual Variable getElement(size_t row, size_t column) const
    {return getElement(makeIndex(row, column));}

  void setElement(size_t row, size_t column, const ElementsType& value)
    {setElement(makeIndex(row, column), value);}

  void setElement(size_t index, const ElementsType& value)
    {jassert(index < elements.size()); elements[index] = value;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    BuiltinTypeMatrix<ElementsType>& target = *t.staticCast< BuiltinTypeMatrix<ElementsType> >();
    target.elementsType = elementsType;
    target.elements = elements;
    target.numRows = numRows;
    target.numColumns = numColumns;
  }

protected:
  TypePtr elementsType;
  std::vector<ElementsType> elements;
  size_t numRows, numColumns;
};

extern ClassPtr shortEnumerationMatrixClass(TypePtr elementsType);

class ShortEnumerationMatrix : public BuiltinTypeMatrix<char>
{
public:
  ShortEnumerationMatrix(EnumerationPtr enumeration, size_t numRows, size_t numColumns, char defaultValue)
    : BuiltinTypeMatrix<char>(shortEnumerationMatrixClass(enumeration), numRows, numColumns, defaultValue)
    {jassert(enumeration->getNumElements() < 255);}

  ShortEnumerationMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeMatrix<char>::setElement(row, column, value.getInteger());}

  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeMatrix<char>::setElement(index, value.getInteger());}
};

extern ClassPtr objectMatrixClass(TypePtr elementsType);

class ObjectMatrix : public BuiltinTypeMatrix<ObjectPtr>
{
public:
  ObjectMatrix(ClassPtr elementsClass, size_t numRows, size_t numColumns)
    : BuiltinTypeMatrix<ObjectPtr>(objectMatrixClass(elementsClass), numRows, numColumns, ObjectPtr()) {}
  ObjectMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeMatrix<ObjectPtr>::setElement(row, column, value.getObject());}

  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeMatrix<ObjectPtr>::setElement(index, value.getObject());}
};

inline MatrixPtr matrix(TypePtr elementsType, size_t numRows, size_t numColumns)
{
  EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();

  if (enumeration && enumeration->getNumElements() < 255)
    return new ShortEnumerationMatrix(enumeration, numRows, numColumns, (char)enumeration->getMissingValue().getInteger());
  else if (elementsType->inheritsFrom(objectClass))
    return new ObjectMatrix(elementsType, numRows, numColumns);
  else
  {
    jassert(false);
    return MatrixPtr();
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_MATRIX_H_

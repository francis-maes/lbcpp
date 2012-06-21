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
  
  virtual void setDimension(size_t size) = 0;
  virtual size_t getDimension() const = 0;
  virtual MatrixPtr toMatrix() const
    {jassertfalse; return MatrixPtr();}
  
  /* Matrix */
  virtual size_t getNumRows() const
    {return getDimension();}

  virtual size_t getNumColumns() const
    {return getDimension();}

  virtual void setSize(size_t numRows, size_t numColumns)
    {jassert(numRows == numColumns); setDimension(numRows);}

  /* Container */
  virtual size_t getNumElements() const
  {
    const size_t n = getDimension();
    return n * (n + 1) / 2;
  }
  
  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

extern ClassPtr symmetricMatrixClass(TypePtr type = anyType);

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
  virtual void setDimension(size_t size)
  {
    dimension = size;
    elements.clear();
    elements.resize(dimension * (dimension + 1) / 2);
    if (!elementsType)
      elementsType = Container::getTemplateParameter(getClass());
  }

  virtual size_t getDimension() const
    {return dimension;}
  
  /* Matrix */
  virtual Variable getElement(size_t row, size_t column) const
    {return getElement(makeIndex(row, column));}
  
  /* Container */
  virtual TypePtr getElementsType() const
    {return elementsType;}
  
  virtual Variable getElement(size_t index) const
    {jassert(index < elements.size()); return Variable(elements[index], elementsType);}

  /* Object */
  void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Object::clone(context, target);
    ReferenceCountedObjectPtr<BuiltinTypeSymmetricMatrix> targetMatrix = target.staticCast<BuiltinTypeSymmetricMatrix>();
    targetMatrix->dimension = dimension;
    targetMatrix->elementsType = elementsType;
    const size_t n = getNumElements();
    targetMatrix->elements.resize(n);
    for (size_t i = 0; i < n; ++i)
      targetMatrix->elements[i] = elements[i];
  }
  
  lbcpp_UseDebuggingNewOperator

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

extern ClassPtr doubleSymmetricMatrixClass(TypePtr type);

class DoubleSymmetricMatrix : public BuiltinTypeSymmetricMatrix<double>
{
public:
  DoubleSymmetricMatrix(TypePtr elementsType, size_t dimension, double defaultValue)
    : BuiltinTypeSymmetricMatrix<double>(elementsType, dimension, defaultValue)
    {setThisClass(doubleSymmetricMatrixClass(elementsType));}
  DoubleSymmetricMatrix() {}

  virtual MatrixPtr toMatrix() const
  {
    DoubleMatrixPtr res = new DoubleMatrix(doubleMatrixClass(getElementsType()), getDimension(), getDimension());
    
    for (size_t i = 0; i < getDimension(); ++i)
      for (size_t j = i; j < getDimension(); ++j)
      {
        const double value = getValue(i, j);
        res->setElement(i, j, value);
        res->setElement(j, i, value);
      }

    return res;
  }

  /* Matrix */
  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeSymmetricMatrix<double>::setElement(row, column, value.getDouble());}

  /* Container */
  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeSymmetricMatrix<double>::setElement(index, value.getDouble());}

  void setValue(size_t row, size_t column, double value)
    {setElement(makeIndex(row, column), value);}

  double getValue(size_t row, size_t column) const
    {return elements[makeIndex(row, column)];}
 
  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<DoubleSymmetricMatrix> DoubleSymmetricMatrixPtr;

extern ClassPtr charSymmetricMatrixClass(TypePtr type);

class CharSymmetricMatrix : public BuiltinTypeSymmetricMatrix<char>
{
public:
  CharSymmetricMatrix(TypePtr elementsType, size_t dimension, char defaultValue)
    : BuiltinTypeSymmetricMatrix<char>(elementsType, dimension, defaultValue)
    {setThisClass(charSymmetricMatrixClass(elementsType));}
  CharSymmetricMatrix() {}

  /* Matrix */
  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeSymmetricMatrix<char>::setElement(row, column, (char)value.getInteger());}

  /* Container */
  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeSymmetricMatrix<char>::setElement(index, (char)value.getInteger());}

  void setValue(size_t row, size_t column, char value)
    {setElement(makeIndex(row, column), value);}

  char getValue(size_t row, size_t column) const
    {return elements[makeIndex(row, column)];}
 
  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<CharSymmetricMatrix> CharSymmetricMatrixPtr;

extern ClassPtr objectSymmetricMatrixClass(TypePtr type);

class ObjectSymmetricMatrix : public BuiltinTypeSymmetricMatrix<ObjectPtr>
{
public:
  ObjectSymmetricMatrix(TypePtr elementsType, size_t dimension, ObjectPtr defaultValue)
    : BuiltinTypeSymmetricMatrix<ObjectPtr>(elementsType, dimension, defaultValue)
    {setThisClass(objectSymmetricMatrixClass(elementsType));}
  ObjectSymmetricMatrix() {}

  /* Matrix */
  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeSymmetricMatrix<ObjectPtr>::setElement(row, column, value.getObject());}

  /* Container */
  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeSymmetricMatrix<ObjectPtr>::setElement(index, value.getObject());}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<ObjectSymmetricMatrix> ObjectSymmetricMatrixPtr;
/*
extern ClassPtr subsetSymmetricMatrixClass;

class SubsetSymmetricMatrix : public SymmetricMatrix
{
public:
  SubsetSymmetricMatrix(const SymmetricMatrixPtr& decorated, const std::vector<size_t>& subsetIndices)
    : decorated(decorated), indices(subsetIndices)
  {
    setThisClass(subsetSymmetricMatrixClass);
    jassert(decorated);
    sort(indices.begin(), indices.end());
    jassert(indices.back() < decorated->getDimension());
  }

  virtual void setDimension(size_t size)
    {jassertfalse;}

  virtual size_t getDimension() const
    {return indices.size();}

  virtual Variable getElement(size_t row, size_t column) const
  {
    jassert(row < indices.size() && column < indices.size());
    return decorated->getElement(indices[row], indices[column]);
  }

  virtual void setElement(size_t row, size_t column, const Variable& value)
  {
    jassert(row < indices.size() && column < indices.size());
    return decorated->setElement(indices[row], indices[column], value);
  }

  virtual Variable getElement(size_t index) const
  {
    size_t currentIndex = 0;
    for (size_t i = 0; i < indices.size(); ++i)
      for (size_t j = i; j < indices.size(); ++j)
      {
        if (currentIndex == index)
          return getElement(indices[i], indices[j]);
        ++index;
      }

    jassertfalse;
    return Variable();
  }

  virtual void setElement(size_t index, const Variable& value)
  {
    size_t currentIndex = 0;
    for (size_t i = 0; i < indices.size(); ++i)
      for (size_t j = i; j < indices.size(); ++j)
      {
        if (currentIndex == index)
          return setElement(indices[i], indices[j], value);
        ++index;
      }
    jassertfalse;
  }

protected:
  friend class SubsetSymmetricMatrixClass;

  SymmetricMatrixPtr decorated;
  std::vector<size_t> indices;

  SubsetSymmetricMatrix() {}
};

typedef ReferenceCountedObjectPtr<SubsetSymmetricMatrix> SubsetSymmetricMatrixPtr;
*/

/*
 ** Symmetric Matrix Constructor Method
 */
extern SymmetricMatrixPtr symmetricMatrix(TypePtr elementsType, size_t dimension);
extern SymmetricMatrixPtr zeroSymmetricMatrix(size_t dimension);
extern MatrixPtr upperLowerSquareMatrix(TypePtr elementsClass, SymmetricMatrixPtr lowerMatrix, SymmetricMatrixPtr upperMatrix);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_SYMMETRIC_MATRIX_H_

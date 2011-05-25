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
# include "../Core/Pair.h"
# include "DoubleVector.h"

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
  virtual void setSize(size_t numRows, size_t numColumns) = 0;

  virtual size_t getNumElements() const
    {return getNumRows() * getNumColumns();}

  virtual Variable getElement(size_t row, size_t column) const = 0;
  virtual void setElement(size_t row, size_t column, const Variable& value) = 0;

  /* Container */
  virtual Variable getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const Variable& value) = 0;

  /* Object */
  virtual String toString() const;
  virtual String toShortString() const;
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

  lbcpp_UseDebuggingNewOperator

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
    : Matrix(thisClass), elementsType(Container::getTemplateParameter(thisClass)), elements(numRows * numColumns, initialValue), numRows(numRows), numColumns(numColumns) {}
  BuiltinTypeMatrix() : numRows(0), numColumns(0) {}

  virtual size_t getNumRows() const
    {return numRows;}

  virtual size_t getNumColumns() const
    {return numColumns;}

  virtual void setSize(size_t numRows, size_t numColumns)
  {
    elements.clear();
    elements.resize(numRows * numColumns);
    this->numRows = numRows;
    this->numColumns = numColumns;
    if (!elementsType)
      elementsType = Container::getTemplateParameter(getClass());
  }

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual Variable getElement(size_t index) const
    {jassert(index < elements.size()); return Variable(elements[index], elementsType);}

  virtual Variable getElement(size_t row, size_t column) const
  {
    size_t index = makeIndex(row, column);
    jassert(index < elements.size());
    return Variable(elements[index], elementsType);
  }

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

  lbcpp_UseDebuggingNewOperator

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

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr objectMatrixClass(TypePtr elementsType);

class ObjectMatrix : public BuiltinTypeMatrix<ObjectPtr>
{
public:
  typedef BuiltinTypeMatrix<ObjectPtr> BaseClass;

  ObjectMatrix(ClassPtr elementsClass, size_t numRows, size_t numColumns)
    : BaseClass(objectMatrixClass(elementsClass), numRows, numColumns, ObjectPtr()) {}
  ObjectMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BaseClass::setElement(row, column, value.getObject());}

  virtual void setElement(size_t index, const Variable& value)
    {BaseClass::setElement(index, value.getObject());}

  const ObjectPtr& getObject(size_t row, size_t column) const
    {return BaseClass::elements[makeIndex(row, column)];}

  template<class Type>
  const ReferenceCountedObjectPtr<Type>& getObjectAndCast(size_t row, size_t column) const
  {
    const ObjectPtr& object = getObject(row, column);
    return object.staticCast<Type>();
  }
  
  void setObject(size_t row, size_t column, const ObjectPtr& object)
    {BaseClass::elements[makeIndex(row, column)] = object;}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<ObjectMatrix> ObjectMatrixPtr;

extern ClassPtr doubleMatrixClass(TypePtr elementsType = doubleType);

class DoubleMatrix;
typedef ReferenceCountedObjectPtr<DoubleMatrix> DoubleMatrixPtr;

class DoubleMatrix : public BuiltinTypeMatrix<double>
{
public:
  typedef BuiltinTypeMatrix<double> BaseClass;

  DoubleMatrix(TypePtr elementsType, size_t numRows, size_t numColumns, double initialValue = 0.0)
    : BaseClass(doubleMatrixClass(elementsType), numRows, numColumns, initialValue) {}
  DoubleMatrix(size_t numRows, size_t numColumns, double initialValue = 0.0)
    : BaseClass(doubleMatrixClass(), numRows, numColumns, initialValue) {}
  DoubleMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BaseClass::setElement(row, column, value.getDouble());}

  virtual void setElement(size_t index, const Variable& value)
    {BaseClass::setElement(index, value.getDouble());}

  void setValue(size_t row, size_t column, double value)
    {BaseClass::setElement(row, column, value);}

  double getValue(size_t row, size_t column) const
    {size_t index = makeIndex(row, column); jassert(index < elements.size()); return elements[index];}

  DoubleMatrixPtr multiplyBy(const DoubleMatrixPtr& factor) const;
  DenseDoubleVectorPtr multiplyVector(const DenseDoubleVectorPtr& vector) const;
  DoubleMatrixPtr transpose();

  void add(const DoubleMatrixPtr& term);
  void subtract(const DoubleMatrixPtr& term);
  void multiplyByScalar(double factor);

  DoubleMatrixPtr choleskyDecomposition() const;

  void inverse();
  DoubleMatrixPtr getInverse() const;

  double determinant() const;
  double inducedL1Norm() const;

  void getExtremumValues(double& minValue, double& maxValue);

  lbcpp_UseDebuggingNewOperator
};

extern MatrixPtr matrix(TypePtr elementsType, size_t numRows, size_t numColumns);


extern ClassPtr matrixRegionClass(TypePtr elementType);

class MatrixRegion : public Object
{
public:
  MatrixRegion(ClassPtr thisClass, size_t index);
  MatrixRegion() : index(0), size(0) {}

  typedef std::pair<size_t, size_t> Position;
  typedef std::set<Position> PositionSet;

  size_t getIndex() const
    {return index;}

  void setValue(const Variable& value)
    {this->value = value;}

  const Variable& getValue() const
    {return value;}

  void addPosition(const Position& position);
  void addNeighbor(const Position& position, size_t count = 1);

  const PositionSet& getNeighborPositions() const;

  void setMatrix(const MatrixPtr& matrix)
    {this->matrix = matrix;}

  const MatrixPtr& getMatrix() const
    {return matrix;}

  lbcpp_UseDebuggingNewOperator

private:
  friend class MatrixRegionClass;

  size_t index;
  Variable value;
  size_t size;
  PositionSet positions;
  PositionSet neighborPositions;
  MatrixPtr matrix;
};

typedef ReferenceCountedObjectPtr<MatrixRegion> MatrixRegionPtr;

class SegmentedMatrix : public BuiltinTypeMatrix<size_t>
{
public:
  typedef BuiltinTypeMatrix<size_t> BaseClass;

  SegmentedMatrix(TypePtr elementsType, size_t numRows, size_t numColumns);
  SegmentedMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BaseClass::setElement(row, column, (size_t)value.getInteger());}

  virtual void setElement(size_t index, const Variable& value)
    {BaseClass::setElement(index, (size_t)value.getInteger());}

  bool hasElement(size_t row, size_t column) const
    {return elements[makeIndex(row, column)] != (size_t)-1;}

  bool hasElement(const std::pair<size_t, size_t>& position) const
    {return elements[makeIndex(position.first, position.second)] != (size_t)-1;}

  MatrixRegionPtr startRegion(const Variable& value);
  void addToCurrentRegion(const std::pair<size_t, size_t>& position);

  const MatrixPtr& getSourceMatrix() const
    {return sourceMatrix;}

  void setSourceMatrix(const MatrixPtr& matrix)
    {sourceMatrix = matrix;}

  lbcpp_UseDebuggingNewOperator

private:
  friend class SegmentedMatrixClass;

  TypePtr elementsType; // the input elementsType  /!\ this is different from BaseClass::elementsType whose value is "PositiveInteger"
  ClassPtr regionClass; // matrixRegion(elementsType)
  std::vector<MatrixRegionPtr> regions;
  MatrixPtr sourceMatrix;
};

typedef ReferenceCountedObjectPtr<SegmentedMatrix> SegmentedMatrixPtr;

extern ClassPtr segmentedMatrixClass(TypePtr elementsType);
extern FunctionPtr segmentMatrixFunction(bool use8Connexity);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_MATRIX_H_

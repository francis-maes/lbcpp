/*-----------------------------------------.---------------------------------.
| Filename: Matrix.cpp                     | Matrix class                    |
| Author  : Becker Julien                  |                                 |
| Started : 22/03/2011 21:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Data/Matrix.h>
#include <lbcpp/Data/SymmetricMatrix.h>
using namespace lbcpp;

/*
** Matrix
*/
String Matrix::toString() const
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
  
String Matrix::toShortString() const
  {return String((int)getNumRows()) + T(" x ") + String((int)getNumColumns()) + T(" matrix");}

void Matrix::saveToXml(XmlExporter& exporter) const
{
  Container::saveToXml(exporter);
  exporter.setAttribute(T("numRows"), getNumRows());
  exporter.setAttribute(T("numColumns"), getNumColumns());
}

bool Matrix::loadFromXml(XmlImporter& importer)
{
  size_t numRows = (size_t)importer.getIntAttribute(T("numRows"));
  size_t numColumns = (size_t)importer.getIntAttribute(T("numColumns"));
  setSize(numRows, numColumns);
  return Container::loadFromXml(importer);
}

/*
** MatrixRegion
*/
MatrixRegion::MatrixRegion(ClassPtr thisClass, size_t index)
  : Object(thisClass), index(index), size(0)
{
}

void MatrixRegion::addPosition(const std::pair<size_t, size_t>& position)
{
  positions.insert(position);
  ++size;
}

void MatrixRegion::addNeighbor(const Position& position, size_t count)
  {neighborPositions.insert(position);}

const MatrixRegion::PositionSet& MatrixRegion::getNeighborPositions() const
  {return neighborPositions;}

/*
** SegmentedMatrix
*/
SegmentedMatrix::SegmentedMatrix(TypePtr elementsType, size_t numRows, size_t numColumns)
  : BaseClass(segmentedMatrixClass(elementsType), numRows, numColumns, (size_t)-1), elementsType(elementsType)
{
  BaseClass::elementsType = positiveIntegerType;
  regionClass = matrixRegionClass(elementsType);
}

MatrixRegionPtr SegmentedMatrix::startRegion(const Variable& value)
{
  size_t index = regions.size();
  MatrixRegionPtr res = new MatrixRegion(regionClass, index);
  res->setValue(value);
  res->setMatrix(sourceMatrix);
  regions.push_back(res);
  return res;
}

void SegmentedMatrix::addToCurrentRegion(const std::pair<size_t, size_t>& position)
{
  jassert(regions.size());
  MatrixRegionPtr region = regions.back();
  region->addPosition(position);
  BaseClass::setElement(position.first, position.second, region->getIndex());
}

DoubleMatrixPtr DoubleMatrix::multiplyBy(const DoubleMatrixPtr& factor) const
{
  jassert(this->getNumColumns() == factor->getNumRows());
  DoubleMatrixPtr product = new DoubleMatrix(this->getNumRows(), factor->getNumColumns(), 0.0);
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < factor->getNumColumns(); j++)
    {
      double value = 0.0;
      for (size_t k = 0; k < this->getNumColumns(); k++)
        value += this->getValue(i, k) * factor->getValue(k, j);
      product->setValue(i, j, value);
    }
  return product;
}

DenseDoubleVectorPtr DoubleMatrix::multiplyVector(const DenseDoubleVectorPtr& vector) const
{
  jassert(this->getNumColumns() == vector->getNumValues());
  DenseDoubleVectorPtr result = new DenseDoubleVector(this->getNumRows(), 0.0);
  for (size_t i = 0; i < this->getNumRows(); i++)
  {
    double value = 0.0;
    for (size_t j = 0; j < this->getNumColumns(); j++) 
      value += this->getValue(i,j)*vector->getValue(j);
    result->setValue(i, value);
  }
  return result;
}

DoubleMatrixPtr DoubleMatrix::transpose()
{
  DoubleMatrixPtr transposed = new DoubleMatrix(this->getNumColumns(), this->getNumRows(), 0.0);
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < this->getNumColumns(); j++)
    {
      transposed->setValue(j, i, this->getValue(i, j));
    }
  return transposed;
}

void DoubleMatrix::add(const DoubleMatrixPtr& term)
{
  jassert((this->getNumRows() == term->getNumRows()) && (this->getNumColumns() == term->getNumColumns()));
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < this->getNumColumns(); j++)
      this->setValue(i, j, this->getValue(i, j) + term->getValue(i, j));
  return;
}

void DoubleMatrix::subtract(const DoubleMatrixPtr& term)
{
  jassert((this->getNumRows() == term->getNumRows()) && (this->getNumColumns() == term->getNumColumns()));
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < this->getNumColumns(); j++)
      this->setValue(i, j, this->getValue(i, j) - term->getValue(i, j));
  return;
}

void DoubleMatrix::multiplyByScalar(double factor)
{
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < this->getNumColumns(); j++)
      this->setValue(i, j, factor * this->getValue(i, j));
  return;
}

DoubleMatrixPtr DoubleMatrix::choleskyDecomposition() const
{
  // matrix must be symmetric... and semi definite positive
  // until now, we suppose it is
  jassert(this->getNumColumns() == this->getNumRows());
  DoubleMatrixPtr temp = this->cloneAndCast<DoubleMatrix>();

  // Decomposition
  temp->setValue(0, 0, std::sqrt(temp->getValue(0, 0)));
  size_t N = temp->getNumColumns();
  for (size_t i = 1; i < N; i++)
    temp->setValue(i, 0, temp->getValue(i, 0) / temp->getValue(0, 0));

  for (size_t k = 1; k < (N - 1); k++)
  {
    double sum1 = 0;

    for (size_t j = 0; j < k; j++)
      sum1 += std::pow(temp->getValue(k, j), 2.0);
    temp->setValue(k, k, std::sqrt(std::abs(temp->getValue(k, k) - sum1)));

    for (size_t i = k + 1; i < N; i++)
    {
      double sum3 = 0;
      for (size_t j = 0; j < k; j++)
        sum3 += temp->getValue(i, j) * temp->getValue(k, j);
      temp->setValue(i, k, (1.0 / temp->getValue(k, k)) * (temp->getValue(i, k) - sum3));
    }
  }

  double sum2 = 0;
  for (size_t j = 0; j < (N - 1); j++)
    sum2 += std::pow(temp->getValue(N - 1, j), 2.0);
  temp->setValue(N - 1, N - 1, std::sqrt(std::abs(temp->getValue(N - 1, N - 1) - sum2)));

  // Set the upper part to 0
  for (size_t i = 0; i < N; i++)
    for (size_t j = i + 1; j < N; j++)
      temp->setValue(i, j, 0.0);

  return temp;
}

void DoubleMatrix::inverse()
{
  // only implemented for 2x2 matrices
  jassert((this->getNumColumns() == 2) && (this->getNumRows() == 2));
  DoubleMatrixPtr temp = this->cloneAndCast<DoubleMatrix> ();
  this->setValue(0, 0, temp->getValue(1, 1));
  this->setValue(0, 1, temp->getValue(0, 1) * (-1.0));
  this->setValue(1, 0, temp->getValue(1, 0) * (-1.0));
  this->setValue(1, 1, temp->getValue(0, 0));

  double invD = 1.0 / this->determinant();
  for (size_t i = 0; i < this->getNumRows(); i++)
    for (size_t j = 0; j < this->getNumColumns(); j++)
      this->setValue(i, j, invD * this->getValue(i, j));
}

DoubleMatrixPtr DoubleMatrix::getInverse() const
{
  DoubleMatrixPtr temp = this->cloneAndCast<DoubleMatrix> ();
  temp->inverse();
  return temp;
}

double DoubleMatrix::inducedL1Norm() const
{
  double norm = 0;
  for (size_t j = 0; j < this->getNumColumns(); j++)
  {
    double sum = 0;
    for (size_t i = 0; i < this->getNumRows(); i++)
      sum += std::abs(this->getValue(i, j));
    if (sum > norm)
      norm = sum;
  }
  return norm;
}

double DoubleMatrix::determinant() const
{
  // only implemented for 2x2 matrices
  jassert((this->getNumColumns() == 2) && (this->getNumRows() == 2));
  return this->getValue(0, 0) * this->getValue(1, 1) - this->getValue(0, 1) * this->getValue(1, 0);
}

void DoubleMatrix::getExtremumValues(double& minValue, double& maxValue)
{
  minValue = DBL_MAX;
  maxValue = -DBL_MAX;
  for (size_t i = 0; i < elements.size(); ++i)
  {
    double v = elements[i];
    if (v < minValue)
      minValue = v;
    if (v > maxValue)
      maxValue = v;
  }
}

/*
** Matrix Constructor Method
*/
namespace lbcpp
{

MatrixPtr matrix(TypePtr elementsType, size_t numRows, size_t numColumns)
{
  EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
  
  if (enumeration && enumeration->getNumElements() < 255)
    return new ShortEnumerationMatrix(enumeration, numRows, numColumns, (char)enumeration->getMissingValue().getInteger());
  else if (elementsType->inheritsFrom(objectClass))
    return new ObjectMatrix(elementsType, numRows, numColumns);
  else if (elementsType->inheritsFrom(doubleType))
    return new DoubleMatrix(elementsType, numRows, numColumns);
  else
  {
    jassert(false);
    return MatrixPtr();
  }
}

SymmetricMatrixPtr symmetricMatrix(TypePtr elementsType, size_t dimension)
{
  jassert(elementsType);
  if (elementsType->inheritsFrom(doubleType))
    return new DoubleSymmetricMatrix(elementsType, dimension, Variable::missingValue(elementsType).getDouble());
  else if (elementsType->inheritsFrom(objectClass))
    return new ObjectSymmetricMatrix(elementsType, dimension, ObjectPtr());
  else
    jassertfalse;
  return SymmetricMatrixPtr();
}

SymmetricMatrixPtr zeroSymmetricMatrix(size_t dimension)
  {return symmetricMatrix(doubleType, dimension);}

};

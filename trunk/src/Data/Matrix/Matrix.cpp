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

/* Matrix */
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

/* Matrix Constructor Method */
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

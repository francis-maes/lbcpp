/*-----------------------------------------.---------------------------------.
| Filename: SymmetricMatrix.cpp            | Symmetric Matrix of variables   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/SymmetricMatrix.h>
using namespace lbcpp;

SymmetricMatrix::SymmetricMatrix(TypePtr contentType, size_t dimension) 
  : Container(symmetricMatrixClass(contentType)), dimension(dimension), values(contentType, (dimension * (dimension + 1)) / 2)
{
}

size_t SymmetricMatrix::getIndex(size_t i, size_t j) const
{
  jassert(i < dimension);
  jassert(j < dimension);
  if (i > j)
    {size_t tmp = i; i = j; j = tmp;}
  size_t res = (j - i) + (i * dimension) - ((i * (i - 1)) / 2);
  jassert(res < values.getNumElements());
  return res;
}

String SymmetricMatrix::toString() const
{
  String res;
  for (size_t i = 0; i < dimension; ++i)
  {
    for (size_t j = 0; j < dimension; ++j)
      res += getElement(i, j).toString() + T(" ");
    res += T("\n");
  }
  return res;
}

String SymmetricMatrix::getShortSummary() const
{
  String dim((int)dimension);
  return dim + T(" x ") + dim + T(" symmetric matrix");
}

void SymmetricMatrix::saveToXml(XmlElement* xml) const
  {xml->setAttribute(T("dimension"), (int)dimension); values.saveToXml(xml);}

bool SymmetricMatrix::loadFromXml(XmlElement* xml, MessageCallback& callback)
{
  jassert(thisClass);
  if (!xml->hasAttribute(T("dimension")))
  {
    callback.errorMessage(T("SymmetricMatrix::loadFromXml"), T("Missing 'dimension' attribute"));
    return false;
  }
  dimension = (size_t)xml->getIntAttribute(T("dimension"));
  values = GenericVector(getMatrixElementsType(), 0);
  return values.loadFromXml(xml, callback);
}

TypePtr SymmetricMatrix::getElementsType() const
  {jassert(thisClass); return symmetricMatrixRowClass(getMatrixElementsType());}

Variable SymmetricMatrix::getElement(size_t index) const
{
  jassert(index < dimension);
  return new SymmetricMatrixRow(refCountedPointerFromThis(this), index);
}

void SymmetricMatrix::setElement(size_t index, const Variable& value)
{
  // not implemented
  jassert(false);
}
/*
ClassPtr lbcpp::symmetricMatrixClass(TypePtr elementsType)
{
  static UnaryTemplateTypeCache cache(T("SymmetricMatrix"));
  return cache(elementsType);
}

ClassPtr lbcpp::symmetricMatrixRowClass(TypePtr elementsType)
{
  static UnaryTemplateTypeCache cache(T("SymmetricMatrixRow"));
  return cache(elementsType);
}
*/
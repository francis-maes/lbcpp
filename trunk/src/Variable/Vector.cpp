/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Object/Vector.h>
using namespace lbcpp;

size_t Vector::getNumVariables() const
  {return values.size();}

Variable Vector::getVariable(size_t index) const
  {jassert(index < values.size()); return Variable::copyFrom(type, values[index]);}

void Vector::setVariable(size_t index, const Variable& value)
{
  if (checkType(value))
  {
    jassert(index < values.size());
    value.copyTo(values[index]);
  }
}

void Vector::clear()
{
  for (size_t i = 0; i < values.size(); ++i)
    type->destroy(values[i]);
  values.clear();
}

void Vector::append(const Variable& value)
{
  if (checkType(value))
  {
    values.push_back(VariableValue());
    value.copyTo(values.back());
  }
}

bool Vector::checkType(const Variable& value) const
  {return checkInheritance(value, type);}

TypePtr Vector::getClass() const
  {return vectorClass(type);}

void Vector::saveToXml(XmlElement* xml) const
{
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String value;
    String oneLetterCodes = enumeration->getOneLetterCodes();
    for (size_t i = 0; i < getNumVariables(); ++i)
    {
      Variable variable = getVariable(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += oneLetterCodes[variable.getInteger()];
    }
    xml->addTextElement(value);
    return;
  }

  VariableContainer::saveToXml(xml);
}

bool Vector::loadFromXml(XmlElement* xml, ErrorHandler& callback)
{
  callback.error(T("Vector::loadFromXml"), T("Not Implemented"));
  return false;
}

ClassPtr lbcpp::vectorClass(TypePtr elementsType)
{
  static UnaryTemplateTypeCache cache(T("Vector"));
  return cache(elementsType);
}


// todo: ranger
#include <lbcpp/Object/SymmetricMatrix.h>
ClassPtr lbcpp::symmetricMatrixClass(TypePtr elementsType)
{
  static UnaryTemplateTypeCache cache(T("SymmetricMatrix"));
  return cache(elementsType);
}

/*-----------------------------------------.---------------------------------.
| Filename: Integer.cpp                    | Integer                         |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Integer.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** NewInteger
*/
NewIntegerPtr NewInteger::create(ClassPtr type, juce::int64 value)
{
  if (type.isInstanceOf<Enumeration>())
    return new NewEnumValue(type.staticCast<Enumeration>(), (size_t)value);
  else if (type == newPositiveIntegerClass)
    return new NewPositiveInteger(type, (size_t)value);
  else
    return new NewInteger(type, value);
}

String NewInteger::toShortString() const
  {return String(value);}

String NewInteger::toString() const
  {return String(value);}
  
double NewInteger::toDouble() const
  {return (double)value;}

int NewInteger::compare(const ObjectPtr& otherObject) const
{
  const NewIntegerPtr& other = otherObject.staticCast<NewInteger>();
  return (int)(value - other->get());
}

void NewInteger::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<NewInteger>()->value = value;}

bool NewInteger::loadFromString(ExecutionContext& context, const String& value)
{
  if (!value.trim().containsOnly(T("-+e0123456789")))
  {
    context.errorCallback(T("IntegerType::loadFromString"), value.quoted() + T(" is not a valid integer"));
    return false;
  }
  this->value = value.getLargeIntValue();
  return true;
}

bool NewInteger::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void NewInteger::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

/*
** NewEnumValue
*/
String NewEnumValue::toShortString() const
{
  EnumerationElementPtr element = getEnumerationElement();
  return element->getShortName().isNotEmpty() ? element->getShortName() : element->getName();
}

bool NewEnumValue::loadFromString(ExecutionContext& context, const String& value)
{
  EnumerationPtr enumeration = getEnumeration();

  String str = value.trim();
  size_t n = enumeration->getNumElements();
  size_t res = n;

  // try element names
  for (size_t i = 0; i < n; ++i)
    if (str == enumeration->getElementName(i))
    {
      res = i;
      break;
    }

  // try element short names
  if (res == n)
  {
    for (size_t i = 0; i < n; ++i)
      if (str == enumeration->getElement(i)->getShortName())
      {
        res = i;
        break;
      }
  }

  if (res == n)
  {
    context.errorCallback(T("NewEnumValue::createFromString"), T("Could not find enumeration value ") + value.quoted());
    return false;
  }
  this->value = (size_t)res;
  return true;
}

String NewEnumValue::toString() const
  {return getEnumerationElement()->getName();}

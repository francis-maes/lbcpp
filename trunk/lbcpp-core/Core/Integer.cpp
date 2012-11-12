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
** Integer
*/
IntegerPtr Integer::create(ClassPtr type, juce::int64 value)
{
  if (type.isInstanceOf<Enumeration>())
    return new EnumValue(type.staticCast<Enumeration>(), (size_t)value);
  else if (type == positiveIntegerClass)
    return new PositiveInteger(type, (size_t)value);
  else
    return new Integer(type, value);
}

String Integer::toShortString() const
  {return String(value);}

String Integer::toString() const
  {return String(value);}
  
double Integer::toDouble() const
  {return (double)value;}

int Integer::compare(const ObjectPtr& otherObject) const
{
  const IntegerPtr& other = otherObject.staticCast<Integer>();
  return (int)(value - other->get());
}

void Integer::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<Integer>()->value = value;}

bool Integer::loadFromString(ExecutionContext& context, const String& value)
{
  if (!value.trim().containsOnly(T("-+e0123456789")))
  {
    context.errorCallback(T("IntegerType::loadFromString"), value.quoted() + T(" is not a valid integer"));
    return false;
  }
  this->value = value.getLargeIntValue();
  return true;
}

bool Integer::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void Integer::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

/*
** EnumValue
*/
String EnumValue::toShortString() const
{
  EnumerationElementPtr element = getEnumerationElement();
  return element->getShortName().isNotEmpty() ? element->getShortName() : element->getName();
}

bool EnumValue::loadFromString(ExecutionContext& context, const String& value)
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
    context.errorCallback(T("EnumValue::createFromString"), T("Could not find enumeration value ") + value.quoted());
    return false;
  }
  this->value = (size_t)res;
  return true;
}

String EnumValue::toString() const
  {return getEnumerationElement()->getName();}

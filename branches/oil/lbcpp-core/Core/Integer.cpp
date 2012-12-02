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
  else if (type == memorySizeClass)
    return new MemorySize(type, (size_t)value);
  else
    return new Integer(type, value);
}

string Integer::toShortString() const
  {return string(value);}

string Integer::toString() const
  {return string(value);}
  
double Integer::toDouble() const
  {return (double)value;}

int Integer::compare(const ObjectPtr& otherObject) const
{
  const IntegerPtr& other = otherObject.staticCast<Integer>();
  return (int)(value - other->get());
}

void Integer::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<Integer>()->value = value;}

bool Integer::loadFromString(ExecutionContext& context, const string& value)
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
** MemorySize
*/
static inline string formatDoubleWithOneDigit(double value)
{
  int val = (int)(value * 10 + 0.5);
  return string(val / 10) + ((val % 10) ? "." + string(val % 10) : "");
}

string MemorySize::toShortString() const
{
  if (value == 0)
    return string("empty");
  if (value < 1024)
    return string(value) + " B";
  double size = value / 1024.0;
  if (size < 1024.0)
    return formatDoubleWithOneDigit(size) + " KB";
  size /= 1024.0;
  if (size < 1024.0)
    return formatDoubleWithOneDigit(size) + " MB";
  size /= 1024.0;
  return formatDoubleWithOneDigit(size) + " GB";
}

/*
** EnumValue
*/
string EnumValue::toShortString() const
{
  EnumerationElementPtr element = getEnumerationElement();
  return element->getShortName().isNotEmpty() ? element->getShortName() : element->getName();
}

bool EnumValue::loadFromString(ExecutionContext& context, const string& value)
{
  EnumerationPtr enumeration = getEnumeration();

  string str = value.trim();
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

string EnumValue::toString() const
  {return getEnumerationElement()->getName();}

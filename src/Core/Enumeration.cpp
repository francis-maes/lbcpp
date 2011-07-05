/*-----------------------------------------.---------------------------------.
| Filename: Enumeration.cpp                | Enumeration Introspection       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Enumeration.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <map>
using namespace lbcpp;

/*
** EnumerationElement
*/
EnumerationElement::EnumerationElement(const String& name, const String& oneLetterCode, const String& shortName, const String& description)
  : name(name), oneLetterCode(oneLetterCode), shortName(shortName), description(description)
{
}

String EnumerationElement::toShortString() const
{
  if (oneLetterCode.isNotEmpty())
    return oneLetterCode;
  if (shortName.isNotEmpty())
    return shortName;
  return name;
}

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const String& baseTypeName)
  : Type(name, lbcpp::getType(baseTypeName))
{
}

ClassPtr Enumeration::getClass() const
  {return enumerationClass;}

Variable Enumeration::create(ExecutionContext& context) const
  {return Variable(getNumElements(), refCountedPointerFromThis(this));}

VariableValue Enumeration::getMissingValue() const
  {return VariableValue((juce::int64)getNumElements());}

Variable Enumeration::createFromXml(XmlImporter& importer) const
  {return createFromString(importer.getContext(), importer.getAllSubText());}
 
Variable Enumeration::createFromString(ExecutionContext& context, const String& value) const
{
  String str = value.trim();
  size_t n = getNumElements();
  size_t res = n;

  // try element names
  for (size_t i = 0; i < n; ++i)
    if (str == getElementName(i))
    {
      res = i;
      break;
    }

  // try element short names
  if (res == n)
  {
    for (size_t i = 0; i < n; ++i)
      if (str == getElement(i)->getShortName())
      {
        res = i;
        break;
      }
  }

  if (res == n)
    context.errorCallback(T("Enumeration::createFromString"), T("Could not find enumeration value ") + value.quoted());
  return Variable(res, refCountedPointerFromThis(this));
}

void Enumeration::saveToXml(XmlExporter& exporter, const VariableValue& value) const
{
  exporter.addTextElement(toString(value));
}

String Enumeration::toString(const VariableValue& value) const
{
  juce::int64 val = value.getInteger();
  return val >= 0 && (size_t)val < getNumElements() ? getElementName((size_t)val) : T("Missing");
}

bool Enumeration::hasOneLetterCodes() const
{
  size_t n = getNumElements(); 
  for (size_t i = 0; i < n; ++i)
    if (getElement(i)->getOneLetterCode().length() != 1)
      return false;
  return true;
}

int Enumeration::findElementByName(const String& name) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (getElementName(i) == name)
      return (int)i;
  return -1;
}

int Enumeration::findElementByOneLetterCode(const juce::tchar c) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String code = getElement(i)->getOneLetterCode();
    if (code.length() == 1 && code[0] == c)
      return (int)i;
  }
  return -1;
}

String Enumeration::getElementName(size_t index) const
{
  jassert(index < getNumElements() || getNumElements() == 0);
  EnumerationElementPtr element = getElement(index);
  return element ? element->getName() : String::empty;
}

int Enumeration::compare(const ObjectPtr& otherObject) const
{
  EnumerationPtr other = otherObject.dynamicCast<Enumeration>();
  if (!other)
    return -1;
  size_t n = getNumElements();
  if (other->getNumElements() != n)
    return -2;
  for (size_t i = 0; i < n; ++i)
    if (getElement(i)->getName() != other->getElement(i)->getName())
      return i + 1;
  return 0;
}

/*
** DefaultEnumeration
*/
DefaultEnumeration::DefaultEnumeration(const String& name, const String& baseTypeName)
  : Enumeration(name, baseTypeName)
{
}

DefaultEnumeration::DefaultEnumeration()
 : Enumeration(T("UnnamedEnumeration"), T("EnumValue"))
{
}

void DefaultEnumeration::addElement(ExecutionContext& context, const String& name, const String& oneLetterCode, const String& shortName, const String& description)
{
  if (findElementByName(name) >= 0)
  {
    context.errorCallback(T("Enumeration::addElement"), T("Element '") + name + T("' already exists"));
    return;
  }
  elementsMap[name] = elements.size();
  elements.push_back(new EnumerationElement(name, oneLetterCode, shortName, description));
}

int DefaultEnumeration::findElementByName(const String& name) const
{
  std::map<String, size_t>::const_iterator it = elementsMap.find(name);
  return it == elementsMap.end() ? -1 : it->second;
}

size_t DefaultEnumeration::findOrAddElement(ExecutionContext& context, const String& name)
{
  int elt = findElementByName(name);
  if (elt >= 0)
    return (size_t)elt;
  size_t res = elements.size();
  elementsMap[name] = elements.size();
  elements.push_back(new EnumerationElement(name));
  return res;
}

void DefaultEnumeration::addElementsWithPrefix(ExecutionContext& context, const EnumerationPtr& enumeration, const String& namePrefix, const String& shortNamePrefix)
{
  size_t n = enumeration->getNumElements();
  elements.reserve(elements.size() + n);
  for (size_t i = 0; i < n; ++i)
  {
    EnumerationElementPtr element = enumeration->getElement(i);
    elements.push_back(new EnumerationElement(namePrefix + element->getName(), String::empty, shortNamePrefix + element->getShortName()));
  }
}

/*
** ConcatenateEnumeration
*/
ConcatenateEnumeration::ConcatenateEnumeration(const String& name)
  : Enumeration(name)
{
  indexToBaseIndex[0] = 0;
}

size_t ConcatenateEnumeration::getNumElements() const
  {return indexToBaseIndex.rbegin()->first;}

EnumerationElementPtr ConcatenateEnumeration::getElement(size_t index) const
{
  std::map<size_t, size_t>::const_iterator it = indexToBaseIndex.upper_bound(index);
  jassert(it != indexToBaseIndex.begin() && it != indexToBaseIndex.end());
  jassert(index < it->first);
  --it;
  jassert(index >= it->first);
  index -= it->first;
  size_t subEnumIndex = it->second;
  jassert(subEnumIndex < subEnumerations.size() && index < subEnumerations[subEnumIndex].second->getNumElements());
  const String& prefix = subEnumerations[subEnumIndex].first;
  EnumerationElementPtr element = subEnumerations[subEnumIndex].second->getElement(index);
  return new EnumerationElement(prefix + T(".") + element->getName(), String::empty, prefix + T(".") + element->getShortName());
}

void ConcatenateEnumeration::addSubEnumeration(const String& prefix, const EnumerationPtr& enumeration)
{
  size_t numElements = indexToBaseIndex.rbegin()->first;
  subEnumerations.push_back(std::make_pair(prefix, enumeration));
  indexToBaseIndex[numElements + enumeration->getNumElements()] = subEnumerations.size();
}

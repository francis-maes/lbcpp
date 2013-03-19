/*-----------------------------------------.---------------------------------.
| Filename: Enumeration.cpp                | Enumeration Introspection       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core.h>
#include <map>
using namespace lbcpp;

/*
** EnumerationElement
*/
EnumerationElement::EnumerationElement(const string& name, const string& oneLetterCode, const string& shortName, const string& description)
  : name(name), oneLetterCode(oneLetterCode), shortName(shortName), description(description)
{
}

string EnumerationElement::toShortString() const
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
Enumeration::Enumeration(const string& name, const string& baseTypeName)
  : Class(name, lbcpp::getType(baseTypeName))
{
}

ClassPtr Enumeration::getClass() const
  {return enumerationClass;}

ObjectPtr Enumeration::createObject(ExecutionContext& context) const
  {return new EnumValue(refCountedPointerFromThis(this), 0);}

bool Enumeration::hasOneLetterCodes() const
{
  size_t n = getNumElements(); 
  for (size_t i = 0; i < n; ++i)
    if (getElement(i)->getOneLetterCode().length() != 1)
      return false;
  return true;
}

int Enumeration::findElementByName(const string& name) const
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
    string code = getElement(i)->getOneLetterCode();
    if (code.length() == 1 && code[0] == c)
      return (int)i;
  }
  return -1;
}

string Enumeration::getElementName(size_t index) const
{
  jassert(index < getNumElements() || getNumElements() == 0);
  EnumerationElementPtr element = getElement(index);
  return element ? element->getName() : string::empty;
}

int Enumeration::compare(const ObjectPtr& otherObject) const
{
  EnumerationPtr other = otherObject.dynamicCast<Enumeration>();
  if (!other)
    return -1;
  size_t n = getNumElements();
  if (other->getNumElements() != n)
    return (int)n - (int)other->getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (getElement(i)->getName() != other->getElement(i)->getName())
      return i + 1;
  return 0;
}

/*
** DefaultEnumeration
*/
DefaultEnumeration::DefaultEnumeration(const string& name, const string& baseTypeName)
  : Enumeration(name, baseTypeName)
{
}

DefaultEnumeration::DefaultEnumeration()
 : Enumeration(T("UnnamedEnumeration"), T("EnumValue"))
{
}

void DefaultEnumeration::addElement(ExecutionContext& context, const string& name, const string& oneLetterCode, const string& shortName, const string& description)
{
  if (findElementByName(name) >= 0)
  {
    context.errorCallback(T("Enumeration::addElement"), T("Element '") + name + T("' already exists"));
    return;
  }
  elementsMap[name] = elements.size();
  elements.push_back(new EnumerationElement(name, oneLetterCode, shortName, description));
}

int DefaultEnumeration::findElementByName(const string& name) const
{
  std::map<string, size_t>::const_iterator it = elementsMap.find(name);
  return it == elementsMap.end() ? -1 : it->second;
}

size_t DefaultEnumeration::findOrAddElement(ExecutionContext& context, const string& name)
{
  int elt = findElementByName(name);
  if (elt >= 0)
    return (size_t)elt;
  size_t res = elements.size();
  elementsMap[name] = elements.size();
  elements.push_back(new EnumerationElement(name));
  return res;
}

void DefaultEnumeration::addElementsWithPrefix(ExecutionContext& context, const EnumerationPtr& enumeration, const string& namePrefix, const string& shortNamePrefix)
{
  size_t n = enumeration->getNumElements();
  elements.reserve(elements.size() + n);
  for (size_t i = 0; i < n; ++i)
  {
    EnumerationElementPtr element = enumeration->getElement(i);
    elements.push_back(new EnumerationElement(namePrefix + element->getName(), string::empty, shortNamePrefix + element->getShortName()));
  }
}

/*
** ConcatenateEnumeration
*/
ConcatenateEnumeration::ConcatenateEnumeration(const string& name)
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
  const string& prefix = subEnumerations[subEnumIndex].first;
  EnumerationElementPtr element = subEnumerations[subEnumIndex].second->getElement(index);
  return new EnumerationElement(prefix + T(".") + element->getName(), string::empty, prefix + T(".") + element->getShortName());
}

void ConcatenateEnumeration::addSubEnumeration(const string& prefix, const EnumerationPtr& enumeration)
{
  size_t numElements = indexToBaseIndex.rbegin()->first;
  subEnumerations.push_back(std::make_pair(prefix, enumeration));
  indexToBaseIndex[numElements + enumeration->getNumElements()] = subEnumerations.size();
}

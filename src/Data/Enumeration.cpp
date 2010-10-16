/*-----------------------------------------.---------------------------------.
| Filename: Enumeration.cpp                | Enumeration Introspection       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Type.h>
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/Vector.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include <map>
using namespace lbcpp;

Enumeration::Enumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes)
  : Type(name, enumValueType), oneLetterCodes(oneLetterCodes)
{
  jassert(!oneLetterCodes.containsChar('_')); // '_' is reserved to denote missing values
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& oneLetterCodes)
  : Type(name, enumValueType), oneLetterCodes(oneLetterCodes)
{
  jassert(!oneLetterCodes.containsChar('_'));
  for (int i = 0; i < oneLetterCodes.length(); ++i)
  {
    String str;
    str += oneLetterCodes[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : Type(name, enumValueType)
{
}

ClassPtr Enumeration::getClass() const
  {return enumerationClass;}

void Enumeration::addElement(const String& elementName, const String& oneLetterCode, const String& threeLettersCode)
{
  if (findElement(elementName) >= 0)
  {
    MessageCallback::error(T("Enumeration::addElement"), T("Element '") + elementName + T("' already exists"));
    return;
  }
  elements.push_back(elementName);
  if (oneLetterCode.length() == 1)
    oneLetterCodes += oneLetterCode;

  // FIXME: store three-letters code
}

int Enumeration::findElement(const String& name) const
{
  for (size_t i = 0; i < elements.size(); ++i)
    if (elements[i] == name)
      return (int)i;
  return -1;
}

VariableValue Enumeration::create() const
  {return getMissingValue();}

VariableValue Enumeration::getMissingValue() const
  {return VariableValue((juce::int64)getNumElements());}

VariableValue Enumeration::createFromXml(XmlImporter& importer) const
  {return createFromString(importer.getAllSubText(), importer.getCallback());}
 
VariableValue Enumeration::createFromString(const String& value, MessageCallback& callback) const
{
  String str = value.trim();
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (str == getElementName(i))
      return VariableValue(i);
  callback.errorMessage(T("Enumeration::createFromString"), T("Could not find enumeration value ") + value.quoted());
  return getMissingValue();
}

String Enumeration::toString(const VariableValue& value) const
{
  juce::int64 val = value.getInteger();
  return val >= 0 && (size_t)val < getNumElements() ? getElementName((size_t)val) : T("Nil");
}

bool Enumeration::hasOneLetterCodes() const
  {return oneLetterCodes.length() == (int)elements.size();}

juce::tchar Enumeration::getOneLetterCode(size_t index) const
{
  jassert(index < elements.size());
  if (oneLetterCodes.length())
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes[index];
  }
  else
  {
    jassert(elements[index].isNotEmpty());
    return elements[index][0];
  }
}

String Enumeration::getOneLetterCodes() const
{
  if (oneLetterCodes.isEmpty())
  {
    String res;
    for (size_t i = 0; i < elements.size(); ++i)
      res += getOneLetterCode(i);
    return res;
  }
  else
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes;
  }
}

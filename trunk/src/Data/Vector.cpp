/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Vector.h>
using namespace lbcpp;

Vector::Vector(TypePtr elementsType, size_t initialSize)
  : Container(vectorClass(elementsType))
{
  jassert(elementsType != topLevelType());
  if (initialSize)
    values.resize(initialSize, elementsType->getMissingValue());
}

size_t Vector::getNumVariables() const
  {return values.size();}

Variable Vector::getVariable(size_t index) const
  {jassert(index < values.size()); return Variable::copyFrom(getElementsType(), values[index]);}

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
  TypePtr type = getElementsType();
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
  {return checkInheritance(value, getElementsType());}

String Vector::toString() const
{
  TypePtr type = getElementsType();
  size_t n = size();
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String value;
    String oneLetterCodes = enumeration->getOneLetterCodes();
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getVariable(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += oneLetterCodes[variable.getInteger()];
    }
    return value;
  }

  if (type->inheritsFrom(doubleType()))
  {
    String value;
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getVariable(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += String(variable.getDouble());
      if (i < n - 1)
        value += ' ';
    }
    return value;
  }

  return Container::toString();
}

void Vector::saveToXml(XmlElement* xml) const
{
  size_t n = getNumVariables();

  TypePtr type = getElementsType();
  xml->setAttribute(T("size"), (int)n);

  if (n > 1000 && (type->inheritsFrom(integerType()) || type->inheritsFrom(doubleType()) || type->inheritsFrom(booleanType())))
  {
    juce::MemoryBlock block(&values[0], sizeof (VariableValue) * values.size());
    xml->setAttribute(T("binary"), T("true"));
    xml->addTextElement(block.toBase64Encoding());
    return;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if ((enumeration && enumeration->hasOneLetterCodes()) || type->inheritsFrom(doubleType()))
    xml->addTextElement(toString());
  else
    Container::saveToXml(xml);
}

bool Vector::loadFromXml(XmlElement* xml, ErrorHandler& callback)
{
  TypePtr type = getElementsType();
  jassert(type);
  int size = xml->getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    callback.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  values.resize(size, type->getMissingValue());

  if (xml->getBoolAttribute(T("binary")))
  {
    if (!type->inheritsFrom(integerType()) && !type->inheritsFrom(doubleType()) && !type->inheritsFrom(booleanType()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Unexpected type for binary encoding"));
      return false;
    }

    juce::MemoryBlock block;
    if (!block.fromBase64Encoding(xml->getAllSubText().trim()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Could not decode base 64"));
      return false;
    }

    if (block.getSize() != (int)(sizeof (VariableValue) * values.size()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Invalid data size: found ") + String(block.getSize())
        + T(" expected ") + String((int)(sizeof (VariableValue) * values.size())));
      return false;
    }
    memcpy(&values[0], block.getData(), block.getSize());
    return true;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String text = xml->getAllSubText().trim();
    if (text.length() != size)
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(text.length()));
      return false;
    }
    
    String oneLetterCodes = enumeration->getOneLetterCodes();
    for (size_t i = 0; i < values.size(); ++i)
    {
      int j = oneLetterCodes.indexOfChar(text[i]);
      if (j >= 0)
        values[i] = VariableValue(j);
      else
      {
        if (text[i] != '_')
          callback.warningMessage(T("Vector::loadFromXml"), String(T("Could not recognize one letter code '")) + text[i] + T("'"));
        values[i] = enumeration->getMissingValue();
      }
    }
    return true;
  }

  if (type->inheritsFrom(doubleType()))
  {
    String text = xml->getAllSubText().trim();
    StringArray tokens;
    tokens.addTokens(text, T(" \t\r\n"), NULL);
    if (tokens.size() != size)
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(tokens.size()));
      return false;
    }
    for (size_t i = 0; i < values.size(); ++i)
      if (tokens[i] != T("_"))
      {
        Variable value = Variable::createFromString(doubleType(), tokens[i], callback);
        if (!value)
          return false;
        values[i] = value.getDouble();
      }
    return true;
  }

  // default implementation  
  return Container::loadFromXml(xml, callback);
}

ClassPtr lbcpp::vectorClass(TypePtr elementsType)
{
  static UnaryTemplateTypeCache cache(T("Vector"));
  return cache(elementsType);
}

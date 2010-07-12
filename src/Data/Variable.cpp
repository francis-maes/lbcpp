/*-----------------------------------------.---------------------------------.
| Filename: Variable.cpp                   | Variable                        |
| Author  : Francis Maes                   |                                 |
| Started : 08/08/2010 12:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Variable.h>
#include "Type/TupleType.h"
using namespace lbcpp;

String Variable::getVariableName(size_t index) const
  {return type->getSubVariableName(value, index);}

Variable Variable::pair(const Variable& variable1, const Variable& variable2)
  {return Variable(pairType(variable1.getType(), variable2.getType()), PairType::allocate(variable1, variable2));}

Variable Variable::copyFrom(TypePtr type, const VariableValue& value)
{
  Variable res;
  res.type = type;
  if (type)
    type->copy(res.value, value);
  return res;
}

Variable Variable::createFromString(TypePtr type, const String& str, ErrorHandler& callback)
{
  String failureReason;
  VariableValue value = type->createFromString(str, callback);
  return type->isMissingValue(value) ? Variable() : Variable(type, value);
}

Variable Variable::createFromXml(XmlElement* xml, ErrorHandler& callback)
{
  String typeName = xml->getStringAttribute(T("type")).replaceCharacters(T("[]"), T("<>"));
  TypePtr type = Type::parseAndGet(typeName, callback);
  if (!type)
  {
    callback.errorMessage(T("Variable::createFromXml"), T("Could not find type ") + typeName.quoted());
    return Variable();
  }
  if (xml->getStringAttribute(T("missing")) == T("true"))
    return missingValue(type);

  return Variable(type, type->createFromXml(xml, callback));
}

Variable Variable::createFromFile(const File& file, ErrorHandler& callback)
{
  if (file.isDirectory())
  {
    callback.errorMessage(T("Variable::createFromFile"), file.getFullPathName() + T(" is a directory"));
    return Variable();
  }
  
  if (!file.existsAsFile())
  {
    callback.errorMessage(T("Variable::createFromFile"), file.getFullPathName() + T(" does not exists"));
    return Variable();
  }

  juce::XmlDocument document(file);
  
  XmlElement* xml = document.getDocumentElement();
  String lastParseError = document.getLastParseError();
  if (!xml)
  {
    callback.errorMessage(T("Variable::createFromFile"), lastParseError);
    return Variable();
  }
  if (lastParseError.isNotEmpty())
    callback.warningMessage(T("Variable::createFromFile"), lastParseError);
  Variable res = createFromXml(xml, callback);
  delete xml;
  return res;
}

XmlElement* Variable::toXml(const String& tagName, const String& name) const
{
  XmlElement* res = new XmlElement(tagName);
  res->setAttribute(T("type"), getTypeName().replaceCharacters(T("<>"), T("[]")));
  if (name.isNotEmpty())
    res->setAttribute(T("name"), name);
  if (type->isMissingValue(value))
    res->setAttribute(T("missing"), T("true"));
  else
    type->saveToXml(res, value);
  return res;
}

bool Variable::saveToFile(const File& file, ErrorHandler& callback) const
{
  if (file.exists())
  {
    if (file.existsAsFile())
    {
      if (!file.deleteFile())
      {
        callback.errorMessage(T("Variable::saveToFile"), T("Could not delete file ") + file.getFullPathName());
        return false;
      }
    }
    else
    {
      callback.errorMessage(T("Variable::saveToFile"), file.getFullPathName() + T(" is a directory"));
      return false;
    }
  }
  
  XmlElement* xml = toXml();
  if (!xml)
  {
    callback.errorMessage(T("Variable::saveToFile"), T("Could not generate xml for file ") + file.getFullPathName());
    return false;
  }
  bool ok = xml->writeToFile(file, String::empty);
  if (!ok)
    callback.errorMessage(T("Variable::saveToFile"), T("Could not write file ") + file.getFullPathName());
  delete xml;
  return ok;
}

int Variable::compare(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.getType();
  if (type != type2)
  {
    if (type->inheritsFrom(type2))
      return type2->compare(value, otherValue.value);
    else if (type2->inheritsFrom(type))
      return type->compare(value, otherValue.value);
    else
      return getTypeName().compare(otherValue.getTypeName());
  }
  return type->compare(value, otherValue.value);
}

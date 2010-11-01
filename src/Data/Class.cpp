/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | Class Introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Type.h>
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/Vector.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include <map>
using namespace lbcpp;

/*
** Class
*/
String Class::toString() const
{
  String res = getName();
  res += T(" = {");
  size_t n = getObjectNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    res += getObjectVariableType(i)->getName() + T(" ") + getObjectVariableName(i);
    if (i < n - 1)
      res += T(", ");
  }
  res += T("}");
  return res;
}

int Class::compare(const VariableValue& value1, const VariableValue& value2) const
{
  ObjectPtr object1 = value1.getObject();
  ObjectPtr object2 = value2.getObject();
  if (!object1)
    return object2 ? -1 : 0;
  if (!object2)
    return 1;
  return object1->compare(object2);
}

VariableValue Class::createFromString(const String& value, MessageCallback& callback) const
{
  VariableValue res = create();
  if (isMissingValue(res))
  {
    callback.errorMessage(T("Class::createFromString"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  return res.getObject()->loadFromString(value, callback) ? res : getMissingValue();
}

VariableValue Class::createFromXml(XmlImporter& importer) const
{
  VariableValue res = create();
  if (isMissingValue(res))
  {
    importer.errorMessage(T("Class::createFromXml"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  return res.getObject()->loadFromXml(importer) ? res : getMissingValue();
}

void Class::saveToXml(XmlExporter& exporter, const VariableValue& value) const
{
  ObjectPtr object = value.getObject();
  jassert(object);
  object->saveToXml(exporter);
}

ClassPtr Class::getClass() const
  {return classClass;}

/*
** DefaultClass
*/
DefaultClass::DefaultClass(const String& name, TypePtr baseClass)
  : Class(name, baseClass)
{
}

DefaultClass::DefaultClass(const String& name, const String& baseClass)
  : Class(name, Class::get(baseClass))
{
}

DefaultClass::DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
  : Class(templateType, templateArguments, baseClass) {}

void DefaultClass::clearVariables()
{
  variables.clear();
}

void DefaultClass::addVariable(const String& typeName, const String& name)
{
  TypePtr type;
  if (templateType)
    type = templateType->instantiateTypeName(typeName, templateArguments, MessageCallback::getInstance());
  else
    type = Type::get(typeName);
  if (type)
    addVariable(type, name);
}

void DefaultClass::addVariable(TypePtr type, const String& name)
{
  if (!type || name.isEmpty())
  {
    MessageCallback::error(T("Class::addVariable"), T("Invalid type or name"));
    return;
  }
  if (findObjectVariable(name) >= 0)
  {
    MessageCallback::error(T("Class::addVariable"), T("Another variable with name '") + name + T("' already exists"));
    return;
  }
  variablesMap[name] = variables.size();
  variables.push_back(std::make_pair(type, name));
}

size_t DefaultClass::getObjectNumVariables() const
{
  size_t n = baseType->getObjectNumVariables();
  return n + variables.size();
}

TypePtr DefaultClass::getObjectVariableType(size_t index) const
{
  size_t n = baseType->getObjectNumVariables();
  if (index < n)
    return baseType->getObjectVariableType(index);
  index -= n;
  
  jassert(index < variables.size());
  return variables[index].first;
}

String DefaultClass::getObjectVariableName(size_t index) const
{
  size_t n = baseType->getObjectNumVariables();
  if (index < n)
    return baseType->getObjectVariableName(index);
  index -= n;
  
  jassert(index < variables.size());
  return variables[index].second;
}

void DefaultClass::deinitialize()
{
  variables.clear();
  Class::deinitialize();
}

int DefaultClass::findObjectVariable(const String& name) const
{
  std::map<String, size_t>::const_iterator it = variablesMap.find(name);
  if (it != variablesMap.end())
    return (int)(baseType->getObjectNumVariables() + it->second);
  return baseType->findObjectVariable(name);
}

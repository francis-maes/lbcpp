/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | Class Introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Class.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Vector.h>
#include <map>
using namespace lbcpp;

/*
** Class
*/
String Class::toString() const
{
  String res = getName();
  res += T(" = {");
  size_t n = getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    res += getMemberVariableType(i)->getName() + T(" ") + getMemberVariableName(i);
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

inline ObjectPtr createObjectFromShortNameOrName(ExecutionContext& context, ClassPtr baseClass, const String& nameOrShortName)
{
  if (nameOrShortName == T("Missing"))
    return ObjectPtr();
  TypePtr type = typeManager().getTypeByShortName(context, nameOrShortName);
  if (!type)
    type = typeManager().getType(context, nameOrShortName);
  if (!type)
    return ObjectPtr();
  if (!context.checkInheritance(type, baseClass))
    return ObjectPtr();
  return Object::create(type);
}

inline ObjectPtr createObjectFromStringWithAbstractClass(ExecutionContext& context, ClassPtr baseClass, const String& value)
{
  int n = value.indexOfChar('(');
  if (n >= 0)
  {
    ObjectPtr res = createObjectFromShortNameOrName(context, baseClass, value.substring(0, n));
    if (!res)
      return ObjectPtr();

    int e = value.lastIndexOfChar(')');
    if (e <= n)
    {
      context.errorCallback(T("Unmatched parenthesis in ") + value.quoted());
      return ObjectPtr();
    }
    String arguments = value.substring(n + 1, e).trim();
    if (arguments.isNotEmpty() && !res->loadFromString(context, arguments))
      res = ObjectPtr();
    return res;
  }
  else
    return createObjectFromShortNameOrName(context, baseClass, value);
}

Variable Class::createFromString(ExecutionContext& context, const String& value) const
{
  ObjectPtr object;

  if (isAbstract())
    object = createObjectFromStringWithAbstractClass(context, refCountedPointerFromThis(this), value);
  else
  {
    object = create(context).getObject();
    if (!object)
      context.errorCallback(T("Class::createFromString"), T("Could not create instance of ") + getName().quoted());
    else if (!object->loadFromString(context, value))
      object = ObjectPtr();
  }
  return Variable(object, refCountedPointerFromThis(this));
}

Variable Class::createFromXml(XmlImporter& importer) const
{
  ObjectPtr object = create(importer.getContext()).getObject();
  if (!object)
    importer.errorMessage(T("Class::createFromXml"), T("Could not create instance of ") + getName().quoted());
  else if (!object->loadFromXml(importer))
    object = ObjectPtr();
  return Variable(object, refCountedPointerFromThis(this));
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
  : Class(name, baseClass), abstractClass(false)
{
}

DefaultClass::DefaultClass(const String& name, const String& baseClass)
: Class(name, lbcpp::getType(baseClass)), abstractClass(false)
{
}

DefaultClass::DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
  : Class(templateType, templateArguments, baseClass), abstractClass(false) {}

namespace lbcpp {extern ClassPtr defaultClassClass;};

ClassPtr DefaultClass::getClass() const
  {return defaultClassClass;}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName, const String& description, bool isGenerated)
{
  TypePtr type;
  if (templateType)
    type = templateType->instantiateTypeName(context, typeName, templateArguments);
  else
    type = typeManager().getType(context, typeName);
  if (type)
    return addMemberVariable(context, type, name, shortName, description, isGenerated);
  else
    return (size_t)-1;
}

class GeneratedVariableSignature : public VariableSignature
{
public:
  GeneratedVariableSignature(TypePtr type,
                    const String& name,
                    const String& shortName = String::empty,
                    const String& description = String::empty)
    : VariableSignature(type, name, shortName, description) {}
    
  GeneratedVariableSignature() {}
};

size_t DefaultClass::addMemberVariable(ExecutionContext& context, TypePtr type, const String& name, const String& shortName, const String& description, bool isGenerated)
{
  if (!type || name.isEmpty())
  {
    context.errorCallback(T("Class::addMemberVariable"), T("Invalid type or name"));
    return (size_t)-1;
  }
  if (findMemberVariable(name) >= 0)
  {
    context.errorCallback(T("Class::addMemberVariable"), T("Another variable with name '") + name + T("' already exists"));
    return (size_t)-1;
  }
  VariableSignaturePtr signature;
  if (isGenerated)
    signature = new GeneratedVariableSignature(type, name, shortName, description);
  else
    signature = new VariableSignature(type, name, shortName, description);
  
  return addMemberVariable(context, signature);
}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature)
{
  jassert(signature);
  size_t res = variables.size();
  variablesMap[signature->getName()] = res;
  variables.push_back(signature);
  return res;
}

bool DefaultClass::isMemberVariableGenerated(size_t index) const
  {return getMemberVariable(index).dynamicCast<GeneratedVariableSignature>();}

size_t DefaultClass::getNumMemberVariables() const
{
  size_t n = baseType->getNumMemberVariables();
  return n + variables.size();
}

VariableSignaturePtr DefaultClass::getMemberVariable(size_t index) const
{
  size_t n = baseType->getNumMemberVariables();
  if (index < n)
    return baseType->getMemberVariable(index);
  index -= n;
  
  jassert(index < variables.size());
  return variables[index];
}

void DefaultClass::deinitialize()
{
  variables.clear();
  Class::deinitialize();
}

int DefaultClass::findMemberVariable(const String& name) const
{
  std::map<String, size_t>::const_iterator it = variablesMap.find(name);
  if (it != variablesMap.end())
    return (int)(baseType->getNumMemberVariables() + it->second);
  return baseType->findMemberVariable(name);
}

size_t DefaultClass::findOrAddMemberVariable(ExecutionContext& context, const String& name, TypePtr type)
{
  int idx = findMemberVariable(name);
  if (idx >= 0)
    return (size_t)idx;
  return addMemberVariable(context, type, name);
}

size_t DefaultClass::getNumMemberFunctions() const
  {return baseType->getNumMemberFunctions() + functions.size();}

FunctionSignaturePtr DefaultClass::getMemberFunction(size_t index) const
{
  size_t n = baseType->getNumMemberFunctions();
  if (index < n)
    return baseType->getMemberFunction(index);
  index -= n;

  jassert(index < functions.size());
  return functions[index];
}

int DefaultClass::findMemberFunction(const String& name) const
{
 std::map<String, size_t>::const_iterator it = functionsMap.find(name);
  if (it != functionsMap.end())
    return (int)(baseType->getNumMemberFunctions() + it->second);
  return baseType->findMemberFunction(name);
}

size_t DefaultClass::addMemberFunction(ExecutionContext& context, LuaFunction function, const String& name, const String& shortName, const String& description)
{
  if (!function || name.isEmpty())
  {
    context.errorCallback(T("Class::addMemberFunction"), T("Invalid function or name"));
    return (size_t)-1;
  }
  if (findMemberFunction(name) >= 0)
  {
    context.errorCallback(T("Class::addMemberFunction"), T("Another function with name '") + name + T("' already exists"));
    return (size_t)-1;
  }
  FunctionSignaturePtr signature = new LuaFunctionSignature(function, name, shortName, description);
 
  size_t res = functions.size();
  functionsMap[signature->getName()] = res;
  functions.push_back(signature);
  return res;
}
/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | Class Introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core.h>
#include <map>
using namespace lbcpp;

/*
** DefaultClass
*/
DefaultClass::DefaultClass(const String& name, ClassPtr baseClass)
  : Class(name, baseClass), abstractClass(false)
{
}

DefaultClass::DefaultClass(const String& name, const String& baseClass)
: Class(name, lbcpp::getType(baseClass)), abstractClass(false)
{
}

DefaultClass::DefaultClass(TemplateTypePtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseClass)
  : Class(templateType, templateArguments, baseClass), abstractClass(false) {}

namespace lbcpp {extern ClassPtr defaultClassClass;};

ClassPtr DefaultClass::getClass() const
  {return defaultClassClass;}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName, const String& description, bool isGenerated)
{
  ClassPtr type;
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
  GeneratedVariableSignature(ClassPtr type,
                    const String& name,
                    const String& shortName = String::empty,
                    const String& description = String::empty)
    : VariableSignature(type, name, shortName, description) {thisClass = variableSignatureClass;}
    
  GeneratedVariableSignature() {thisClass = variableSignatureClass;}
};

size_t DefaultClass::addMemberVariable(ExecutionContext& context, ClassPtr type, const String& name, const String& shortName, const String& description, bool isGenerated)
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

size_t DefaultClass::findOrAddMemberVariable(ExecutionContext& context, const String& name, ClassPtr type)
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

size_t DefaultClass::addMemberFunction(ExecutionContext& context, LuaCFunction function, const String& name, const String& shortName, const String& description, bool isStatic)
{
  if (!function || name.isEmpty())
  {
    context.errorCallback(T("Class::addMemberFunction"), T("Invalid function or name"));
    return (size_t)-1;
  }

  FunctionSignaturePtr signature = new LuaFunctionSignature(function, name, shortName, description, isStatic);
 
  size_t res = functions.size();
  functionsMap[signature->getName()] = res;
  functions.push_back(signature);
  return res;
}

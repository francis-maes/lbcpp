/*-----------------------------------------.---------------------------------.
| Filename: ClassManager.cpp               | Global Class Manager            |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/ClassManager.h>
#include <lbcpp/Core/Class.h>
#include <lbcpp/Core/DefaultClass.h>
#include <lbcpp/Core/TemplateClass.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/library.h>
using namespace lbcpp;

/*
** TemplateClassCache
*/
namespace lbcpp
{

struct TemplateClassCache
{
  TemplateClassCache() {}
  TemplateClassCache(const TemplateClassCache& other) : definition(other.definition)
  {
    ScopedLock _(other.instancesLock);
    instances = other.instances;
  }
  TemplateClassPtr definition;

  ClassPtr getInstanceCached(ExecutionContext& context, const std::vector<ClassPtr>& arguments)
  {
    ScopedLock _(instancesLock);
    TemplateClassCache::InstanceMap::iterator it = instances.find(arguments);
    if (it != instances.end())
      return it->second;
    else
      return instantiate(context, arguments);
  }

  void clear(std::vector<Class* >& toDelete)
  {
    ScopedLock _(instancesLock);
    toDelete.reserve(toDelete.size() + instances.size());
    for (InstanceMap::iterator it = instances.begin(); it != instances.end(); ++it)
    {
      it->second->deinitialize();
      toDelete.push_back(it->second.get());
    }
    instances.clear();
  }

private:
  CriticalSection instancesLock;
  typedef std::map< std::vector<ClassPtr>, ClassPtr > InstanceMap;
  InstanceMap instances;

  ClassPtr instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments)
  {
    ClassPtr res = definition->instantiate(context, arguments);
    if (!res || !res->initialize(context))
      return ClassPtr();
    res->setStaticAllocationFlag();
    instances[arguments] = res;
    bool isNamedType = true;
    for (size_t i = 0; i < arguments.size(); ++i)
      isNamedType &= arguments[i]->isNamedType();
    res->namedType = isNamedType;
    return res;
  }
};

}; /* namespace lbcpp */

/*
** ClassManager
*/
ClassManager::ClassManager() {}

ClassManager::~ClassManager()
  {shutdown();}

bool ClassManager::declare(ExecutionContext& context, ClassPtr type)
{
  if (!type || type->getName().isEmpty())
  {
    context.errorCallback(T("ClassManager::declare"), T("Empty class name"));
    return false;
  }

  ScopedLock _(typesLock);
  string typeName = type->getName();
  if (doTypeExists(typeName))
  {
    context.errorCallback(T("ClassManager::declare"), T("Class '") + typeName + T("' has already been declared"));
    return false;
  }
  type->setStaticAllocationFlag();
  types[typeName] = type;
  if (type->getShortName().isNotEmpty())
    typesByShortName[type->getShortName()] = type;
  type->namedType = true;
  return true;
}

bool ClassManager::declare(ExecutionContext& context, TemplateClassPtr templateType)
{
  if (!templateType || templateType->getName().isEmpty())
    context.errorCallback(T("ClassManager::declare"), T("Empty template class name"));

  ScopedLock _(typesLock);
  string typeName = templateType->getName();
  TemplateClassMap::const_iterator it = templateTypes.find(typeName);
  if (it != templateTypes.end())
  {
    context.errorCallback(T("ClassManager::declare"), T("Template type '") + typeName + T("' has already been declared"));
    return false;
  }
  templateTypes[typeName].definition = templateType;
  return true;
}

void ClassManager::finishDeclarations(ExecutionContext& context)
{
  ScopedLock _(typesLock);
  for (TypeMap::const_iterator it = types.begin(); it != types.end(); ++it)
  {
    string name = it->first;
    ClassPtr type = it->second;
    if (!type->isInitialized() && !type->initialize(context))
      context.errorCallback(T("ClassManager::finishDeclarations()"), T("Could not initialize type ") + type->getName());
  }

  for (TemplateClassMap::const_iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
  {
    string name = it->first;
    TemplateClassPtr templateType = it->second.definition;
    if (!templateType->isInitialized() && !templateType->initialize(context))
      context.errorCallback(T("ClassManager::finishDeclarations()"), T("Could not initialize template type ") + templateType->getName());
  }
}

ClassPtr ClassManager::getType(ExecutionContext& context, const string& typeName, const std::vector<ClassPtr>& arguments) const
{
#ifdef JUCE_DEBUG
  jassert(arguments.size());
  for (size_t i = 0; i < arguments.size(); ++i)
    jassert(arguments[i]);
#endif // JUCE_DEBUG

  jassert(!typeName.containsAnyOf(T(" \t\r\n")));
  if (typeName.isEmpty())
  {
    context.errorCallback(T("ClassManager::getType"), T("Empty type name"));
    return ClassPtr();
  }
  jassert(!TemplateClass::isInstanciatedTypeName(typeName));

  ScopedLock _(typesLock);
  TemplateClassCache* templateType = getTemplateClass(context, typeName);
  return templateType ? templateType->getInstanceCached(context, arguments) : ClassPtr();
}

ClassPtr ClassManager::getTypeByShortName(ExecutionContext& context, const string& shortName) const
{
  TypeMap::const_iterator it = typesByShortName.find(shortName);
  if (it == typesByShortName.end())
    return ClassPtr();
  else
    return it->second;
}

ClassPtr ClassManager::getType(ExecutionContext& context, const string& name) const
{
  string typeName = removeAllSpaces(name);
  if (typeName.isEmpty())
  {
    context.errorCallback(T("ClassManager::getType"), T("Empty type name"));
    return ClassPtr();
  }

  ScopedLock _(typesLock);
  ClassPtr type = findType(typeName);
  if (type)
    return type;

  if (TemplateClass::isInstanciatedTypeName(typeName))
  {
    // this is a template type, parse, instantiate and retrieve it
    string templateName;
    std::vector<ClassPtr> templateArguments;
    if (!TemplateClass::parseInstanciatedTypeName(context, typeName, templateName, templateArguments))
      return ClassPtr();
    return getType(context, templateName, templateArguments);
  }
  else
  {
    // this is a simple type that was not found, make error message
    string message;
    if (hasTemplateClass(typeName))
    {
      TemplateClassCache* cache = getTemplateClass(context, typeName);
      size_t n = cache->definition->getNumParameters();
      message = typeName.quoted() + T(" is a template type with ") + string((int)n) + T(" parameters. Replace ") + typeName + T(" by ");
      message += typeName + T("[");
      for (size_t i = 0; i < n; ++i)
      {
        message += cache->definition->getParameterName(i) + T(" (") + cache->definition->getParameterBaseType(i)->getName() + T(")");
        if (i < n - 1)
          message += T(", ");
      }
      message += T("]");
    }
    else
      message = T("Class ") + typeName.quoted() + T(" does not exists");

    context.errorCallback(T("ClassManager::getType"), message);
    return ClassPtr();
  }
}

string ClassManager::removeAllSpaces(const string& str)
{
  if (str.indexOfAnyOf(T(" \t\n\r")) < 0)
    return str;
  string res;
  for (int i = 0; i < str.length(); ++i)
    if (str[i] != ' ' && str[i] != '\t')
      res += str[i];
  return res;
}

ClassPtr ClassManager::findType(const string& name) const
{
  ScopedLock _(typesLock);
  TypeMap::const_iterator it = types.find(removeAllSpaces(name));
  return it == types.end() ? ClassPtr() : it->second;
}

bool ClassManager::doTypeExists(const string& type) const
  {return findType(type);}

void ClassManager::shutdown()
{
  ScopedLock _(typesLock);
  std::vector<Class* > toDelete; // we keep traditional pointers, remove the remaining shared ptr and then perform deletion
  for (TemplateClassMap::iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
    it->second.clear(toDelete);
  templateTypes.clear();

  toDelete.reserve(toDelete.size() + types.size());
  for (TypeMap::iterator it = types.begin(); it != types.end(); ++it)
  {
    it->second->deinitialize();
    jassert(it->second->hasStaticAllocationFlag());
    toDelete.push_back(it->second.get());
  }
  types.clear();
  typesByShortName.clear();

  for (size_t i = 0; i < toDelete.size(); ++i)
    delete toDelete[i];
}

bool ClassManager::hasTemplateClass(const string& templateTypeName) const
{
  ScopedLock _(typesLock);
  TemplateClassMap::iterator it = const_cast<ClassManager* >(this)->templateTypes.find(templateTypeName);
  return it != templateTypes.end();
}

TemplateClassCache* ClassManager::getTemplateClass(ExecutionContext& context, const string& templateTypeName) const
{
  ScopedLock _(typesLock);
  TemplateClassMap::iterator it = const_cast<ClassManager* >(this)->templateTypes.find(templateTypeName);
  if (it == templateTypes.end())
  {
    context.errorCallback(T("ClassManager::getTemplateClass()"), T("Could not find template type ") + templateTypeName);
    return NULL;
  }
  return &it->second;
}

/*
** Global
*/
namespace lbcpp
{
  ClassPtr getType(const string& typeName)
    {return typeManager().getType(defaultExecutionContext(), typeName);}

  ClassPtr getType(const string& name, const std::vector<ClassPtr>& arguments)
    {return typeManager().getType(defaultExecutionContext(), name, arguments);}

  bool doTypeExists(const string& typeName)
    {return typeManager().doTypeExists(typeName);}

  bool declareType(ClassPtr type)
    {return typeManager().declare(defaultExecutionContext(), type);}

};

/*-----------------------------------------.---------------------------------.
| Filename: TypeManager.cpp                | Global Type Manager             |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Type.h>
#include <lbcpp/Core/Class.h>
#include <lbcpp/Core/TemplateType.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/library.h>
using namespace lbcpp;

/*
** TemplateTypeCache
*/
namespace lbcpp
{

struct TemplateTypeCache
{
  TemplateTypeCache() {}
  TemplateTypeCache(const TemplateTypeCache& other) : definition(other.definition)
  {
    ScopedLock _(other.instancesLock);
    instances = other.instances;
  }
  TemplateTypePtr definition;

  TypePtr getInstanceCached(ExecutionContext& context, const std::vector<TypePtr>& arguments)
  {
    ScopedLock _(instancesLock);
    TemplateTypeCache::InstanceMap::iterator it = instances.find(arguments);
    if (it != instances.end())
      return it->second;
    else
      return instantiate(context, arguments);
  }

  void clear(std::vector<Type* >& toDelete)
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
  typedef std::map< std::vector<TypePtr>, TypePtr > InstanceMap;
  InstanceMap instances;

  TypePtr instantiate(ExecutionContext& context, const std::vector<TypePtr>& arguments)
  {
    TypePtr res = definition->instantiate(context, arguments);
    if (!res || !res->initialize(context))
      return TypePtr();
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
** TypeManager
*/
TypeManager::TypeManager() {}

TypeManager::~TypeManager()
  {shutdown();}

bool TypeManager::declare(ExecutionContext& context, TypePtr type)
{
  if (!type || type->getName().isEmpty())
  {
    context.errorCallback(T("TypeManager::declare"), T("Empty class name"));
    return false;
  }

  ScopedLock _(typesLock);
  String typeName = type->getName();
  if (doTypeExists(typeName))
  {
    context.errorCallback(T("TypeManager::declare"), T("Type '") + typeName + T("' has already been declared"));
    return false;
  }
  type->setStaticAllocationFlag();
  types[typeName] = type;
  if (type->getShortName().isNotEmpty())
    typesByShortName[type->getShortName()] = type;
  type->namedType = true;
  return true;
}

bool TypeManager::declare(ExecutionContext& context, TemplateTypePtr templateType)
{
  if (!templateType || templateType->getName().isEmpty())
    context.errorCallback(T("TypeManager::declare"), T("Empty template class name"));

  ScopedLock _(typesLock);
  String typeName = templateType->getName();
  TemplateTypeMap::const_iterator it = templateTypes.find(typeName);
  if (it != templateTypes.end())
  {
    context.errorCallback(T("TypeManager::declare"), T("Template type '") + typeName + T("' has already been declared"));
    return false;
  }
  templateTypes[typeName].definition = templateType;
  return true;
}

void TypeManager::finishDeclarations(ExecutionContext& context)
{
  ScopedLock _(typesLock);
  for (TypeMap::const_iterator it = types.begin(); it != types.end(); ++it)
  {
    String name = it->first;
    TypePtr type = it->second;
    if (!type->isInitialized() && !type->initialize(context))
      context.errorCallback(T("TypeManager::finishDeclarations()"), T("Could not initialize type ") + type->getName());
  }

  for (TemplateTypeMap::const_iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
  {
    String name = it->first;
    TemplateTypePtr templateType = it->second.definition;
    if (!templateType->isInitialized() && !templateType->initialize(context))
      context.errorCallback(T("TypeManager::finishDeclarations()"), T("Could not initialize template type ") + templateType->getName());
  }
}

TypePtr TypeManager::getType(ExecutionContext& context, const String& typeName, const std::vector<TypePtr>& arguments) const
{
#ifdef JUCE_DEBUG
  jassert(arguments.size());
  for (size_t i = 0; i < arguments.size(); ++i)
    jassert(arguments[i]);
#endif // JUCE_DEBUG

  jassert(!typeName.containsAnyOf(T(" \t\r\n")));
  if (typeName.isEmpty())
  {
    context.errorCallback(T("TypeManager::getType"), T("Empty type name"));
    return TypePtr();
  }
  jassert(!TemplateType::isInstanciatedTypeName(typeName));

  ScopedLock _(typesLock);
  TemplateTypeCache* templateType = getTemplateType(context, typeName);
  return templateType ? templateType->getInstanceCached(context, arguments) : TypePtr();
}

TypePtr TypeManager::getTypeByShortName(ExecutionContext& context, const String& shortName) const
{
  TypeMap::const_iterator it = typesByShortName.find(shortName);
  if (it == typesByShortName.end())
    return TypePtr();
  else
    return it->second;
}

TypePtr TypeManager::getType(ExecutionContext& context, const String& name) const
{
  String typeName = removeAllSpaces(name);
  if (typeName.isEmpty())
  {
    context.errorCallback(T("TypeManager::getType"), T("Empty type name"));
    return TypePtr();
  }

  ScopedLock _(typesLock);
  TypePtr type = findType(typeName);
  if (type)
    return type;

  if (TemplateType::isInstanciatedTypeName(typeName))
  {
    // this is a template type, parse, instantiate and retrieve it
    String templateName;
    std::vector<TypePtr> templateArguments;
    if (!TemplateType::parseInstanciatedTypeName(context, typeName, templateName, templateArguments))
      return TypePtr();
    return getType(context, templateName, templateArguments);
  }
  else
  {
    // this is a simple type that was not found, make error message
    String message;
    if (hasTemplateType(typeName))
    {
      TemplateTypeCache* cache = getTemplateType(context, typeName);
      size_t n = cache->definition->getNumParameters();
      message = typeName.quoted() + T(" is a template type with ") + String((int)n) + T(" parameters. Replace ") + typeName + T(" by ");
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
      message = T("Type ") + typeName.quoted() + T(" does not exists");

    context.errorCallback(T("TypeManager::getType"), message);
    return TypePtr();
  }
}

String TypeManager::removeAllSpaces(const String& str)
{
  if (str.indexOfAnyOf(T(" \t\n\r")) < 0)
    return str;
  String res;
  for (int i = 0; i < str.length(); ++i)
    if (str[i] != ' ' && str[i] != '\t')
      res += str[i];
  return res;
}

TypePtr TypeManager::findType(const String& name) const
{
  ScopedLock _(typesLock);
  TypeMap::const_iterator it = types.find(removeAllSpaces(name));
  return it == types.end() ? TypePtr() : it->second;
}

bool TypeManager::doTypeExists(const String& type) const
  {return findType(type);}

void TypeManager::shutdown()
{
  ScopedLock _(typesLock);
  std::vector<Type* > toDelete; // we keep traditional pointers, remove the remaining shared ptr and then perform deletion
  for (TemplateTypeMap::iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
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

bool TypeManager::hasTemplateType(const String& templateTypeName) const
{
  ScopedLock _(typesLock);
  TemplateTypeMap::iterator it = const_cast<TypeManager* >(this)->templateTypes.find(templateTypeName);
  return it != templateTypes.end();
}

TemplateTypeCache* TypeManager::getTemplateType(ExecutionContext& context, const String& templateTypeName) const
{
  ScopedLock _(typesLock);
  TemplateTypeMap::iterator it = const_cast<TypeManager* >(this)->templateTypes.find(templateTypeName);
  if (it == templateTypes.end())
  {
    context.errorCallback(T("TypeManager::getTemplateType()"), T("Could not find template type ") + templateTypeName);
    return NULL;
  }
  return &it->second;
}

/*
** Library
*/
bool Library::declareType(ExecutionContext& context, TypePtr type)
{
  if (!typeManager().declare(context, type))
    return false;
  types.push_back(type);
  return true;
}

bool Library::declareTemplateType(ExecutionContext& context, TemplateTypePtr templateType)
{
  if (!typeManager().declare(context, templateType))
    return false;
  templateTypes.push_back(templateType);
  return true;
}

bool Library::declareSubLibrary(ExecutionContext& context, LibraryPtr subLibrary)
{
  if (!subLibrary)
    return true; // this may happen with the "ifdef" directive in the <import> command
  if (!subLibrary->initialize(context))
    return false;
  subLibraries.push_back(subLibrary);
  return true;
}

#ifdef LBCPP_USER_INTERFACE
bool Library::declareUIComponent(ExecutionContext& context, const String& typeName, UIComponentConstructor constructor)
{
  TypePtr type = typeManager().getType(context, typeName);
  if (!type)
    return false;
  uiComponents.push_back(std::make_pair(type, constructor));
  return true;
}

juce::Component* Library::createUIComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name)
{
  ClassPtr type = object->getClass();
  for (size_t i = 0; i < uiComponents.size(); ++i)
    if (type->inheritsFrom(uiComponents[i].first))
      return uiComponents[i].second(object, name);
  for (size_t i = 0; i < subLibraries.size(); ++i)
  {
    juce::Component* res = subLibraries[i]->createUIComponentIfExists(context, object, name);
    if (res)
      return res;
  }
  return NULL;
}
#endif // LBCPP_USER_INTERFACE

void Library::getTypesInheritingFrom(TypePtr baseType, std::vector<TypePtr>& res) const
{
  for (size_t i = 0; i < types.size(); ++i)
    if (types[i]->inheritsFrom(baseType))
      res.push_back(types[i]);

  for (size_t i = 0; i < subLibraries.size(); ++i)
    subLibraries[i]->getTypesInheritingFrom(baseType, res);
}

std::vector<TypePtr> Library::getTypesInheritingFrom(TypePtr baseType) const
{
  std::vector<TypePtr> res;
  getTypesInheritingFrom(baseType, res);
  return res;
}

void Library::luaRegister(LuaState& state) const
{
  for (size_t i = 0; i < types.size(); ++i)
    types[i]->luaRegister(state);
  for (size_t i = 0; i < subLibraries.size(); ++i)
    subLibraries[i]->luaRegister(state);
}

/*
** Global
*/
namespace lbcpp
{
  TypePtr getType(const String& typeName)
    {return typeManager().getType(defaultExecutionContext(), typeName);}

  TypePtr getType(const String& name, const std::vector<TypePtr>& arguments)
    {return typeManager().getType(defaultExecutionContext(), name, arguments);}

  bool doTypeExists(const String& typeName)
    {return typeManager().doTypeExists(typeName);}

  bool declareType(TypePtr type)
    {return typeManager().declare(defaultExecutionContext(), type);}

};

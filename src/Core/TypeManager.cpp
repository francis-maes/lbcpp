/*-----------------------------------------.---------------------------------.
| Filename: TypeManager.cpp                | Global Type Manager             |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Type.h>
#include <lbcpp/Core/TemplateType.h>
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

  void clear()
  {
    ScopedLock _(instancesLock);
    for (InstanceMap::iterator it = instances.begin(); it != instances.end(); ++it)
      delete it->second;
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
    return res;
  }
};

}; /* namespace lbcpp */

/*
** TypeManager
*/
TypeManager::TypeManager()
  : standardTypesAreDeclared(false)
{
}

TypeManager::~TypeManager()
  {shutdown();}

void TypeManager::declare(ExecutionContext& context, TypePtr type)
{
  if (!type || type->getName().isEmpty())
  {
    context.errorCallback(T("TypeManager::declare"), T("Empty class name"));
    return;
  }
  if (type->isUnnamedType())
  {
    context.errorCallback(T("TypeManager::declare"), T("Trying to declare an unnamed type"));
    return;
  }

  ScopedLock _(typesLock);
  String typeName = type->getName();
  if (doTypeExists(typeName))
  {
    context.errorCallback(T("TypeManager::declare"), T("Type '") + typeName + T("' has already been declared"));
    return;
  }
  type->setStaticAllocationFlag();
  types[typeName] = type;
}

void TypeManager::declare(ExecutionContext& context, TemplateTypePtr templateType)
{
  if (!templateType || templateType->getName().isEmpty())
    context.errorCallback(T("TypeManager::declare"), T("Empty template class name"));

  ScopedLock _(typesLock);
  String typeName = templateType->getName();
  TemplateTypeMap::const_iterator it = templateTypes.find(typeName);
  if (it != templateTypes.end())
  {
    context.errorCallback(T("TypeManager::declare"), T("Template type '") + typeName + T("' has already been declared"));
    return;
  }
  templateTypes[typeName].definition = templateType;
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
  jassert(arguments.size());

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

  if (!TemplateType::isInstanciatedTypeName(typeName))
  {
    context.errorCallback(T("TypeManager::getType()"), T("Could not find type ") + typeName);
    return TypePtr();
  }

  String templateName;
  std::vector<TypePtr> templateArguments;
  if (!TemplateType::parseInstanciatedTypeName(context, typeName, templateName, templateArguments))
    return TypePtr();
  return getType(context, templateName, templateArguments);
}

String TypeManager::removeAllSpaces(const String& str)
{
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

extern void declareCoreClasses(ExecutionContext& context);
extern void declareLBCppClasses(ExecutionContext& context);

void TypeManager::ensureIsInitialized(ExecutionContext& context)
{
  if (!standardTypesAreDeclared)
  {
    standardTypesAreDeclared = true;
    declareCoreClasses(context);
    finishDeclarations(context);
    declareLBCppClasses(context);
    topLevelType = anyType = variableType;
  }
}

void TypeManager::shutdown()
{
  ScopedLock _(typesLock);
  for (TemplateTypeMap::iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
    it->second.clear();
  templateTypes.clear();
  for (TypeMap::iterator it = types.begin(); it != types.end(); ++it)
  {
    it->second->deinitialize();
    delete it->second;
  }
  types.clear();
}


TemplateTypeCache* TypeManager::getTemplateType(ExecutionContext& context, const String& templateTypeName) const
{
  String name = templateTypeName;
  if (name == T("DiscreteProbabilityDistribution"))
    name = T("EnumerationProbabilityDistribution");
  ScopedLock _(typesLock);
  TemplateTypeMap::iterator it = const_cast<TypeManager* >(this)->templateTypes.find(name);
  if (it == templateTypes.end())
  {
    context.errorCallback(T("TypeManager::getTemplateType()"), T("Could not find template type ") + name);
    return NULL;
  }
  return &it->second;
}

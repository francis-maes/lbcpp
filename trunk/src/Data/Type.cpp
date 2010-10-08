/*-----------------------------------------.---------------------------------.
| Filename: Type.cpp                       | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Type.h>
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/Vector.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include <map>
using namespace lbcpp;

extern void declareLBCppClasses();

/*
** TypeManager
*/
namespace lbcpp {

class TypeManager
{
public:
  TypeManager() : standardTypesAreDeclared(false) {}

  void declare(TypePtr type)
  {
    if (!type || type->getName().isEmpty())
      MessageCallback::error(T("TypeManager::declare"), T("Empty class name"));

    ScopedLock _(typesLock);
    String typeName = type->getName();
    if (doTypeExists(typeName))
    {
      MessageCallback::error(T("TypeManager::declare"), T("Type '") + typeName + T("' has already been declared"));
      return;
    }
    types[typeName] = type;
  }

  void declare(TemplateTypePtr templateType)
  {
    if (!templateType || templateType->getName().isEmpty())
      MessageCallback::error(T("TypeManager::declare"), T("Empty template class name"));

    ScopedLock _(typesLock);
    String typeName = templateType->getName();
    TemplateTypeMap::const_iterator it = templateTypes.find(typeName);
    if (it != templateTypes.end())
    {
      MessageCallback::error(T("TypeManager::declare"), T("Template type '") + typeName + T("' has already been declared"));
      return;
    }
    templateTypes[typeName].definition = templateType;
  }

  void finishDeclarations(MessageCallback& callback)
  {
    ScopedLock _(typesLock);
    for (TypeMap::const_iterator it = types.begin(); it != types.end(); ++it)
    {
      String name = it->first;
      TypePtr type = it->second;
      if (!type->isInitialized() && !type->initialize(callback))
        callback.errorMessage(T("TypeManager::finishDeclarations()"), T("Could not initialize type ") + type->getName());
    }

    for (TemplateTypeMap::const_iterator it = templateTypes.begin(); it != templateTypes.end(); ++it)
    {
      String name = it->first;
      TemplateTypePtr templateType = it->second.definition;
      if (!templateType->isInitialized() && !templateType->initialize(callback))
        callback.errorMessage(T("TypeManager::finishDeclarations()"), T("Could not initialize template type ") + templateType->getName());
    }
  }

  TypePtr getType(const String& name, const std::vector<TypePtr>& arguments, MessageCallback& callback) const
  {
    jassert(arguments.size());

    String typeName = removeAllSpaces(name);
    if (typeName.isEmpty())
    {
      callback.errorMessage(T("TypeManager::getType"), T("Empty type name"));
      return TypePtr();
    }
    jassert(!TemplateType::isInstanciatedTypeName(typeName));

    ScopedLock _(typesLock);
    TemplateTypeCache* templateType = getTemplateType(typeName, callback);
    return templateType ? templateType->getInstanceCached(arguments, callback) : TypePtr();
  }

  TypePtr getType(const String& name, MessageCallback& callback) const
  {
    String typeName = removeAllSpaces(name);
    if (typeName.isEmpty())
    {
      callback.errorMessage(T("TypeManager::getType"), T("Empty type name"));
      return TypePtr();
    }

    ScopedLock _(typesLock);
    TypePtr type = findType(typeName);
    if (type)
      return type;

    if (!TemplateType::isInstanciatedTypeName(typeName))
    {
      callback.errorMessage(T("TypeManager::getType()"), T("Could not find type ") + typeName);
      return TypePtr();
    }

    String templateName;
    std::vector<TypePtr> templateArguments;
    if (!TemplateType::parseInstanciatedTypeName(typeName, templateName, templateArguments, callback))
      return TypePtr();
    return getType(templateName, templateArguments, callback);
  }

  static String removeAllSpaces(const String& str)
  {
    String res;
    for (int i = 0; i < str.length(); ++i)
      if (str[i] != ' ' && str[i] != '\t')
        res += str[i];
    return res;
  }

  TypePtr findType(const String& name) const
  {
    ScopedLock _(typesLock);
    TypeMap::const_iterator it = types.find(removeAllSpaces(name));
    return it == types.end() ? TypePtr() : it->second;
  }

  bool doTypeExists(const String& type) const
    {return findType(type);}

  void ensureStandardClassesAreLoaded()
  {
    if (!standardTypesAreDeclared)
    {
      standardTypesAreDeclared = true;
      declareLBCppClasses();
    }
  }

  void clear()
  {
    ScopedLock _(typesLock);
    for (TypeMap::iterator it = types.begin(); it != types.end(); ++it)
      it->second->deinitialize();
    types.clear();
    templateTypes.clear();
  }

private:
  typedef std::map<String, TypePtr> TypeMap;

  struct TemplateTypeCache
  {
    TemplateTypeCache() {}
    TemplateTypeCache(const TemplateTypeCache& other) : definition(other.definition)
    {
      ScopedLock _(other.instancesLock);
      instances = other.instances;
    }

    TemplateTypePtr definition;

    TypePtr getInstanceCached(const std::vector<TypePtr>& arguments, MessageCallback& callback)
    {
      ScopedLock _(instancesLock);
      TemplateTypeCache::InstanceMap::iterator it = instances.find(arguments);
      if (it != instances.end())
        return it->second;
      else
        return instantiate(arguments, callback);
    }

  private:
    CriticalSection instancesLock;
    typedef std::map< std::vector<TypePtr>, TypePtr > InstanceMap;
    InstanceMap instances;

    TypePtr instantiate(const std::vector<TypePtr>& arguments, MessageCallback& callback)
    {
      TypePtr res = definition->instantiate(arguments, callback);
      if (!res || !res->initialize(callback))
        return TypePtr();
      instances[arguments] = res;
      return res;
    }
  };

  typedef std::map<String, TemplateTypeCache> TemplateTypeMap;

  CriticalSection typesLock;
  TypeMap types;
  TemplateTypeMap templateTypes;
 
  bool standardTypesAreDeclared;

  TemplateTypeCache* getTemplateType(const String& templateTypeName, MessageCallback& callback) const
  {
    ScopedLock _(typesLock);
    TemplateTypeMap::iterator it = const_cast<TypeManager* >(this)->templateTypes.find(templateTypeName);
    if (it == templateTypes.end())
    {
      callback.errorMessage(T("TypeManager::getTemplateType()"), T("Could not find template type ") + templateTypeName);
      return NULL;
    }
    return &it->second;
  }
};

}; /* namespace lbcpp */

inline TypeManager& getTypeManagerInstance()
{
  static TypeManager instance;
  return instance;
}

TypePtr lbcpp::topLevelType;
TypePtr lbcpp::anyType;

void lbcpp::initialize()
{
  getTypeManagerInstance().ensureStandardClassesAreLoaded();
  topLevelType = anyType = variableType;
}

void lbcpp::deinitialize()
  {getTypeManagerInstance().clear();}

void Type::declare(TypePtr typeInstance)
  {getTypeManagerInstance().declare(typeInstance);}

void Type::declare(TemplateTypePtr templateTypeInstance)
  {getTypeManagerInstance().declare(templateTypeInstance);}

void Type::finishDeclarations(MessageCallback& callback)
  {getTypeManagerInstance().finishDeclarations(callback);}

TypePtr Type::get(const String& typeName, MessageCallback& callback)
  {return getTypeManagerInstance().getType(typeName, callback);}

TypePtr Type::get(const String& name, const std::vector<TypePtr>& arguments, MessageCallback& callback)
  {return getTypeManagerInstance().getType(name, arguments, callback);}

bool Type::doTypeExists(const String& typeName)
  {return getTypeManagerInstance().doTypeExists(typeName);}

/*
** Type
*/
Type::Type(const String& className, TypePtr baseType)
  : NameableObject(className), initialized(false), baseType(baseType) {}

Type::Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType)
  : NameableObject(templateType->makeTypeName(templateArguments)), initialized(false),
      baseType(baseType), templateType(templateType), templateArguments(templateArguments)
 {}

bool Type::initialize(MessageCallback& callback)
  {return (initialized = true);}

void Type::deinitialize()
{
  baseType = TypePtr();
  templateType = TemplateTypePtr();
  templateArguments.clear();
  initialized = false;
}

ClassPtr Type::getClass() const
  {return typeClass;}

bool Type::inheritsFrom(TypePtr baseType) const
{
  jassert(this && baseType.get());

  if (this == baseType.get())
    return true;

  if (!this->baseType)
    return false;

  if (this->templateType && this->templateType == baseType->templateType)
  {
    jassert(this->templateArguments.size() == baseType->templateArguments.size());
    for (size_t i = 0; i < this->templateArguments.size(); ++i)
      if (!this->templateArguments[i]->inheritsFrom(baseType->templateArguments[i]))
        return false;
    return true;
  }

  return this->baseType->inheritsFrom(baseType);
}

bool Type::canBeCastedTo(TypePtr targetType) const
  {return inheritsFrom(targetType);}

VariableValue Type::getMissingValue() const
{
  jassert(sizeof (VariableValue) == 8);
  static const juce::int64 missing = 0x0FEEFEEEFEEEFEEELL;
  return VariableValue(missing);
}

bool Type::isMissingValue(const VariableValue& value) const
{
  VariableValue missing = getMissingValue();
  return value.getInteger() == missing.getInteger();
}

VariableValue Type::create() const
  {jassert(baseType); return baseType->create();}

VariableValue Type::createFromString(const String& value, MessageCallback& callback) const
  {jassert(baseType); return baseType->createFromString(value, callback);}

VariableValue Type::createFromXml(XmlImporter& importer) const
  {jassert(baseType); return baseType->createFromXml(importer);}

String Type::toString(const VariableValue& value) const
  {jassert(baseType); return baseType->toString(value);}

void Type::destroy(VariableValue& value) const
  {jassert(baseType); baseType->destroy(value);}

void Type::copy(VariableValue& dest, const VariableValue& source) const
  {jassert(baseType); baseType->copy(dest, source);}

int Type::compare(const VariableValue& value1, const VariableValue& value2) const
  {jassert(baseType); return baseType->compare(value1, value2);}

void Type::saveToXml(XmlExporter& exporter, const VariableValue& value) const
  {jassert(baseType); return baseType->saveToXml(exporter, value);}

size_t Type::getObjectNumVariables() const
  {jassert(baseType); return baseType->getObjectNumVariables();}

TypePtr Type::getObjectVariableType(size_t index) const
  {jassert(baseType); return baseType->getObjectVariableType(index);}

String Type::getObjectVariableName(size_t index) const
  {jassert(baseType); return baseType->getObjectVariableName(index);}

int Type::findObjectVariable(const String& name) const
  {jassert(baseType); return baseType->findObjectVariable(name);}
  
Variable Type::getObjectVariable(const VariableValue& value, size_t index) const
  {jassert(baseType); return baseType->getObjectVariable(value, index);}

void Type::setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const
  {if (baseType) baseType->setObjectVariable(value, index, subValue);}

size_t Type::getNumElements(const VariableValue& value) const
  {jassert(baseType); return baseType->getNumElements(value);}

Variable Type::getElement(const VariableValue& value, size_t index) const
  {jassert(baseType); return baseType->getElement(value, index);}

String Type::getElementName(const VariableValue& value, size_t index) const
  {jassert(baseType); return baseType->getElementName(value, index);}

size_t Type::getNumTemplateArguments() const
  {return templateArguments.size();}

TypePtr Type::getTemplateArgument(size_t index) const
  {jassert(index < templateArguments.size()); return templateArguments[index];}

TypePtr Type::findCommonBaseType(TypePtr type1, TypePtr type2)
{
  if (type1->inheritsFrom(type2))
    return type2;
  if (type2->inheritsFrom(type1))
    return type1;
  if (type1 == topLevelType || type2 == topLevelType)
    return topLevelType;
  TypePtr baseType1 = type1->getBaseType();
  TypePtr baseType2 = type2->getBaseType();
  jassert(baseType1 && baseType2);
  baseType1 = findCommonBaseType(baseType1, type2);
  baseType2 = findCommonBaseType(type1, baseType2);
  return baseType1->inheritsFrom(baseType2) ? baseType1 : baseType2;
}

void Type::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(getName());}

/*
** TypeCache
*/
UnaryTemplateTypeCache::UnaryTemplateTypeCache(const String& typeName)
{
  ScopedLock _(lock);
  this->typeName = typeName;
}

TypePtr UnaryTemplateTypeCache::operator ()(TypePtr argument)
{
  jassert(argument);

  TypePtr res;
  {
    ScopedLock _(lock);
    std::map<Type*, Type*>::const_iterator it = m.find(argument.get());
    if (it == m.end())
    {
      res = getTypeManagerInstance().getType(typeName, std::vector<TypePtr>(1, argument), MessageCallback::getInstance());
      jassert(res);
      m[argument.get()] = res.get();
    }
    else
      res = it->second;
  }
  return res;
}

BinaryTemplateTypeCache::BinaryTemplateTypeCache(const String& typeName)
{
  ScopedLock _(lock);
  this->typeName = typeName;
}

TypePtr BinaryTemplateTypeCache::operator ()(TypePtr argument1, TypePtr argument2)
{
  jassert(argument1 && argument2);
  std::pair<Type*, Type*> key(argument1.get(), argument2.get());

  TypePtr res;
  //double timeBegin = Time::getMillisecondCounterHiRes();
  {
    ScopedLock _(lock);
    std::map<std::pair<Type*, Type*>, Type*>::const_iterator it = m.find(key);
    if (it == m.end())
    {
      std::vector<TypePtr> arguments(2);
      arguments[0] = argument1;
      arguments[1] = argument2;

      res = getTypeManagerInstance().getType(typeName, arguments, MessageCallback::getInstance());
      jassert(res);
      m[std::make_pair(argument1.get(), argument2.get())] = res.get();
    }
    else
      res = it->second;
  }
/*
  double deltaTime = Time::getMillisecondCounterHiRes() - timeBegin;
  {
    static double sum = 0.0;
    static int count = 0;
    sum += deltaTime;
    ++count;
    if (count == 20)
    {
      MessageCallback::info(T("UnaryTemplateTypeCache length: ") + String(sum / count));
      sum = 0.0;
      count = 0;
    }
  }*/

  return res;
}

#include "Type/FileType.h"

DirectoriesCache FileType::cache;

TypePtr lbcpp::sumType(TypePtr type1, TypePtr type2)
{
  std::vector<TypePtr> types(2);
  types[0] = type1;
  types[1] = type2;
  return sumType(types);
}

TypePtr lbcpp::sumType(TypePtr type1, TypePtr type2, TypePtr type3)
{
  std::vector<TypePtr> types(3);
  types[0] = type1;
  types[1] = type2;
  types[2] = type3;
  return sumType(types);
}

TypePtr lbcpp::sumType(TypePtr type1, TypePtr type2, TypePtr type3, TypePtr type4)
{
  std::vector<TypePtr> types(4);
  types[0] = type1;
  types[1] = type2;
  types[2] = type3;
  types[3] = type4;
  return sumType(types);
}

TypePtr lbcpp::sumType(const std::vector<TypePtr>& types)
{
  // TODO !
  return topLevelType;
}

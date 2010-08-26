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
      Object::error(T("TypeManager::declare"), T("Empty class name"));

    ScopedLock _(typesLock);
    String typeName = type->getName();
    if (doTypeExists(typeName))
    {
      Object::error(T("TypeManager::declare"), T("Type '") + typeName + T("' has already been declared"));
      return;
    }
    types[typeName] = type;
  }

  void declare(TemplateTypePtr templateType)
  {
    if (!templateType || templateType->getName().isEmpty())
      Object::error(T("TypeManager::declare"), T("Empty template class name"));

    ScopedLock _(typesLock);
    String typeName = templateType->getName();
    TemplateTypeMap::const_iterator it = templateTypes.find(typeName);
    if (it != templateTypes.end())
    {
      Object::error(T("TypeManager::declare"), T("Template type '") + typeName + T("' has already been declared"));
      return;
    }
    templateTypes[typeName] = templateType;
  }

  TemplateTypePtr getTemplateType(const String& templateTypeName, ErrorHandler& callback) const
  {
    ScopedLock _(typesLock);
    TemplateTypeMap::const_iterator it = templateTypes.find(templateTypeName);
    if (it == templateTypes.end())
    {
      callback.errorMessage(T("TypeManager::getTemplateType()"), T("Could not find template type ") + templateTypeName);
      return TemplateTypePtr();
    }
    return it->second;
  }

  void finishDeclarations(ErrorHandler& callback)
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
      TemplateTypePtr templateType = it->second;
      if (!templateType->isInitialized() && !templateType->initialize(callback))
        callback.errorMessage(T("TypeManager::finishDeclarations()"), T("Could not initialize template type ") + templateType->getName());
    }
  }

  TypePtr getType(const String& name, ErrorHandler& callback) const
  {
    String typeName = removeAllSpaces(name);
    ScopedLock _(typesLock);
    TypePtr type = findType(typeName);
    if (type)
      return type;

    if (TemplateType::isInstanciatedTypeName(typeName))
    {
      String templateName;
      std::vector<TypePtr> templateArguments;
      if (!TemplateType::parseInstanciatedTypeName(typeName, templateName, templateArguments, callback))
        return TypePtr();

      TemplateTypePtr templateType = getTemplateType(templateName, callback);
      if (!templateType)
        return TypePtr();
      
      type = templateType->instantiate(templateArguments, callback);
      if (type)
      {
        if (!type->initialize(callback))
          return TypePtr();
        const_cast<TypeManager* >(this)->types[typeName] = type;
      }
    }
    else
      callback.errorMessage(T("TypeManager::getType()"), T("Could not find type ") + typeName);

    return type;
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
  typedef std::map<String, TemplateTypePtr> TemplateTypeMap;

  CriticalSection typesLock;
  TypeMap types;
  TemplateTypeMap templateTypes;
 
  bool standardTypesAreDeclared;
};

}; /* namespace lbcpp */

inline TypeManager& getTypeManagerInstance()
{
  static TypeManager instance;
  return instance;
}

void lbcpp::initialize()
  {getTypeManagerInstance().ensureStandardClassesAreLoaded();}

void lbcpp::deinitialize()
  {getTypeManagerInstance().clear();}

void Type::declare(TypePtr typeInstance)
  {getTypeManagerInstance().declare(typeInstance);}

void Type::declare(TemplateTypePtr templateTypeInstance)
  {getTypeManagerInstance().declare(templateTypeInstance);}

void Type::finishDeclarations(ErrorHandler& callback)
  {getTypeManagerInstance().finishDeclarations(callback);}

TypePtr Type::get(const String& typeName, ErrorHandler& callback)
  {return getTypeManagerInstance().getType(typeName, callback);}

bool Type::doTypeExists(const String& typeName)
  {return getTypeManagerInstance().doTypeExists(typeName);}

/*
** Type
*/
Type::Type(const String& className, TypePtr baseType)
  : NameableObject(className), initialized(false), baseType(baseType) {}

Type::Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType)
  : NameableObject(templateType->makeTypeName(templateArguments)), initialized(false),
      templateType(templateType), templateArguments(templateArguments), baseType(baseType)
 {}

bool Type::initialize(ErrorHandler& callback)
  {return (initialized = true);}

void Type::deinitialize()
{
  baseType = TypePtr();
  templateType = TemplateTypePtr();
  templateArguments.clear();
  initialized = false;
}

ClassPtr Type::getClass() const
  {return typeClass();}

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

VariableValue Type::createFromString(const String& value, ErrorHandler& callback) const
  {jassert(baseType); return baseType->createFromString(value, callback);}

VariableValue Type::createFromXml(XmlElement* xml, ErrorHandler& callback) const
  {jassert(baseType); return baseType->createFromXml(xml, callback);}

String Type::toString(const VariableValue& value) const
  {jassert(baseType); return baseType->toString(value);}

void Type::destroy(VariableValue& value) const
  {jassert(baseType); baseType->destroy(value);}

void Type::copy(VariableValue& dest, const VariableValue& source) const
  {jassert(baseType); baseType->copy(dest, source);}

int Type::compare(const VariableValue& value1, const VariableValue& value2) const
  {jassert(baseType); return baseType->compare(value1, value2);}

void Type::saveToXml(XmlElement* xml, const VariableValue& value) const
  {jassert(baseType); return baseType->saveToXml(xml, value);}

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

/*
** TypeCache
*/
TypeCache::TypeCache(const String& typeName)
{
  type = Type::get(typeName).get();
  if (!type)
    Object::error(T("TypeCache()"), T("Could not find type ") + typeName.quoted());
}

TypePtr UnaryTemplateTypeCache::operator ()(TypePtr argument)
{
  jassert(argument);
  std::map<Type*, Type*>::const_iterator it = m.find(argument.get());
  if (it == m.end())
  {
    TypePtr res = Type::get(typeName + T("<") + argument->getName() + T(">"));
    m[argument.get()] = res.get();
    return res;
  }
  else
    return it->second;
}

TypePtr BinaryTemplateTypeCache::operator ()(TypePtr argument1, TypePtr argument2)
{
  jassert(argument1 && argument2);
  std::pair<Type*, Type*> key(argument1.get(), argument2.get());
  std::map<std::pair<Type*, Type*>, Type*>::const_iterator it = m.find(key);
  if (it == m.end())
  {
    TypePtr res = Type::get(typeName + T("<") + argument1->getName() + T(", ") + argument2->getName() + T(">"));
    m[key] = res.get();
    return res;
  }
  else
    return it->second;
}

#include "Type/TupleType.h"
#include "Type/FileType.h"

DirectoriesCache FileType::cache;

TypePtr lbcpp::pairType(TypePtr firstClass, TypePtr secondClass)
  {static BinaryTemplateTypeCache cache(T("Pair")); return cache(firstClass, secondClass);}

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
  return topLevelType();
}

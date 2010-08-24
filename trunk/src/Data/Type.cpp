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

extern void declareLBCppCoreClasses();

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
      declareLBCppCoreClasses();
    }
  }

  void clear()
  {
    ScopedLock _(typesLock);
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

TypePtr Class::multiplyByScalar(VariableValue& value, double scalar)
{
  ObjectPtr object = value.getObject();
  if (object && scalar != 1.0)
  {
    object = object->multiplyByScalar(scalar);
    value.clearObject();
    value.setObject(object);
    return object->getClass();
  }
  else
    return TypePtr(this);
}

TypePtr Class::addWeighted(VariableValue& target, const Variable& source, double weight)
{
  ObjectPtr object = target.getObject();
  if (object && weight != 0.0)
  {
    object = object->addWeighted(source, weight);
    target.clearObject();
    target.setObject(object);
    return object->getClass();
  }
  else
    return TypePtr(this);
}

VariableValue Class::createFromString(const String& value, ErrorHandler& callback) const
{
  VariableValue res = create();
  if (isMissingValue(res))
  {
    callback.errorMessage(T("Class::createFromString"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  res.getObject()->thisClass = refCountedPointerFromThis(this);
  return res.getObject()->loadFromString(value, callback) ? res : getMissingValue();
}

VariableValue Class::createFromXml(XmlElement* xml, ErrorHandler& callback) const
{
  VariableValue res = create();
  if (isMissingValue(res))
  {
    callback.errorMessage(T("Class::createFromXml"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  res.getObject()->thisClass = refCountedPointerFromThis(this);
  return res.getObject()->loadFromXml(xml, callback) ? res : getMissingValue();
}

void Class::saveToXml(XmlElement* xml, const VariableValue& value) const
{
  ObjectPtr object = value.getObject();
  jassert(object);
  object->saveToXml(xml);
}

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

void DefaultClass::addVariable(const String& typeName, const String& name)
{
  TypePtr type = Type::get(typeName);
  if (type)
    addVariable(type, name);
}

size_t DefaultClass::getObjectNumVariables() const
{
  size_t n = baseType->getObjectNumVariables();
  ScopedLock _(variablesLock);
  return n + variables.size();
}

TypePtr DefaultClass::getObjectVariableType(size_t index) const
{
  size_t n = baseType->getObjectNumVariables();
  if (index < n)
    return baseType->getObjectVariableType(index);
  index -= n;
  
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].first;
}

String DefaultClass::getObjectVariableName(size_t index) const
{
  size_t n = baseType->getObjectNumVariables();
  if (index < n)
    return baseType->getObjectVariableName(index);
  index -= n;
  
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].second;
}

void DefaultClass::addVariable(TypePtr type, const String& name)
{
  if (!type || name.isEmpty())
  {
    Object::error(T("Class::addVariable"), T("Invalid type or name"));
    return;
  }
  ScopedLock _(variablesLock);
  if (findObjectVariable(name) >= 0)
    Object::error(T("Class::addVariable"), T("Another variable with name '") + name + T("' already exists"));
  else
    variables.push_back(std::make_pair(type, name));
}

int DefaultClass::findObjectVariable(const String& name) const
{
  ScopedLock _(variablesLock);
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i].second == name)
      return (int)(i + baseType->getObjectNumVariables());
  return baseType->findObjectVariable(name);
}

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes)
  : Type(name, enumValueType()), oneLetterCodes(oneLetterCodes)
{
  jassert(!oneLetterCodes.containsChar('_')); // '_' is reserved to denote missing values
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& oneLetterCodes)
  : Type(name, enumValueType()), oneLetterCodes(oneLetterCodes)
{
  jassert(!oneLetterCodes.containsChar('_'));
  for (int i = 0; i < oneLetterCodes.length(); ++i)
  {
    String str;
    str += oneLetterCodes[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : Type(name, enumValueType())
{
}

ClassPtr Enumeration::getClass() const
  {return enumerationClass();}

void Enumeration::addElement(const String& elementName, const String& oneLetterCode, const String& threeLettersCode)
{
  if (findElement(elementName) >= 0)
  {
    Object::error(T("Enumeration::addElement"), T("Element '") + elementName + T("' already exists"));
    return;
  }
  elements.push_back(elementName);
  if (oneLetterCode.length() == 1)
    oneLetterCodes += oneLetterCode;

  // FIXME: store three-letters code
}

int Enumeration::findElement(const String& name) const
{
  for (size_t i = 0; i < elements.size(); ++i)
    if (elements[i] == name)
      return (int)i;
  return -1;
}

VariableValue Enumeration::create() const
  {return getMissingValue();}

VariableValue Enumeration::getMissingValue() const
  {return VariableValue((juce::int64)getNumElements());}

VariableValue Enumeration::createFromXml(XmlElement* xml, ErrorHandler& callback) const
  {return createFromString(xml->getAllSubText(), callback);}
 
VariableValue Enumeration::createFromString(const String& value, ErrorHandler& callback) const
{
  String str = value.trim();
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (str == getElementName(i))
      return VariableValue(i);
  callback.errorMessage(T("Enumeration::createFromString"), T("Could not find enumeration value ") + value.quoted());
  return getMissingValue();
}

String Enumeration::toString(const VariableValue& value) const
{
  juce::int64 val = value.getInteger();
  return val >= 0 && (size_t)val < getNumElements() ? getElementName((size_t)val) : T("Nil");
}

void Enumeration::saveToXml(XmlElement* xml, const VariableValue& value) const
  {xml->addTextElement(toString(value));}

#include <lbcpp/Data/ProbabilityDistribution.h>

TypePtr Enumeration::multiplyByScalar(VariableValue& value, double scalar)
{
  DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(EnumerationPtr(this));
  distribution->setVariable((size_t)value.getInteger(), scalar);
  value.clearBuiltin();
  value.setObject(distribution);
  return distribution->getClass();
}

bool Enumeration::hasOneLetterCodes() const
  {return oneLetterCodes.length() == (int)elements.size();}

juce::tchar Enumeration::getOneLetterCode(size_t index) const
{
  jassert(index < elements.size());
  if (oneLetterCodes.length())
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes[index];
  }
  else
  {
    jassert(elements[index].isNotEmpty());
    return elements[index][0];
  }
}

String Enumeration::getOneLetterCodes() const
{
  if (oneLetterCodes.isEmpty())
  {
    String res;
    for (size_t i = 0; i < elements.size(); ++i)
      res += getOneLetterCode(i);
    return res;
  }
  else
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes;
  }
}

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

/*
** Type Declarations
*/
#include "Type/TopLevelType.h"
#include "Type/NilType.h"
#include "Type/BooleanType.h"
#include "Type/IntegerType.h"
#include "Type/DoubleType.h"
#include "Type/StringType.h"
#include "Type/FileType.h"
#include "Type/TupleType.h"

DirectoriesCache FileType::cache;

#define DECLARE_CLASS_SINGLETON_ACCESSOR(AccessorName, ClassName) \
  TypePtr lbcpp::AccessorName() { static TypeCache cache(ClassName); return cache(); }

DECLARE_CLASS_SINGLETON_ACCESSOR(topLevelType, T("Variable"));
DECLARE_CLASS_SINGLETON_ACCESSOR(nilType, T("Nil"));

DECLARE_CLASS_SINGLETON_ACCESSOR(booleanType, T("Boolean"));
DECLARE_CLASS_SINGLETON_ACCESSOR(integerType, T("Integer"));
DECLARE_CLASS_SINGLETON_ACCESSOR(doubleType, T("Double"));
  DECLARE_CLASS_SINGLETON_ACCESSOR(probabilityType, T("Probability"));
  DECLARE_CLASS_SINGLETON_ACCESSOR(angstromDistanceType, T("AngstromDistance"));

DECLARE_CLASS_SINGLETON_ACCESSOR(stringType, T("String"));
DECLARE_CLASS_SINGLETON_ACCESSOR(fileType, T("File"));

DECLARE_CLASS_SINGLETON_ACCESSOR(enumValueType, T("EnumValue"));

ClassPtr lbcpp::objectClass()
  {static TypeCache cache(T("Object")); return cache();}

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

void declareClassClasses()
{
  Type::declare(new TopLevelType());
    Type::declare(new NilType());

  Type::declare(new BooleanType());
  Type::declare(new IntegerType());
    Type::declare(new IntegerType(T("EnumValue"), integerType()));
  Type::declare(new DoubleType());
    Type::declare(new ProbabilityType());
    Type::declare(new AngstromDistanceType());

  Type::declare(new StringType());
    Type::declare(new FileType());

  Type::declare(new PairTemplateType());

  Type::declare(new Class());
}

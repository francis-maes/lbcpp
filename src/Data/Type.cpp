/*-----------------------------------------.---------------------------------.
| Filename: Type.cpp                       | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Type.h>
#include <lbcpp/Data/Variable.h>
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
    TypeMap::const_iterator it = types.find(typeName);
    if (it != types.end())
    {
      Object::error(T("TypeManager::declare"), T("Type '") + typeName + T("' has already been declared"));
      return;
    }
    types[typeName] = type;
  }

  TypePtr get(const String& typeName) const
  {
    ScopedLock _(typesLock);
    TypeMap::const_iterator it = types.find(typeName);
    return it == types.end() ? TypePtr() : it->second;
  }

  TypePtr get(const String& typeName, TypePtr argument1, TypePtr argument2 = TypePtr(), TypePtr argument3 = TypePtr()) const
  {
    std::vector<TypePtr> arguments(1, argument1);
    if (argument2) arguments.push_back(argument2);
    if (argument3) arguments.push_back(argument3);
    return get(typeName, arguments);
  }

  TypePtr get(const String& typeName, const std::vector<TypePtr>& arguments) const
  {
    TemplateTypeKey key(typeName, arguments);
    
    ScopedLock _(typesLock);
    TemplateTypeMap::const_iterator it = templateTypes.find(key);
    if (it != templateTypes.end())
      return it->second;

    TypePtr type = get(typeName);
    if (!type)
    {
      Object::error(T("TypeManager::get"), T("Could not find type '") + typeName + T("'"));
      return TypePtr();
    }
    TypePtr specializedType = type->cloneAndCast<Type>();
    if (!specializedType)
    {
      Object::error(T("TypeManager::get"), T("Could not specialize template type '") + typeName + T("'"));
      return TypePtr();
    }
    for (size_t i = 0; i < key.second.size(); ++i)
      specializedType->setTemplateArgument(i, key.second[i]);
    specializedType->setBaseType(type);
    specializedType->setName(makeTemplateClassName(typeName, key.second));
    const_cast<TypeManager* >(this)->templateTypes[key] = specializedType;
    return specializedType;
  }

  TypePtr parseAndGet(const String& typeName, ErrorHandler& callback) const
  {
    int b = typeName.indexOfChar('<');
    int e = typeName.lastIndexOfChar('>');
    if ((b >= 0) != (e >= 0))
    {
      callback.errorMessage(T("TypeManager::parseAndGet"), T("Invalid type syntax: ") + typeName.quoted());
      return TypePtr();
    }
    if (b < 0 && e < 0)
      return get(typeName);
    String baseName = typeName.substring(0, b);
    String arguments = typeName.substring(b + 1, e);
    StringArray tokens;
    tokens.addTokens(arguments, T(","), T("<>"));
    std::vector<TypePtr> typeArguments(tokens.size());
    for (int i = 0; i < tokens.size(); ++i)
    {
      typeArguments[i] = parseAndGet(tokens[i].trim(), callback);
      if (!typeArguments[i])
        return TypePtr();
    } 
    return get(baseName, typeArguments);
  }

  Variable createInstance(const String& typeName) const
  {
    if (typeName.isEmpty())
    {
      Object::error(T("TypeManager::create"), T("Empty type name"));
      return Variable();
    }

    TypePtr type = get(typeName);
    if (!type)
    {
      Object::error(T("TypeManager::create"), T("Could not find type '") + typeName + T("'"));
      return Variable();
    }
    return Variable::create(type);
  }

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
  typedef std::pair<String, std::vector<TypePtr> > TemplateTypeKey;
  typedef std::map<TemplateTypeKey, TypePtr> TemplateTypeMap;

  CriticalSection typesLock;
  TypeMap types;
  TemplateTypeMap templateTypes;

  bool standardTypesAreDeclared;

  static String makeTemplateClassName(const String& typeName, const std::vector<TypePtr>& arguments)
  {
    jassert(arguments.size());
    String res;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      if (i == 0)
        res = typeName + T("<");
      else
        res += T(", ");
      res += arguments[i] ? arguments[i]->getName() : T("ERROR");
    }
    res += T(">");
    return res;
  }
};

}; /* namespace lbcpp */

inline TypeManager& getClassManagerInstance()
{
  static TypeManager instance;
  return instance;
}

void lbcpp::initialize()
  {getClassManagerInstance().ensureStandardClassesAreLoaded();}

void lbcpp::deinitialize()
  {getClassManagerInstance().clear();}

/*
** Type
*/
ClassPtr Type::getClass() const
  {return typeClass();}

bool Type::inheritsFrom(TypePtr baseType) const
{
  jassert(this && baseType.get());
  if (this == baseType.get())
    return true;
  if (!this->baseType)
    return false;
  if (this->baseType->inheritsFrom(baseType))
    return true;

  // FIXME: the check is not complete, TemplateClass should be distinguished from Class to properly check inheritance
  size_t n = getNumTemplateArguments();
  if (n > 0)
  {
    if (n != baseType->getNumTemplateArguments())
      return false;
    for (size_t i = 0; i < n; ++i)
      if (!getTemplateArgument(i)->inheritsFrom(baseType->getTemplateArgument(i)))
        return false;
    return true;
  }

  return false;
}

bool Type::canBeCastedTo(TypePtr targetType) const
  {return inheritsFrom(targetType);}

void Type::declare(TypePtr typeInstance)
  {getClassManagerInstance().declare(typeInstance);}

TypePtr Type::get(const String& typeName)
  {return getClassManagerInstance().get(typeName);}

TypePtr Type::get(const String& typeName, TypePtr argument)
  {return getClassManagerInstance().get(typeName, argument);}

TypePtr Type::get(const String& typeName, TypePtr argument1, TypePtr argument2)
  {return getClassManagerInstance().get(typeName, argument1, argument2);}

TypePtr Type::parseAndGet(const String& typeName, ErrorHandler& callback)
  {return getClassManagerInstance().parseAndGet(typeName, callback);}

Variable Type::createInstance(const String& typeName)
  {return getClassManagerInstance().createInstance(typeName);}

bool Type::doClassNameExists(const String& className)
  {return get(className) != TypePtr();}

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

void Type::saveToXml(XmlElement* xml, const VariableValue& value) const
  {jassert(baseType); return baseType->saveToXml(xml, value);}

size_t Type::getNumStaticVariables() const
  {jassert(baseType); return baseType->getNumStaticVariables();}

TypePtr Type::getStaticVariableType(size_t index) const
  {jassert(baseType); return baseType->getStaticVariableType(index);}

String Type::getStaticVariableName(size_t index) const
  {jassert(baseType); return baseType->getStaticVariableName(index);}

int Type::findStaticVariable(const String& name) const
{
  size_t n = getNumStaticVariables();
  for (size_t i = 0; i < n; ++i)
    if (getStaticVariableName(i) == name)
      return (int)i;
  return -1;
}
  
Variable Type::getSubVariable(const VariableValue& value, size_t index) const
  {jassert(baseType); return baseType->getSubVariable(value, index);}

/*
** Class
*/
String Class::toString() const
{
  String res = getName();
  res += T(" = {");
  size_t n = getNumStaticVariables();
  for (size_t i = 0; i < n; ++i)
  {
    res += getStaticVariableType(i)->getName() + T(" ") + getStaticVariableName(i);
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
  res.getObject()->thisClass = ClassPtr(const_cast<Class* >(this));
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
  res.getObject()->thisClass = ClassPtr(const_cast<Class* >(this));
  return res.getObject()->loadFromXml(xml, callback) ? res : getMissingValue();
}

void Class::saveToXml(XmlElement* xml, const VariableValue& value) const
{
  ObjectPtr object = value.getObject();
  jassert(object);
  object->saveToXml(xml);
}

size_t Class::getNumSubVariables(const VariableValue& value) const
{
  ObjectPtr object = value.getObject();
  return object ? object->getNumVariables() : 0;
}

String Class::getSubVariableName(const VariableValue& value, size_t index) const
  {return value.getObject()->getVariableName(index);}

/*
** DynamicClass
*/
DynamicClass::DynamicClass(const String& name, TypePtr baseClass)
  : Class(name, baseClass)
{
}

DynamicClass::DynamicClass(const String& name, const String& baseClass)
  : Class(name, Class::get(baseClass))
{
}

void DynamicClass::addVariable(const String& typeName, const String& name)
{
  TypePtr type = Type::parseAndGet(typeName, ErrorHandler::getInstance());
  addVariable(type, name);
}

size_t DynamicClass::getNumStaticVariables() const
{
  size_t n = baseType->getNumStaticVariables();
  ScopedLock _(variablesLock);
  return n + variables.size();
}

TypePtr DynamicClass::getStaticVariableType(size_t index) const
{
  size_t n = baseType->getNumStaticVariables();
  if (index < n)
    return baseType->getStaticVariableType(index);
  index -= n;
  
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].first;
}

String DynamicClass::getStaticVariableName(size_t index) const
{
  size_t n = baseType->getNumStaticVariables();
  if (index < n)
    return baseType->getStaticVariableName(index);
  index -= n;
  
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].second;
}

void DynamicClass::addVariable(Type* type, const String& name)
{
  if (!type || name.isEmpty())
  {
    Object::error(T("Class::addVariable"), T("Invalid type or name"));
    return;
  }
  ScopedLock _(variablesLock);
  if (findStaticVariable(name) >= 0)
    Object::error(T("Class::addVariable"), T("Another variable with name '") + name + T("' already exists"));
  else
    variables.push_back(std::make_pair(type, name));
}

int DynamicClass::findStaticVariable(const String& name) const
{
  ScopedLock _(variablesLock);
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i].second == name)
      return (int)(i + baseType->getNumStaticVariables());
  return baseType->findStaticVariable(name);
}

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes)
  : IntegerType(name, enumValueType()), oneLetterCodes(oneLetterCodes)
{
  jassert(!oneLetterCodes.containsChar('_')); // '_' is reserved to denote missing values
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& oneLetterCodes)
  : IntegerType(name, enumValueType()), oneLetterCodes(oneLetterCodes)
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
  : IntegerType(name, enumValueType())
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


#include <lbcpp/Data/ProbabilityDistribution.h>
 
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
    Type* res = Type::get(typeName, argument).get();
    m[argument.get()] = res;
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
    Type* res = Type::get(typeName, argument1, argument2).get();
    m[key] = res;
    return res;
  }
  else
    return it->second;
}

/*
** Type Declarations
*/
#include "Type/TopLevelType.h"
#include "Type/BooleanType.h"
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
DECLARE_CLASS_SINGLETON_ACCESSOR(pairType, T("Pair"));

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
  Type::declare(new PairType());

  Type::declare(new Class());
}

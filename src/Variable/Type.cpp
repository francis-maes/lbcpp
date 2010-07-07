/*-----------------------------------------.---------------------------------.
| Filename: Type.cpp                      | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Type.h>
#include <lbcpp/Object/Variable.h>
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
    TemplateTypeKey key;
    key.first = typeName;
    key.second.push_back(argument1);
    if (argument2) key.second.push_back(argument2);
    if (argument3) key.second.push_back(argument3);
    
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
    specializedType->setBaseClass(type);
    specializedType->setName(makeTemplateClassName(typeName, key.second));
    const_cast<TypeManager* >(this)->templateTypes[key] = specializedType;
    return specializedType;
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
      res += arguments[i]->getName();
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


/*
** Type
*/
bool Type::inheritsFrom(TypePtr baseClass) const
{
  jassert(this && baseClass.get());
  return this == baseClass.get() || (this->baseClass && this->baseClass->inheritsFrom(baseClass));
}

Variable Type::getSubVariable(const VariableValue& value, size_t index) const
  {jassert(false); return Variable();}

void Type::declare(TypePtr classInstance)
  {getClassManagerInstance().declare(classInstance);}

TypePtr Type::get(const String& typeName)
  {return getClassManagerInstance().get(typeName);}

TypePtr Type::get(const String& typeName, TypePtr argument)
  {return getClassManagerInstance().get(typeName, argument);}

TypePtr Type::get(const String& typeName, TypePtr argument1, TypePtr argument2)
  {return getClassManagerInstance().get(typeName, argument1, argument2);}

Variable Type::createInstance(const String& typeName)
  {return getClassManagerInstance().createInstance(typeName);}

bool Type::doClassNameExists(const String& className)
  {return get(className) != TypePtr();}

/*
** Class
*/
size_t Class::getNumStaticVariables() const
{
  ScopedLock _(variablesLock);
  return variables.size();
}

TypePtr Class::getStaticVariableType(size_t index) const
{
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].first;
}

String Class::getStaticVariableName(size_t index) const
{
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].second;
}

int Class::findStaticVariable(const String& name) const
{
  ScopedLock _(variablesLock);
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i].second == name)
      return (int)i;
  return -1;
}

void Class::addVariable(TypePtr type, const String& name)
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

Variable Class::getSubVariable(const VariableValue& value, size_t index) const
  {return value.getObject()->getVariable(index);}

void Class::setSubVariable(const VariableValue& value, size_t index, const Variable& subValue) const
  {value.getObject()->setVariable(index, subValue);}

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

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const juce::tchar** elements)
  : IntegerType(name, enumerationType())
{
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& elementChars)
  : IntegerType(name, enumerationType())
{
  for (int i = 0; i < elementChars.length(); ++i)
  {
    String str;
    str += elementChars[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : IntegerType(name, enumerationType())
{
}

void Enumeration::addElement(const String& elementName)
{
  for (size_t i = 0; i < elements.size(); ++i)
    if (elements[i] == elementName)
    {
      Object::error(T("Enumeration::addElement"), T("Element '") + elementName + T("' already exists"));
      return;
    }
  elements.push_back(elementName);
}

#include <lbcpp/Object/ProbabilityDistribution.h>

TypePtr Enumeration::multiplyByScalar(VariableValue& value, double scalar)
{
  DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(EnumerationPtr(this));
  distribution->setVariable((size_t)value.getInteger(), scalar);
  value.clearBuiltin();
  value.setObject(distribution);
  return distribution->getClass();
}

/*
** TypeCache
*/
TypeCache::TypeCache(const String& typeName)
{
  type = Type::get(typeName);
  if (!type)
    Object::error(T("TypeCache()"), T("Could not find type ") + typeName.quoted());
}

TypePtr UnaryTemplateTypeCache::operator ()(TypePtr argument)
{
  std::map<TypePtr, TypePtr>::const_iterator it = m.find(argument);
  if (it == m.end())
  {
    TypePtr res = Type::get(typeName, argument);
    m[argument] = res;
    return res;
  }
  else
    return it->second;
}

TypePtr BinaryTemplateTypeCache::operator ()(TypePtr argument1, TypePtr argument2)
{
  std::pair<TypePtr, TypePtr> key(argument1, argument2);
  std::map<std::pair<TypePtr, TypePtr>, TypePtr>::const_iterator it = m.find(key);
  if (it == m.end())
  {
    TypePtr res = Type::get(typeName, argument1, argument2);
    m[key] = res;
    return res;
  }
  else
    return it->second;
}

/*
** Type Declarations
*/
#include "../Type/TopLevelType.h"
#include "../Type/BooleanType.h"
#include "../Type/DoubleType.h"
#include "../Type/StringType.h"
#include "../Type/TupleType.h"

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
DECLARE_CLASS_SINGLETON_ACCESSOR(pairType, T("Pair"));

DECLARE_CLASS_SINGLETON_ACCESSOR(enumerationType, T("Enumeration"));

ClassPtr lbcpp::objectClass()
  {static TypeCache cache(T("Object")); return cache();}

TypePtr lbcpp::pairType(TypePtr firstClass, TypePtr secondClass)
  {static BinaryTemplateTypeCache cache(T("Pair")); return cache(firstClass, secondClass);}

void declareClassClasses()
{
  Type::declare(new TopLevelType());
    Type::declare(new NilType());

  Type::declare(new BooleanType());
  Type::declare(new IntegerType());
    Type::declare(new IntegerType(T("Enumeration"), integerType()));
  Type::declare(new DoubleType());
    Type::declare(new ProbabilityType());
    Type::declare(new AngstromDistanceType());

  Type::declare(new StringType());
  Type::declare(new PairType());

  Type::declare(new Class());
}

/*
** Variable
*/
Variable Variable::pair(const Variable& variable1, const Variable& variable2)
  {return Variable(pairType(variable1.getType(), variable2.getType()), PairType::allocate(variable1, variable2));}

Variable Variable::copyFrom(TypePtr type, const VariableValue& value)
{
  Variable res;
  res.type = type;
  if (type)
    type->copy(res.value, value);
  return res;
}

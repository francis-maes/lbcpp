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

  ObjectPtr createInstance(const String& className) const
  {
    if (className.isEmpty())
    {
      Object::error(T("TypeManager::create"), T("Empty class name"));
      return ObjectPtr();
    }

    TypePtr cl = get(className);
    if (!cl)
    {
      Object::error(T("TypeManager::create"), T("Could not find class '") + className + T("'"));
      return ObjectPtr();
    }

    Type::DefaultConstructor defaultConstructor = cl->getDefaultConstructor();
    if (!defaultConstructor)
    {
      Object::error(T("TypeManager::create"), T("Class '") + className + T("' has no default constructor"));
      return ObjectPtr();
    }
    return defaultConstructor();
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

  CriticalSection typesLock;
  TypeMap types;

  bool standardTypesAreDeclared;
};

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

TypePtr Type::get(const String& className)
  {return getClassManagerInstance().get(className);}

ObjectPtr Type::createInstance(const String& className)
  {return getClassManagerInstance().createInstance(className);}

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

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const juce::tchar** elements)
  : IntegerType(name)
{
  baseClass = integerType();
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& elementChars)
  : IntegerType(name)
{
  baseClass = integerType();
  for (int i = 0; i < elementChars.length(); ++i)
  {
    String str;
    str += elementChars[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : IntegerType(name)
{
  baseClass = integerType();
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

#include "../Type/BooleanType.h"
#include "../Type/IntegerType.h"
#include "../Type/DoubleType.h"
#include "../Type/StringType.h"
#include "../Type/TupleType.h"

#define DECLARE_CLASS_SINGLETON_ACCESSOR(AccessorName, ClassName) \
  TypePtr lbcpp::AccessorName() { \
      static TypePtr res; \
      if (!res) {res = Type::get(ClassName); jassert(res);} \
      return res; }

DECLARE_CLASS_SINGLETON_ACCESSOR(topLevelType, T("Variable"));

DECLARE_CLASS_SINGLETON_ACCESSOR(booleanType, T("Boolean"));
DECLARE_CLASS_SINGLETON_ACCESSOR(integerType, T("Integer"));
DECLARE_CLASS_SINGLETON_ACCESSOR(doubleType, T("Double"));
DECLARE_CLASS_SINGLETON_ACCESSOR(stringType, T("String"));
DECLARE_CLASS_SINGLETON_ACCESSOR(pairType, T("Pair"));

DECLARE_CLASS_SINGLETON_ACCESSOR(objectClass, T("Object"));

TypePtr lbcpp::pairType(TypePtr firstClass, TypePtr secondClass)
  {return pairType();} // FIXME

Variable Variable::pair(const Variable& variable1, const Variable& variable2)
{
  return Variable(pairType(), PairType::allocate(variable1, variable2));
}
Variable Variable::copyFrom(TypePtr type, const VariableValue& value)
{
  Variable res;
  res.type = type;
  if (type)
    type->copy(res.value, value);
  return res;
}

void declareClassClasses()
{
  Type::declare(new TopLevelType());

  Type::declare(new BooleanType());
  Type::declare(new IntegerType());
  Type::declare(new DoubleType());
  Type::declare(new StringType());
  Type::declare(new PairType());

  Type::declare(new Class());
}

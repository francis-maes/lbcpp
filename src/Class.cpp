/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Class.h>
#include <lbcpp/Object/Variable.h>
#include <map>
using namespace lbcpp;

extern void declareLBCppCoreClasses();

/*
** ClassManager
*/
class ClassManager
{
public:
  ClassManager() : standardClassesAreDeclared(false) {}

  void declare(ClassPtr classInstance)
  {
    if (!classInstance || classInstance->getName().isEmpty())
      Object::error(T("ClassManager::declare"), T("Empty class name"));

    ScopedLock _(classesLock);
    String className = classInstance->getName();
    ClassMap::const_iterator it = classes.find(className);
    if (it != classes.end())
    {
      Object::error(T("ClassManager::declare"), T("The class '") + className + T("' has already been declared"));
      return;
    }
    classes[className] = classInstance;
  }

  ClassPtr get(const String& className) const
  {
    ScopedLock _(classesLock);
    ClassMap::const_iterator it = classes.find(className);
    return it == classes.end() ? ClassPtr() : it->second;
  }

  ObjectPtr createInstance(const String& className) const
  {
    if (className.isEmpty())
    {
      Object::error(T("ClassManager::create"), T("Empty class name"));
      return ObjectPtr();
    }

    ClassPtr cl = get(className);
    if (!cl)
    {
      Object::error(T("ClassManager::create"), T("Could not find class '") + className + T("'"));
      return ObjectPtr();
    }

    Class::DefaultConstructor defaultConstructor = cl->getDefaultConstructor();
    if (!defaultConstructor)
    {
      Object::error(T("ClassManager::create"), T("Class '") + className + T("' has no default constructor"));
      return ObjectPtr();
    }
    return defaultConstructor();
  }

  void ensureStandardClassesAreLoaded()
  {
    if (!standardClassesAreDeclared)
    {
      standardClassesAreDeclared = true;
      declareLBCppCoreClasses();
    }
  }

private:
  typedef std::map<String, ClassPtr> ClassMap;

  CriticalSection classesLock;
  ClassMap classes;

  bool standardClassesAreDeclared;
};

inline ClassManager& getClassManagerInstance()
{
  static ClassManager instance;
  return instance;
}

void lbcpp::initialize()
  {getClassManagerInstance().ensureStandardClassesAreLoaded();}


/*
** Class
*/
bool Class::inheritsFrom(ClassPtr baseClass) const
{
  jassert(this && baseClass.get());
  return this == baseClass.get() || this->baseClass->inheritsFrom(baseClass);
}

Variable Class::getSubVariable(const VariableValue& value, size_t index) const
  {jassert(false); return Variable();}

void Class::declare(ClassPtr classInstance)
  {getClassManagerInstance().declare(classInstance);}

ClassPtr Class::get(const String& className)
  {return getClassManagerInstance().get(className);}

ObjectPtr Class::createInstance(const String& className)
  {return getClassManagerInstance().createInstance(className);}

bool Class::doClassNameExists(const String& className)
  {return get(className) != ClassPtr();}

std::vector< std::pair<ClassPtr, String> > variables;
  CriticalSection variablesLock;

size_t Class::getNumVariables() const
{
  ScopedLock _(variablesLock);
  return variables.size();
}

ClassPtr Class::getVariableType(size_t index) const
{
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].first;
}

const String& Class::getVariableName(size_t index) const
{
  ScopedLock _(variablesLock);
  jassert(index < variables.size());
  return variables[index].second;
}

int Class::findVariable(const String& name) const
{
  ScopedLock _(variablesLock);
  for (size_t i = 0; i < variables.size(); ++i)
    if (variables[i].second == name)
      return (int)i;
  return -1;
}

void Class::addVariable(ClassPtr type, const String& name)
{
  if (!type || name.isEmpty())
  {
    Object::error(T("Class::addVariable"), T("Invalid type or name"));
    return;
  }
  ScopedLock _(variablesLock);
  if (findVariable(name) >= 0)
    Object::error(T("Class::addVariable"), T("Another variable with name '") + name + T("' already exists"));
  else
    variables.push_back(std::make_pair(type, name));
}

/*
** Enumeration
*/
Enumeration::Enumeration(const String& name, const juce::tchar** elements)
  : IntegerClass(name)
{
  baseClass = integerClass();
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& elementChars)
  : IntegerClass(name)
{
  baseClass = integerClass();
  for (int i = 0; i < elementChars.length(); ++i)
  {
    String str;
    str += elementChars[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : IntegerClass(name)
{
  baseClass = integerClass();
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

#include "Class/BooleanClass.h"
#include "Class/IntegerClass.h"
#include "Class/DoubleClass.h"
#include "Class/StringClass.h"
#include "Class/TupleClass.h"

#define DECLARE_CLASS_SINGLETON_ACCESSOR(AccessorName, ClassName) \
  ClassPtr lbcpp::AccessorName() { \
      static ClassPtr res; \
      if (!res) {res = Class::get(ClassName); jassert(res);} \
      return res; }

DECLARE_CLASS_SINGLETON_ACCESSOR(topLevelClass, T("Variable"));

DECLARE_CLASS_SINGLETON_ACCESSOR(booleanClass, T("Boolean"));
DECLARE_CLASS_SINGLETON_ACCESSOR(integerClass, T("Integer"));
DECLARE_CLASS_SINGLETON_ACCESSOR(doubleClass, T("Double"));
DECLARE_CLASS_SINGLETON_ACCESSOR(stringClass, T("String"));
DECLARE_CLASS_SINGLETON_ACCESSOR(pairClass, T("Pair"));

DECLARE_CLASS_SINGLETON_ACCESSOR(objectClass, T("Object"));

Variable Variable::pair(const Variable& variable1, const Variable& variable2)
{
  return Variable(pairClass(), PairClass::allocate(variable1, variable2));
}

void declareClassClasses()
{
  Class::declare(new TopLevelClass());

  Class::declare(new BooleanClass());
  Class::declare(new IntegerClass());
  Class::declare(new DoubleClass());
  Class::declare(new StringClass());
  Class::declare(new PairClass());

  Class::declare(new ObjectClass());
}

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
    ensureStandardClassesAreLoaded();
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
    const_cast<ClassManager* >(this)->ensureStandardClassesAreLoaded();
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

private:
  typedef std::map<String, ClassPtr> ClassMap;

  CriticalSection classesLock;
  ClassMap classes;

  bool standardClassesAreDeclared;

  void ensureStandardClassesAreLoaded()
  {
    if (!standardClassesAreDeclared)
    {
      standardClassesAreDeclared = true;
      declareLBCppCoreClasses();
    }
  }
};

inline ClassManager& getClassManagerInstance()
{
  static ClassManager instance;
  return instance;
}

/*
** Class
*/
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
  : IntegerClass(name, IntegerClass::getInstance())
{
  for (size_t index = 0; elements[index]; ++index)
    addElement(elements[index]);
}

Enumeration::Enumeration(const String& name, const String& elementChars)
  : IntegerClass(name, IntegerClass::getInstance())
{
  for (int i = 0; i < elementChars.length(); ++i)
  {
    String str;
    str += elementChars[i];
    addElement(str);
  }
}

Enumeration::Enumeration(const String& name)
  : IntegerClass(name, IntegerClass::getInstance())
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

/*
** VariablePairClass
*/
VariableValue VariablePairClass::allocate(const Variable& variable1, const Variable& variable2)
{
  char* data = new char[sizeof (Variable) * 2];
  memset(data, 0, sizeof (Variable) * 2);
  ((Variable* )data)[0] = variable1;
  ((Variable* )data)[1] = variable2;
  return data;
}
void VariablePairClass::copy(VariableValue& dest, const VariableValue& source) const
{
  jassert(false);
  //dest = allocate(source[0], source[1]);
}


void declareClassClasses()
{
  Class::declare(new BooleanClass());
  Class::declare(new IntegerClass());
  Class::declare(new DoubleClass());
  Class::declare(new StringClass());
  Class::declare(new ObjectClass());
  Class::declare(new VariablePairClass());
}

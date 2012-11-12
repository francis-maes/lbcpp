/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core.h>
#include <lbcpp/Lua/Lua.h>
#include <lbcpp/library.h>
#include <fstream>
using namespace lbcpp;

#ifdef JUCE_DEBUG
namespace lbcpp
{
#ifdef JUCE_MSVC
  void* debugMalloc(const int size, const char* file, const int line)
    {return _malloc_dbg (size, _NORMAL_BLOCK, file, line);}

  void* debugCalloc(const int size, const char* file, const int line)
    {return _calloc_dbg(1, size, _NORMAL_BLOCK, file, line);}

  void* debugRealloc(void* const block, const int size, const char* file, const int line)
    {return _realloc_dbg(block, size, _NORMAL_BLOCK, file, line);}

  void debugFree(void* const block)
    {_free_dbg(block, _NORMAL_BLOCK);}
#else
  void* debugMalloc(const int size, const char* file, const int line)
    {return malloc(size);}

  void* debugCalloc(const int size, const char* file, const int line)
    {return calloc(1, size);}

  void* debugRealloc(void* const block, const int size, const char* file, const int line)
    {return realloc(block, size);}

  void debugFree(void* const block)
    {free(block);}
#endif 
}; /* namespace lbcpp */
#endif // JUCE_DEBUG

#ifdef LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS

static CriticalSection refCountInfoLock;
static std::map<String, int> numAccessesPerType;
static int totalNumAccesses = 0;

void ReferenceCountedObject::resetRefCountDebugInfo()
{
  ScopedLock _(refCountInfoLock);
  numAccessesPerType.clear();
  totalNumAccesses = 0;
}

void ReferenceCountedObject::displayRefCountDebugInfo(std::ostream& ostr)
{
  ScopedLock _(refCountInfoLock);
  ostr << "Total num accesses: " << totalNumAccesses << std::endl;
  std::multimap<int, String> invMap;
  for (std::map<String, int>::const_iterator it = numAccessesPerType.begin(); it != numAccessesPerType.end(); ++it)
    invMap.insert(std::make_pair(it->second, it->first));
  for (std::multimap<int, String>::const_reverse_iterator it = invMap.rbegin(); it != invMap.rend(); ++it)
    ostr << "  " << it->second << ": " << it->first << std::endl;
}

void ReferenceCountedObject::incrementReferenceCounter()
{
  if (!hasStaticAllocationFlag())
    juce::atomicIncrement(refCount);
  ScopedLock _(refCountInfoLock);
  numAccessesPerType[getTypeName(typeid(*this))]++;
  ++totalNumAccesses;
}

/** Decrements the object's reference count.  */
void ReferenceCountedObject::decrementReferenceCounter()
{
  {
    ScopedLock _(refCountInfoLock);
    numAccessesPerType[getTypeName(typeid(*this))]++;
    ++totalNumAccesses;
  }

  if (!hasStaticAllocationFlag() && juce::atomicDecrementAndReturn(refCount) == 0)
    delete this;
}
#else
void ReferenceCountedObject::resetRefCountDebugInfo()
  {}
void ReferenceCountedObject::displayRefCountDebugInfo(std::ostream& ostr)
  {ostr << "No RefCount Debug Info." << std::endl;}
#endif // LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS

/*
** Object Class
*/
void Object::setThisClass(ClassPtr thisClass)
  {this->thisClass = thisClass;}

String Object::getClassName() const
{
  if (thisClass)
    return thisClass->getName();
  else
    return getTypeName(typeid(*this));
}

ClassPtr Object::getClass() const
{
  if (!thisClass)
  {
    // /!!\ This is not thread-safe
    //jassert(false);
    String typeName = getTypeName(typeid(*this));
    const_cast<Object* >(this)->thisClass = lbcpp::getType(typeName);
    jassert(thisClass);
  }
  return thisClass;
}

ObjectPtr Object::create(ClassPtr type)
  {return type->createObject(defaultExecutionContext());}

/*
** Create Object From String
*/
static ObjectPtr createObjectFromShortNameOrName(ExecutionContext& context, ClassPtr baseClass, const String& nameOrShortName)
{
  if (nameOrShortName == T("Missing"))
    return ObjectPtr();
  ClassPtr type = typeManager().getTypeByShortName(context, nameOrShortName);
  if (!type)
    type = typeManager().getType(context, nameOrShortName);
  if (!type)
    return ObjectPtr();
  if (!context.checkInheritance(type, baseClass))
    return ObjectPtr();
  return Object::create(type);
}

static ObjectPtr createObjectFromStringWithAbstractClass(ExecutionContext& context, ClassPtr baseClass, const String& value)
{
  int n = value.indexOfChar('(');
  if (n >= 0)
  {
    ObjectPtr res = createObjectFromShortNameOrName(context, baseClass, value.substring(0, n));
    if (!res)
      return ObjectPtr();

    int e = value.lastIndexOfChar(')');
    if (e <= n)
    {
      context.errorCallback(T("Unmatched parenthesis in ") + value.quoted());
      return ObjectPtr();
    }
    String arguments = value.substring(n + 1, e).trim();
    if (arguments.isNotEmpty() && !res->loadFromString(context, arguments))
      res = ObjectPtr();
    return res;
  }
  else
    return createObjectFromShortNameOrName(context, baseClass, value);
}

ObjectPtr Object::createFromString(ExecutionContext& context, ClassPtr type, const String& value)
{
  if (type->inheritsFrom(classClass))
    return typeManager().getType(context, value);

  if (type->isAbstract())
    return createObjectFromStringWithAbstractClass(context, type, value);
  else
  {
    ObjectPtr res = type->createObject(context);
    if (!res)
      context.errorCallback(T("Object::createFromString"), T("Could not create instance of ") + type->getName().quoted());
    else if (!res->loadFromString(context, value))
      res = ObjectPtr();
    return res;
  }
}

/*
** Create Object From Xml
*/
ObjectPtr Object::createFromXml(XmlImporter& importer, ClassPtr type)
{
  if (type->inheritsFrom(classClass))
  {
    String text = importer.getAllSubText().trim();
    if (text.isNotEmpty())
      return typeManager().getType(importer.getContext(), importer.getAllSubText());
    else
      return importer.loadUnnamedType();
  }
  else
  {
    ObjectPtr res = type->createObject(importer.getContext());
    if (!res)
      importer.errorMessage(T("Class::createFromXml"), T("Could not create instance of ") + type->getName().quoted());
    else if (!res->loadFromXml(importer))
      res = ObjectPtr();
    return res;
  }
}

/*
** Create Object from File / Save Object to File
*/
ObjectPtr Object::createFromFile(ExecutionContext& context, const File& file)
{
  LoaderPtr loader = lbcpp::getTopLevelLibrary()->findLoaderForFile(context, file);
  return loader ? loader->loadFromFile(context, file) : ObjectPtr();
}

bool Object::saveToFile(ExecutionContext& context, const File& file) const
{
  XmlExporter exporter(context);
  exporter.saveObject(String::empty, refCountedPointerFromThis(this), ClassPtr());
  return exporter.saveToFile(file);
}

size_t Object::getNumVariables() const
  {return getClass()->getNumMemberVariables();}

ClassPtr Object::getVariableType(size_t index) const
  {return getClass()->getMemberVariableType(index);}

String Object::getVariableName(size_t index) const
  {return getClass()->getMemberVariableName(index);}

ObjectPtr Object::getVariable(size_t index) const
{
  ClassPtr thisClass = getClass();
  jassert(index < thisClass->getNumMemberVariables());
  return thisClass->getMemberVariableValue(this, index);
}

void Object::setVariable(size_t index, const ObjectPtr& value)
{
  jassert(index < getClass()->getNumMemberVariables());
  getClass()->setMemberVariableValue(this, index, value);
}

int Object::compare(const ObjectPtr& object1, const ObjectPtr& object2)
{
  if (!object1 && !object2)
    return 0;
  if (!object1 && object2)
    return -1;
  if (object1 && !object2)
    return 1;
  return object1->compare(object2);
}

bool Object::equals(const ObjectPtr& object1, const ObjectPtr& object2)
{
  if (!object1 && !object2)
    return true;
  if (!object1 || !object2)
    return false;
  if (object1->getClass() != object2->getClass())
    return false;
  return object1->compare(object2) == 0;
}

/*
** to string
*/
String Object::toString() const
{
  ClassPtr thisClass = getClass();
  String res = thisClass->getShortName().isNotEmpty() ? thisClass->getShortName() : thisClass->getName();
  if (getNumVariables())
    res += T("(") + defaultToStringImplementation(false) + T(")");
  return res;
}

String Object::toShortString() const
{
  ClassPtr thisClass = getClass();
  String res = thisClass->getShortName().isNotEmpty() ? thisClass->getShortName() : thisClass->getName();
  if (getNumVariables())
    res += T("(") + defaultToStringImplementation(true) + T(")");
  return res;
}

String Object::defaultToStringImplementation(bool useShortString) const
{
  ClassPtr type = getClass();
  size_t n = type->getNumMemberVariables();
  String res;
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr value = getVariable(i);
    res += (useShortString ? value->toShortString() : value->toString());
    if (i < n - 1)
      res += T(", ");
  }
  return res;
}

String Object::variablesToString(const String& separator, bool includeTypes) const
{
  String res;
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr v = getVariable(i);
    if (includeTypes)
      res += v->getClassName() + T(" ");
    res += v->toString();
    if (i < n - 1)
      res += separator;
  }
  return res;
}

size_t Object::getSizeInBytes(bool recursively) const
{
  size_t res = sizeof (*this);
  if (recursively && !hasStaticAllocationFlag())
  {
    ClassPtr thisClass = getClass();
    if (thisClass)
    {
      size_t n = getNumVariables();
      for (size_t i = 0; i < n; ++i)
      {
        ObjectPtr v = getVariable(i);
        if (v)
          res += v->getSizeInBytes(true);
      }
    }
  }
  return res;
}

int Object::compareVariables(const ObjectPtr& otherObject) const
{
  ClassPtr c1 = getClass();
  ClassPtr c2 = otherObject->getClass();
  if (c1 != c2)
    return c1 < c2;
  size_t n = getNumVariables();
  jassert(n == otherObject->getNumVariables());
  for (size_t i = 0; i < n; ++i)
  {
    int cmp = Object::compare(getVariable(i), otherObject->getVariable(i));
    if (cmp != 0)
      return cmp;
  }
  return 0;
}

/*
** Clone
*/
ObjectPtr Object::clone(ExecutionContext& context) const
{
  ClassPtr cl = getClass();
  if (!cl)
    return ObjectPtr();
  ObjectPtr res = Object::create(getClass());
  jassert(res);
  clone(context, res);
  return res;
}

void Object::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
    target->setVariable(i, getVariable(i));
}

ObjectPtr Object::deepClone(ExecutionContext& context) const
{
  ClassPtr thisClass = getClass();
  ObjectPtr res = clone(context);
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    ClassPtr variableType = thisClass->getMemberVariableType(i);
    if (variableType->inheritsFrom(objectClass) && !variableType->inheritsFrom(classClass))
    {
      ObjectPtr object = res->getVariable(i);
      if (object)
        res->setVariable(i, object->deepClone(context));
    }
  }
  return res;
}

ObjectPtr Object::cloneToNewType(ExecutionContext& context, ClassPtr newType) const
{
  ClassPtr thisClass = getClass();
  if (newType == thisClass)
    return clone(context);

  ObjectPtr res = Object::create(newType);
  size_t n = newType->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    int sourceIndex = thisClass->findMemberVariable(newType->getMemberVariableName(i));
    if (sourceIndex >= 0)
    {
      ObjectPtr object = getVariable((size_t)sourceIndex);
      if (object)
      {
        ClassPtr newVariableType = newType->getMemberVariableType(i);
        res->setVariable(i, object->cloneToNewType(context, newVariableType));
      }
    }
  }
  return res;
}

/*
** XML Serialisation
*/
void Object::saveToXml(XmlExporter& exporter) const
{
  ClassPtr type = getClass();
  DefaultClassPtr defaultClass = type.dynamicCast<DefaultClass>();
  
  size_t n = type->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr variable = getVariable(i);
    bool isGenerated = defaultClass && defaultClass->isMemberVariableGenerated(i);

    if (isGenerated)
      exporter.saveGeneratedObject(getVariableName(i), variable, getVariableType(i));
    else if (variable)
      exporter.saveObject(getVariableName(i), variable, getVariableType(i));
  }
}

ObjectPtr Object::computeGeneratedObject(ExecutionContext& context, const String& variableName)
{
  jassert(false);
  return ObjectPtr();
}

bool Object::loadFromXml(XmlImporter& importer)
{
  ClassPtr thisClass = getClass();
  DefaultClassPtr defaultClass = thisClass.dynamicCast<DefaultClass>();
  bool ok = true;
  
  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), child, T("variable"))
  {
    String name = child->getStringAttribute(T("name"));
    if (name.isEmpty())
    {
      importer.errorMessage(T("Object::loadFromXml"), T("Missing name attribute in variable"));
      ok = false;
      continue;
    }
    int variableNumber = thisClass->findMemberVariable(name);
    if (variableNumber < 0)
    {
      importer.unknownVariableWarning(thisClass, name);
      continue;
    }
    ClassPtr expectedType = thisClass->getMemberVariableType((size_t)variableNumber);
    jassert(expectedType);
    
    ObjectPtr value;
    if (child->getBoolAttribute(T("generated"), false))
    {
      value = computeGeneratedObject(importer.getContext(), name);
      if (!value)
      {
        ok = false;
        continue;
      }
      importer.enter(child);
      importer.linkCurrentElementToObject(value);
      importer.leave();
    }
    else
      value = importer.loadObject(child, expectedType);
      
    if (value)
    {
      if (!importer.getContext().checkInheritance((ClassPtr)value->getClass(), expectedType))
      {
        ok = false;
        continue;
      }
      setVariable((size_t)variableNumber, value);
    }
  }
  return ok;
}

void Object::saveVariablesToXmlAttributes(XmlExporter& exporter) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
    exporter.setAttribute(getVariableName(i), getVariable(i)->toString());
}

bool Object::loadVariablesFromXmlAttributes(XmlImporter& importer)
{
  juce::XmlElement* xml = importer.getCurrentElement();
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    String name = getVariableName(i);
    if (xml->hasAttribute(name))
    {
      ObjectPtr var = Object::createFromString(importer.getContext(), getVariableType(i), xml->getStringAttribute(name));
      if (var)
        setVariable(i, var);
    }
    else if (name != T("thisClass"))
      importer.warningMessage(T("Object::loadVariablesFromXmlAttributes"), T("No value for variable ") + name.quoted());
  }
  return true;
}

bool Object::loadFromString(ExecutionContext& context, const String& str)
  {return loadArgumentsFromString(context, str);}

namespace lbcpp
{
  extern bool parseListWithParenthesis(ExecutionContext& context, const String& str, char openParenthesis, char closeParenthesis, char comma, std::vector<String>& res);
};

bool Object::loadArgumentsFromString(ExecutionContext& context, const String& str)
{
  std::vector<String> tokens;
  if (!parseListWithParenthesis(context, str, '(', ')', ',', tokens))
    return false;

  size_t n = getNumVariables();
  if (tokens.size() > n)
  {
    context.errorCallback(T("Could not parse ") + str.quoted() + T(": too much tokens (") +
      String((int)tokens.size()) + T(" tokens, ") + String((int)n) + T(" expected variables)"));
    return false;
  }
  bool ok = true;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    ObjectPtr value = Object::createFromString(context, getVariableType(i), tokens[i]);
    if (value)
      setVariable(i, value);
    else
      ok = false;
  }
  return ok;
}

/*
** Lua
*/
int Object::create(LuaState& state)
{
  int numArguments = state.getTop();

  const char* className = state.checkString(1);
  ClassPtr type = typeManager().getType(state.getContext(), className);
  if (!type)
    return 0;

  if (numArguments - 1 > (int)type->getNumMemberVariables())
  {
    state.error("Too much arguments in Object::create()");
    return 0;
  }

  ObjectPtr res = Object::create(type);
  for (int i = 2; i <= numArguments; ++i)
  {
    ObjectPtr v = state.checkObject(i);
    if (!v)
      continue; // ignore null values
    ClassPtr targetType = type->getMemberVariableType(i - 2);
    ClassPtr sourceType = v->getClass();
    if (targetType->inheritsFrom(integerClass) && sourceType->inheritsFrom(doubleClass))
      res->setVariable(i - 2, new Integer((int)Double::get(v))); // a la rache cast from double to int
    else
      res->setVariable(i - 2, v);
  }
  if (!res)
    return 0;
  state.pushObject(res);
  return 1;
}

int Object::fromFile(LuaState& state)
{
  File file = state.checkFile(1);
  ObjectPtr res = createFromFile(state.getContext(), file);
  if (!res)
    return 0;
  state.pushObject(res);
  return 1;
}

int Object::toString(LuaState& state)
{
  ObjectPtr object = state.checkObject(1);
  state.pushString(object->toString());
  return 1;
}

int Object::toShortString(LuaState& state)
{
  ObjectPtr object = state.checkObject(1);
  state.pushString(object->toShortString());
  return 1;
}

int Object::clone(LuaState& state)
{
  ObjectPtr object = state.checkObject(1);
  ObjectPtr res = object->clone(state.getContext());
  if (!res)
    return 0;
  state.pushObject(res);
  return 1;
}

int Object::save(LuaState& state)
{
  ObjectPtr object = state.checkObject(1);
  File file = state.checkFile(2);
  object->saveToFile(state.getContext(), file);
  return 0;
}

int Object::__index(LuaState& state) const
{
  if (state.isString(1)) // indiced by a string
  {
    String string = state.checkString(1);

    if (string == T("className"))
    {
      state.pushString(getClassName());
      return 1;
    }

    // check if it is a variable
    ClassPtr type = getClass();
    int index = type->findMemberVariable(string);
    if (index >= 0)
    {
      state.pushObject(getVariable(index));
      return 1;
    }

    // check if it is a function
    index = type->findMemberFunction(string);
    if (index >= 0)
    {
      LuaFunctionSignaturePtr signature = type->getMemberFunction(index).dynamicCast<LuaFunctionSignature>();
      if (!signature)
      {
        state.error("This kind of functions is not supported");
        return 0;
      }

      state.pushFunction(signature->getFunction());
      return 1;
    }

    state.error("Could not find identifier " + string.quoted() + " in class " + type->getName());
    return 0;
  }
  else if (state.isInteger(1))
  {
    state.error("No operator to index this kind of objects with integers");
    return 0;
  }
  else
  {
    state.error("Invalid type of key in index");
    return 0;
  }
}

int Object::__newIndex(LuaState& state)
{
  if (state.isString(1))
  {
    String string = state.checkString(1);

    // check if it is a variable
    ClassPtr type = getClass();
    int index = type->findMemberVariable(string);
    if (index >= 0)
    {
      ObjectPtr object = state.checkObject(2);
      if (type->getMemberVariableType(index)->inheritsFrom(integerClass))
        setVariable(index, new Integer(juce::roundDoubleToInt(Double::get(object))));
      else
        setVariable(index, object);
    }
    else
      state.error("Could not find variable");
  }
  else
  {
    state.error("Invalid type of key in newindex");
    return 0;
  }
  return 0;
}

int Object::__add(LuaState& state)
{
  state.error("Cannot add an object");
  return 0;
}

int Object::__sub(LuaState& state)
{
  state.error("Cannot subtract from an object");
  return 0;
}

int Object::__mul(LuaState& state)
{
  state.error("Cannot multiply an object");
  return 0;
}

int Object::__div(LuaState& state)
{
  state.error("Cannot divide an object");
  return 0;
}

int Object::__eq(LuaState& state)
{
  ObjectPtr otherObject = state.checkObject(1);
  state.pushBoolean(compare(otherObject) == 0);
  return 1;
}

int Object::__len(LuaState& state) const
{
  state.pushInteger(0);
  return 1;
}

int Object::__call(LuaState& state)
{
  state.error("Cannot call an object");
  return 0;
}

int Object::garbageCollect(LuaState& state)
{
  ObjectPtr& object = state.checkObject(1);
  //std::cout << "GC: " << object->toShortString() << std::endl;
  object = ObjectPtr();
  return 0;
}

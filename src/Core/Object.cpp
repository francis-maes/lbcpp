/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Object.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/Container.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
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

ObjectPtr Object::create(ClassPtr objectClass)
  {return objectClass->create(defaultExecutionContext()).getObject();}

size_t Object::getNumVariables() const
  {return getClass()->getNumMemberVariables();}

TypePtr Object::getVariableType(size_t index) const
  {return getClass()->getMemberVariableType(index);}

String Object::getVariableName(size_t index) const
  {return getClass()->getMemberVariableName(index);}

Variable Object::getVariable(size_t index) const
{
  ClassPtr thisClass = getClass();
  jassert(index < thisClass->getNumMemberVariables());
  return thisClass->getMemberVariableValue(this, index);
}

void Object::setVariable(size_t index, const Variable& value)
{
  jassert(index < getClass()->getNumMemberVariables());
  getClass()->setMemberVariableValue(this, index, value);
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
    Variable value = getVariable(i);
    res += (useShortString ? value.toShortString() : value.toString());
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
    Variable v = getVariable(i);
    if (includeTypes && !v.isNil())
      res += v.getTypeName() + T(" ");
    res += v.toString();
    if (i < n - 1)
      res += separator;
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
    int cmp = getVariable(i).compare(otherObject->getVariable(i));
    if (cmp != 0)
      return cmp;
  }
  return 0;
}

static void getAllChildObjectsRecursively(const ObjectPtr& object, std::set<ObjectPtr>& res)
{
  if (res.find(object) == res.end())
  {
    res.insert(object);
    std::vector<ObjectPtr> childObjects;
    object->getChildObjects(childObjects);
    for (size_t i = 0; i < childObjects.size(); ++i)
      getAllChildObjectsRecursively(childObjects[i], res);
  }
}

void Object::getChildObjects(std::vector<ObjectPtr>& res) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = getVariable(i);
    if (v.isObject() && v.exists())
      res.push_back(v.getObject());
  }
}
  

void Object::getAllChildObjects(std::set<ObjectPtr>& res) const
  {return getAllChildObjectsRecursively(refCountedPointerFromThis(this), res);}

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
    TypePtr variableType = thisClass->getMemberVariableType(i);
    if (variableType->inheritsFrom(objectClass) && !variableType->inheritsFrom(typeClass))
    {
      ObjectPtr object = res->getVariable(i).getObject();
      if (object)
        res->setVariable(i, Variable(object->deepClone(context), variableType));
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
      Variable variable = getVariable((size_t)sourceIndex);
      if (!variable.exists())
        continue;

      TypePtr newVariableType = newType->getMemberVariableType(i);
      if (variable.isObject())
        res->setVariable(i, variable.getObject()->cloneToNewType(context, newVariableType));
      else
      {
        jassert(variable.getType() == newVariableType);
        res->setVariable(i, variable);
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
    Variable variable = getVariable(i);
    bool isGenerated = defaultClass && defaultClass->isMemberVariableGenerated(i);

    if (isGenerated)
      exporter.saveGeneratedVariable(getVariableName(i), variable.getObject(), getVariableType(i));
    else
    {
      if (!variable.isMissingValue())
        exporter.saveVariable(getVariableName(i), variable, getVariableType(i));
    }
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
      importer.errorMessage(T("Object::loadFromXml"), T("Could not find variable name"));
      ok = false;
      continue;
    }
    int variableNumber = thisClass->findMemberVariable(name);
    if (variableNumber < 0)
    {
      importer.warningMessage(T("Object::loadFromXml"), T("Could not find variable ") + name.quoted() + T(" in class ") + thisClass->getName());
      continue;
    }
    TypePtr expectedType = thisClass->getMemberVariableType((size_t)variableNumber);
    jassert(expectedType);
    
    Variable value;
    if (child->getBoolAttribute(T("generated"), false))
    {
      ObjectPtr object = computeGeneratedObject(importer.getContext(), name);
      if (!object)
      {
        ok = false;
        continue;
      }
      value = Variable(object);
      importer.enter(child);
      importer.linkCurrentElementToObject(object);
      importer.leave();
    }
    else
      value = importer.loadVariable(child, expectedType);
      
    if (value.exists())
    {
      if (!importer.getContext().checkInheritance(value, expectedType))
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
    exporter.setAttribute(getVariableName(i), getVariable(i).toString());
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
      Variable var = Variable::createFromString(importer.getContext(), getVariableType(i), xml->getStringAttribute(name));
      if (!var.isMissingValue())
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
      String((int)tokens.size()) + T(" tokens, ") + String((int)n) + T(" expected variables."));
    return false;
  }
  bool ok = true;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    Variable value = Variable::createFromString(context, getVariableType(i), tokens[i]);
    if (!value.isNil())
      setVariable(i, value);
    else
      ok = false;
  }
  return ok;
}

void Object::saveToFile(ExecutionContext& context, const File& file) const
  {Variable(const_cast<Object* >(this)).saveToFile(context, file);}

ObjectPtr Object::createFromFile(ExecutionContext& context, const File& file)
  {return Variable::createFromFile(context, file).getObject();}

/*
** Lua
*/
int Object::create(LuaState& state)
{
  int numArguments = state.getTop();

  const char* className = state.checkString(1);
  TypePtr type = getType(className);
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
    Variable v = state.checkVariable(i);
    if (v.isNil())
      continue; // ignore nil values
    TypePtr targetType = type->getMemberVariableType(i - 2);
    TypePtr sourceType = v.getType();
    if (targetType->inheritsFrom(integerType) && sourceType->inheritsFrom(doubleType))
      res->setVariable(i - 2, (int)v.getDouble()); // a la rache cast from double to int
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
    TypePtr type = getClass();
    int index = type->findMemberVariable(string);
    if (index >= 0)
    {
      state.pushVariable(getVariable(index));
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
    TypePtr type = getClass();
    int index = type->findMemberVariable(string);
    if (index >= 0)
    {
      Variable variable = state.checkVariable(2);
      if (type->getMemberVariableType(index)->inheritsFrom(integerType))
        setVariable(index, juce::roundDoubleToInt(variable.toDouble()));
      else
        setVariable(index, variable);
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

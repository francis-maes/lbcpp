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
  {std::cout << "No RefCount Debug Info." << std::endl;}
#endif // LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS

#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
static CriticalSection objectsCountPerTypeLock;
static std::map<String, size_t > objectsCountPerType;
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION

Object::Object(ClassPtr thisClass)
  : thisClass(thisClass)
{
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  ScopedLock _(objectsCountPerTypeLock);
  classNameUnderWhichThisIsKnown = thisClass ? thisClass->getName() : T("<Unknown Class: ") + getTypeName(typeid(*this)) + T(">");
  objectsCountPerType[classNameUnderWhichThisIsKnown]++;
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
}

void Object::setThisClass(ClassPtr thisClass)
{
  this->thisClass = thisClass;
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  ScopedLock _(objectsCountPerTypeLock);
  if (thisClass && classNameUnderWhichThisIsKnown.startsWith(T("<Unknown Class:")))
  {
    objectsCountPerType[classNameUnderWhichThisIsKnown]--;
    classNameUnderWhichThisIsKnown = thisClass->getName();
    objectsCountPerType[classNameUnderWhichThisIsKnown]++;
  }
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
}

Object::~Object()
{
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  if (classNameUnderWhichThisIsKnown.isNotEmpty())
  {
    ScopedLock _(objectsCountPerTypeLock);
    objectsCountPerType[classNameUnderWhichThisIsKnown]--;
  }
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
}

void Object::displayObjectAllocationInfo(std::ostream& ostr)
{
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  std::multimap<size_t, String> info;
  {
    ScopedLock _(objectsCountPerTypeLock);
    for (std::map<String, size_t>::const_iterator it = objectsCountPerType.begin(); it != objectsCountPerType.end(); ++it)
      info.insert(std::make_pair(it->second, it->first));
  }
  for (std::multimap<size_t, String>::const_reverse_iterator it = info.rbegin(); it != info.rend(); ++it)
    std::cout << it->first << " " << it->second << std::endl;
#else
  ostr << "No Object Allocation Info, enable LBCPP_DEBUG_OBJECT_ALLOCATION flag" << std::endl;
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
}

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
    //jassert(false);
    const_cast<Object* >(this)->thisClass = lbcpp::getType(getTypeName(typeid(*this)));
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
  jassert(index < getClass()->getNumMemberVariables());
  return getClass()->getMemberVariableValue(this, index);
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
  {return defaultToStringImplementation(true);}

String Object::defaultToStringImplementation(bool useShortString) const
{
  ClassPtr type = getClass();
  size_t n = type->getNumMemberVariables();
  String res;
  for (size_t i = 0; i < n; ++i)
  {
    Variable value = getVariable(i);
    /*
    if (value.exists())
    {
      String valueString = useShortString ? value.toShortString() : value.toString();
      if (valueString.isNotEmpty())
      {
        String name = type->getMemberVariableName(i);
        String shortName = type->getMemberVariableShortName(i);
        if (res.isNotEmpty())
          res += T(" ");
        if (shortName.isNotEmpty() && shortName.length() < name.length())
          res += T("-") + shortName;
        else
          res += T("--") + name;
        if (!value.isBoolean())
        {
          if (valueString.indexOfAnyOf(T(" \t\r\n")) >= 0)
            valueString = valueString.quoted();
          res += T(" ") + valueString;
        }
      }
    }*/
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

    size_t n = object->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = object->getVariable(i);
      if (v.isObject() && v.exists())
        getAllChildObjectsRecursively(v.getObject(), res);
    }

    ContainerPtr container = object.dynamicCast<Container>();
    if (container && container->getElementsType()->inheritsFrom(objectClass))
    {
      size_t n = container->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        ObjectPtr object = container->getElement(i).getObject();
        if (object)
          getAllChildObjectsRecursively(object, res);
      }
    }
  }
}

void Object::getAllChildObjects(std::set<ObjectPtr>& res) const
  {return getAllChildObjectsRecursively(refCountedPointerFromThis(this), res);}

/*
** Clone
*/
ObjectPtr Object::clone(ExecutionContext& context) const
{
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
  size_t n = type->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable variable = getVariable(i);
    if (!variable.isMissingValue())
      exporter.saveVariable(getVariableName(i), variable, getVariableType(i));
  }
}

bool Object::loadFromXml(XmlImporter& importer)
{
  ClassPtr thisClass = getClass();
  
  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), child, T("variable"))
  {
    String name = child->getStringAttribute(T("name"));
    if (name.isEmpty())
    {
      importer.errorMessage(T("Object::loadFromXml"), T("Could not find variable name"));
      return false;
    }
    int variableNumber = thisClass->findMemberVariable(name);
    if (variableNumber < 0)
    {
      importer.warningMessage(T("Object::loadFromXml"), T("Could not find variable ") + name.quoted() + T(" in class ") + thisClass->getName());
      continue;
    }
    TypePtr expectedType = thisClass->getMemberVariableType(variableNumber);
    jassert(expectedType);
    Variable value = importer.loadVariable(child, expectedType);
    if (value.exists())
    {
      if (!importer.getContext().checkInheritance(value, expectedType))
        return false;
      setVariable((size_t)variableNumber, value);
    }
  }
  return true;
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
{
 /* String arguments;
  int b = str.indexOfChar('(');
  if (b >= 0)
  {
    String name = str.substring(0, b).trim();
    if (name != getClassName() && name != getClass()->getShortName())
    {
      context.errorCallback(T("Unrecognized name: ") + name);
      return false;
    }
    int e = str.lastIndexOfChar(')');
    if (e < 0)
    {
      context.errorCallback(T("Unmatched parenthesis"));
      return false;
    }
    arguments = str.substring(b + 1, e);
    String remaining = str.substring(e + 1).trim();
    if (remaining.isNotEmpty())
      context.warningCallback(remaining.quoted() + T(" is beyond parenthesis => skipped"));
  }
  else
    arguments = str.trim();

  return arguments.isEmpty() || loadArgumentsFromString(context, arguments);*/
  return loadArgumentsFromString(context, str);
}

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
    context.errorCallback(T("Could not parse ") + str.quoted() + T(": too much tokens"));
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

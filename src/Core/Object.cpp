/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Object.h>
#include <lbcpp/Core/Variable.h>
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

  if (juce::atomicDecrementAndReturn(refCount) == 0)
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
  if (thisClass)
  {
    classNameUnderWhichThisIsKnown = thisClass->getName();
    objectsCountPerType[classNameUnderWhichThisIsKnown]++;
  }
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
}

void Object::setThisClass(ClassPtr thisClass)
{
  this->thisClass = thisClass;
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  ScopedLock _(objectsCountPerTypeLock);
  if (thisClass && classNameUnderWhichThisIsKnown.isEmpty())
  {
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
{
  ObjectPtr res = objectClass->create(defaultExecutionContext()).getObject();
  jassert(res);
  if (res)
  {
    jassert(res->getReferenceCount() == 2);
    res->decrementReferenceCounter();
  }
  return res;
}

size_t Object::getNumVariables() const
  {return getClass()->getObjectNumVariables();}

TypePtr Object::getVariableType(size_t index) const
  {return getClass()->getObjectVariableType(index);}

String Object::getVariableName(size_t index) const
  {return getClass()->getObjectVariableName(index);}

Variable Object::getVariable(size_t index) const
{
  jassert(index < getClass()->getObjectNumVariables());
  return getClass()->getObjectVariable(this, index);
}

void Object::setVariable(ExecutionContext& context, size_t index, const Variable& value)
{
  jassert(index < getClass()->getObjectNumVariables());
  getClass()->setObjectVariable(context, this, index, value);
}


/*
** to string
*/
String Object::toString() const
{
  String res = getClassName() + T("{");
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    String name = getVariableName(i);
    res += name + T(" = ") + getVariable(i).toShortString();
    if (i < n - 1)
      res += T(", ");
  }
  res += T("}");
  return res;
}

String Object::toShortString() const
{
  String str = toString();
  return str.containsChar('\n') ? String::empty : str;
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

int Object::compareVariables(ObjectPtr otherObject) const
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
    target->setVariable(context, i, getVariable(i));
}

ObjectPtr Object::deepClone(ExecutionContext& context) const
{
  ClassPtr thisClass = getClass();
  ObjectPtr res = clone(context);
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    TypePtr variableType = thisClass->getObjectVariableType(i);
    if (variableType->inheritsFrom(objectClass) && !variableType->inheritsFrom(typeClass))
    {
      ObjectPtr object = res->getVariable(i).getObject();
      if (object)
        res->setVariable(context, i, Variable(object->deepClone(context), variableType));
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
  size_t n = newType->getObjectNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    int sourceIndex = thisClass->findObjectVariable(newType->getObjectVariableName(i));
    if (sourceIndex >= 0)
    {
      Variable variable = getVariable((size_t)sourceIndex);
      if (!variable.exists())
        continue;

      TypePtr newVariableType = newType->getObjectVariableType(i);
      if (variable.isObject())
        res->setVariable(context, i, variable.getObject()->cloneToNewType(context, newVariableType));
      else
      {
        jassert(variable.getType() == newVariableType);
        res->setVariable(context, i, variable);
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
  size_t n = type->getObjectNumVariables();
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
    int variableNumber = thisClass->findObjectVariable(name);
    if (variableNumber < 0)
    {
      importer.warningMessage(T("Object::loadFromXml"), T("Could not find variable ") + name.quoted() + T(" in class ") + thisClass->getName());
      continue;
    }
    TypePtr expectedType = thisClass->getObjectVariableType(variableNumber);
    jassert(expectedType);
    Variable value = importer.loadVariable(child, expectedType);
    if (value.exists())
    {
      if (!importer.getContext().checkInheritance(value, expectedType))
        return false;
      setVariable(importer.getContext(), (size_t)variableNumber, value);
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
  XmlElement* xml = importer.getCurrentElement();
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    String name = getVariableName(i);
    if (xml->hasAttribute(name))
    {
      Variable var = Variable::createFromString(importer.getContext(), getVariableType(i), xml->getStringAttribute(name));
      if (!var.isMissingValue())
        setVariable(importer.getContext(), i, var);
    }
    else if (name != T("thisClass"))
      importer.warningMessage(T("Object::loadVariablesFromXmlAttributes"), T("No value for variable ") + name.quoted());
  }
  return true;
}

bool Object::loadFromString(ExecutionContext& context, const String& str)
{
  context.errorCallback(T("Object::loadFromString"), T("Not implemented"));
  return false;
}

void Object::saveToFile(ExecutionContext& context, const File& file) const
  {Variable(const_cast<Object* >(this)).saveToFile(context, file);}

ObjectPtr Object::createFromFile(ExecutionContext& context, const File& file)
  {return Variable::createFromFile(context, file).getObject();}

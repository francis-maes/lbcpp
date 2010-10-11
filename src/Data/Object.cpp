/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Object.h>
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include <fstream>
using namespace lbcpp;

int ReferenceCountedObject::numAccesses = 0;
#ifdef LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS
void ReferenceCountedObject::incrementReferenceCounter()
{
  juce::atomicIncrement(refCount);
  juce::atomicIncrement(numAccesses);
}

/** Decrements the object's reference count.  */
void ReferenceCountedObject::decrementReferenceCounter()
{
  juce::atomicIncrement(numAccesses);
  if (juce::atomicDecrementAndReturn(refCount) == 0)
    delete this;
}
#endif // LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS

extern void declareLBCppClasses();

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
    const_cast<Object* >(this)->thisClass = Class::get(getTypeName(typeid(*this)));
    jassert(thisClass);
  }
  return thisClass;
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

void Object::setVariable(size_t index, const Variable& value)
{
  jassert(index < getClass()->getObjectNumVariables());
  getClass()->setObjectVariable(this, index, value);
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
    res += name + T(" = ") + getVariable(i).toString();
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
ObjectPtr Object::clone() const
{
  Variable variable = Variable::create(getClass());
  jassert(variable.exists());
  ObjectPtr res = variable.getObject();
  clone(res);
  return res;
}

void Object::clone(ObjectPtr target) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
    target->setVariable(i, getVariable(i));
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
    if (value.exists() && !checkInheritance(value, expectedType))
      return false;
    setVariable((size_t)variableNumber, value);
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
      Variable var = Variable::createFromString(getVariableType(i), xml->getStringAttribute(name), importer.getCallback());
      if (!var.isMissingValue())
        setVariable(i, var);
    }
    else
      importer.warningMessage(T("Object::loadVariablesFromXmlAttributes"), T("No value for variable ") + name.quoted());
  }
  return true;
}

bool Object::loadFromString(const String& str, MessageCallback& callback)
{
  callback.errorMessage(T("Object::loadFromString"), T("Not implemented"));
  return false;
}

void Object::saveToFile(const File& file, MessageCallback& callback)
  {Variable(refCountedPointerFromThis(this)).saveToFile(file, callback);}

ObjectPtr Object::createFromFile(const File& file, MessageCallback& callback)
  {return Variable::createFromFile(file, callback).getObject();}

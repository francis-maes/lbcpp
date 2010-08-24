/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Object.h>
#include <lbcpp/Data/Variable.h>
#include <fstream>
using namespace lbcpp;

extern void declareLBCppCoreClasses();

String Object::getClassName() const
{
  if (thisClass)
    return thisClass->getName();
  else
    return lbcpp::toString(typeid(*this));
}

ClassPtr Object::getClass() const
{
  if (!thisClass)
  {
    const_cast<Object* >(this)->thisClass = Class::get(lbcpp::toString(typeid(*this)));
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
  VariableValue pthis(const_cast<Object* >(this));
  Variable res = getClass()->getObjectVariable(pthis, index);
  pthis.clearObject();
  return res;
}

void Object::setVariable(size_t index, const Variable& value)
{
  jassert(index < getClass()->getObjectNumVariables());
  VariableValue pthis(this);
  getClass()->setObjectVariable(pthis, index, value);
  pthis.clearObject();
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

String Object::getShortSummary() const
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

/*
** Create and load
*/
ObjectPtr Object::createFromStream(InputStream& istr, bool doLoading)
{
  String className;
  if (!read(istr, className))
  {
    error(T("Object::create"), T("Could not read class name"));
    return ObjectPtr();
  }
  if (className == T("__null__"))
    return ObjectPtr();
  ObjectPtr res; // FIXME
  //ObjectPtr res = Type::createInstance(className).getObject();
  if (res && doLoading && !res->load(istr))
    error(T("Object::create"), T("Could not load object of class ") + className);
  return res;
}

/*
** Save
*/
void Object::saveToStream(OutputStream& ostr) const
{
  ostr.writeString(getClassName());
  save(ostr);
}

/*
** Clone
*/
ObjectPtr Object::clone() const
{
  Variable variable = Variable::create(getClass());
  jassert(variable);
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
XmlElement* Object::variableToXml(size_t index) const
{
  ClassPtr type = getClass();
  Variable value = getVariable(index);
  jassert(index < type->getObjectNumVariables());
  TypePtr staticType = type->getObjectVariableType(index);
  if (value.isNil())
    value = Variable::missingValue(staticType);
  else
    jassert(value.getType()->inheritsFrom(staticType));
  return value.toXml(T("static"), type->getObjectVariableName(index));
}

void Object::saveToXml(XmlElement* xml) const
{
  ClassPtr type = getClass();
  size_t n = type->getObjectNumVariables();
  for (size_t i = 0; i < n; ++i)
    xml->addChildElement(variableToXml(i));
}

bool Object::loadFromXml(XmlElement* xml, ErrorHandler& callback)
{
  ClassPtr thisClass = getClass();
  
  for (XmlElement* child = xml->getFirstChildElement(); child; child = child->getNextElement())
    if (child->getTagName() == T("static"))
    {
      String name = child->getStringAttribute(T("name"));
      if (name.isEmpty())
      {
        callback.errorMessage(T("Object::loadFromXml"), T("Could not find variable name"));
        return false;
      }
      int variableNumber = thisClass->findObjectVariable(name);
      if (variableNumber < 0)
      {
        callback.warningMessage(T("Object::loadFromXml"), T("Could not find variable ") + name.quoted() + T(" in class ") + thisClass->getName());
        continue;
      }
      Variable value = Variable::createFromXml(child, callback);
      if (value && !checkInheritance(value, thisClass->getObjectVariableType(variableNumber)))
        return false;
      setVariable((size_t)variableNumber, value);
    }

  return true;
}

void Object::saveVariablesToXmlAttributes(XmlElement* xml) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
    xml->setAttribute(getVariableName(i), getVariable(i).toString());
}

bool Object::loadVariablesFromXmlAttributes(XmlElement* xml, ErrorHandler& callback)
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    String name = getVariableName(i);
    if (xml->hasAttribute(name))
    {
      Variable var = Variable::createFromString(getVariableType(i), xml->getStringAttribute(name), callback);
      if (!var.isMissingValue())
        setVariable(i, var);
    }
    else
      callback.warningMessage(T("Object::loadVariablesFromXmlAttributes"), T("No value for variable ") + name.quoted());
  }
  return true;
}

bool Object::loadFromString(const String& str, ErrorHandler& callback)
{
  callback.errorMessage(T("Object::loadFromString"), T("Not implemented"));
  return false;
}

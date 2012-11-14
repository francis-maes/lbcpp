/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/Lua/Lua.h>
using namespace lbcpp;

ClassPtr Vector::computeElementsCommonBaseType() const
{
  size_t n = getNumElements();
  if (n == 0)
    return objectClass;
  ClassPtr type = getElement(0)->getClass();
  for (size_t i = 1; i < n; ++i)
  {
    type = Class::findCommonBaseClass(type, getElement(i)->getClass());
    if (type == objectClass)
      break;
  }
  return type;
}

int Vector::findElement(const ObjectPtr& value) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (Object::equals(getElement(i), value))
      return (int)i;
  return -1;
}

ClassPtr Vector::getTemplateParameter(ClassPtr type)
{
  ClassPtr dvType = type->findBaseTypeFromTemplateName(T("Vector"));
  jassert(dvType && dvType->getNumTemplateArguments() == 1);
  ClassPtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

bool Vector::getTemplateParameter(ExecutionContext& context, ClassPtr type, ClassPtr& res)
{
  ClassPtr dvType = type->findBaseTypeFromTemplateName(T("Vector"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Vector"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 1);
  res = dvType->getTemplateArgument(0);
  return true;
}

string Vector::toShortString() const
{
  size_t n = getNumElements(); 
  if (n == 0)
    return T("<empty>");
  if (n < 10)
  {
    string res;
    for (size_t i = 0; i < n; ++i)
    {
      res += getElement(i)->toShortString();
      if (i < n - 1)
        res += T(", ");
    }
    return res;
  }
  else
    return string((int)n) + T(" elements...");
}

string Vector::toString() const
{
  ClassPtr type = getElementsType();
  size_t n = getNumElements();
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    string value;
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr element = getElement(i);
      if (!element)
        value += '_';
      else
        value += enumeration->getElement(EnumValue::get(element))->getOneLetterCode();
    }
    return value;
  }

  if (type->inheritsFrom(doubleClass))
  {
    string value;
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr element = getElement(i);
      if (!element)
        value += '_';
      else
        value += string(Double::get(element));
      if (i < n - 1)
        value += " ";
    }
    return value;
  }

  string res;
  for (size_t i = 0; i < n; ++i)
  {
    res += getElement(i)->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res;
}

bool Vector::loadFromXml(XmlImporter& importer)
{
  int size = importer.getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    importer.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + string(size));
    return false;
  }
  resize(size);
  if (!Object::loadFromXml(importer))
    return false;

  ClassPtr elementsType = getElementsType();
  juce::XmlElement* elementsActualType = importer.getCurrentElement()->getChildByName(T("elementsActualType"));
  if (elementsActualType)
  {
    importer.enter(elementsActualType);
    elementsType = importer.loadType(ClassPtr());
    importer.leave();
    if (!elementsType)
      return false;
  }

  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), child, T("element"))
  {
    int index = child->getIntAttribute(T("index"), -1);
    if (index < 0)
    {
      importer.errorMessage(T("Container::loadFromXml"), T("Invalid index for element: ") + string(index));
      return false;
    }
    
    ObjectPtr value = importer.loadObject(child, elementsType);
    setElement((size_t)index, value);
  }
  return true;
}

void Vector::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = getNumElements();
  exporter.setAttribute(T("size"), (int)n);
  ClassPtr elementsType = getElementsType();
  if (n > 1)
  {
    ClassPtr actualType = computeElementsCommonBaseType();
    if (elementsType != actualType)
    {
      exporter.enter(T("elementsActualType"));
      exporter.writeType(actualType);
      exporter.leave();
      elementsType = actualType;
    }
  }

  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr element = getElement(i);
    if (element)
      exporter.saveElement(i, element, elementsType);
  }
}

bool Vector::loadFromString(ExecutionContext& context, const string& stringValue)
{
  ClassPtr elementsType = getElementsType();
  StringArray tokens;
  tokens.addTokens(stringValue, T(","), T("\""));
  resize(tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
  {
    ObjectPtr variable = Object::createFromString(context, elementsType, tokens[i]);
    if (!variable)
      return false;
    setElement(i, variable);
  }
  return true;
}

void Vector::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  VectorPtr targetVector = target.staticCast<Vector>();
  targetVector->resize(getNumElements());
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    targetVector->setElement(i, getElement(i));
}

int Vector::compare(const ObjectPtr& otherObject) const
{
  if (otherObject.get() == this)
    return 0;
  if (otherObject.isInstanceOf<Vector>())
  {
    const VectorPtr& other = otherObject.staticCast<Vector>();
    size_t n = getNumElements();
    if (n != other->getNumElements())
      return (int)n - (int)other->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      int c = Object::compare(getElement(i), other->getElement(i));
      if (c != 0)
        return c;
    }
    return 0;
  }
  else
    return Object::compare(otherObject);
}

int Vector::__len(LuaState& state) const
{ 
  state.pushInteger(getNumElements());
  return 1;
}

int Vector::__newIndex(LuaState& state)
{
  if (!state.isInteger(1))
    return Object::__newIndex(state);

  int index = state.toInteger(1);
  if (index < 1 || index > (int)getNumElements())
    state.error("Invalid index in Container::set()");
  else
    setElement(index - 1, state.checkObject(2));
  return 0;
}

int Vector::__index(LuaState& state) const
{
  if (!state.isInteger(1))
    return Object::__index(state);

  int index = state.toInteger(1);
  if (index < 1 || index > (int)getNumElements())
  {
    state.error("Invalid index in Container::get()");
    return 0;
  }

  state.pushObject(getElement(index - 1));
  return 1;
}

int Vector::resize(LuaState& state)
{
  VectorPtr vector = state.checkObject(1, vectorClass()).staticCast<Vector>();
  int size = state.checkInteger(2);
  if (size >= 0)
    vector->resize((size_t)size);
  else
    state.error("Invalid size in Vector::resize()");
  return 0;
}

int Vector::append(LuaState& state)
{
  VectorPtr vector = state.checkObject(1, vectorClass()).staticCast<Vector>();
  ObjectPtr value = state.checkObject(2);
  vector->append(value);
  return 0;
}

/*
** BVector
*/
string BVector::toString() const
{
  string res = T("[");
  for (size_t i = 0; i < v.size(); ++i)
  {
    switch (v[i])
    {
    case 0: res += '-'; break;
    case 1: res += '+'; break;
    case 2: res += '?'; break;
    default: res += ' '; break;
    };
  }
  res += T("]");
  return res;
}  

size_t BVector::getSizeInBytes(bool recursively) const
  {return Object::getSizeInBytes(recursively) + sizeof (v) + v.size() * sizeof (unsigned char);}

/*
** IVector / DVector / SVector
*/
juce::int64 IVector::missingValue = 0x0FEEFEEEFEEEFEEELL;
double DVector::missingValue = doubleMissingValue;
string SVector::missingValue = T("<missing string>");

/*
** OVector
*/
ObjectPtr OVector::missingValue = ObjectPtr();

size_t OVector::getSizeInBytes(bool recursively) const
{
  size_t res = Object::getSizeInBytes(recursively);
  // all the objects are assumed to have the same size
  size_t sizePerObject = 0;
  if (recursively)
  {
    for (size_t i = 0; i < v.size(); ++i)
      if (v[i])
      {
        sizePerObject = v[i]->getSizeInBytes(recursively);
        break;
      }
  }
  else
    sizePerObject = sizeof (ObjectPtr);
  res += sizeof (v) + v.size() * sizePerObject;
  return res;
}

/*
** Vector Constructor Method
*/
VectorPtr lbcpp::vector(ClassPtr elementsType, size_t initialSize)
{
  jassert(elementsType);
  if (elementsType->inheritsFrom(booleanClass))
    return new BVector(elementsType, initialSize);
  else if (elementsType->inheritsFrom(integerClass))
    return new IVector(elementsType, initialSize);
  else if (elementsType->inheritsFrom(doubleClass))
    return new DVector(elementsType, initialSize);
  else if (elementsType->inheritsFrom(stringClass))
    return new SVector(elementsType, initialSize);
  else
    return new OVector(elementsType, initialSize);
}

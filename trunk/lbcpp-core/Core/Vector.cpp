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
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Lua/Lua.h>
using namespace lbcpp;

/*
** Vector
*/
bool Vector::checkType(const Variable& value) const
  {return checkInheritance(value, getElementsType());}

String Vector::toString() const
{
  TypePtr type = getElementsType();
  size_t n = getNumElements();
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String value;
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getElement(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += enumeration->getElement(variable.getInteger())->getOneLetterCode();
    }
    return value;
  }

  if (type->inheritsFrom(doubleType))
  {
    String value;
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getElement(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += String(variable.getDouble());
      if (i < n - 1)
        value += " ";
    }
    return value;
  }

  return Container::toString();
}

bool Vector::loadFromXml(XmlImporter& importer)
{
  int size = importer.getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    importer.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  resize(size);
  return Container::loadFromXml(importer);
}

bool Vector::loadFromString(ExecutionContext& context, const String& stringValue)
{
  TypePtr elementsType = getElementsType();
  StringArray tokens;
  tokens.addTokens(stringValue, T(","), T("\""));
  resize(tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
  {
    Variable variable = Variable::createFromString(context, elementsType, tokens[i]);
    if (!variable.exists())
      return false;
    setElement(i, variable);
  }
  return true;
}

void Vector::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  VectorPtr targetVector = target.staticCast<Vector>();
  targetVector->resize(getNumElements());
  Container::clone(context, targetVector);
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
  Variable value = state.checkVariable(2);
  vector->append(value);
  return 0;
}

/*
** GenericVector
*/
GenericVector::GenericVector(TypePtr elementsType, size_t initialSize)
  : Vector(genericVectorClass(elementsType))
{
  jassert(elementsType != topLevelType);
  if (initialSize)
    values.resize(initialSize, elementsType->getMissingValue());
}

size_t GenericVector::getNumElements() const
  {return values.size();}

Variable GenericVector::getElement(size_t index) const
{
  jassert(index < values.size());
  TypePtr elementsType = getElementsType();
  if (elementsType.isInstanceOf<Class>())
  {
    ObjectPtr res = values[index].getObjectPointer();
    return res ? Variable(res) : Variable::missingValue(elementsType);
  }
  else
    return Variable::copyFrom(elementsType, values[index]);
}

void GenericVector::setElement(size_t index, const Variable& value)
{
  jassert(value.getType()->inheritsFrom(getElementsType()));
  jassert(index < values.size());
  value.copyTo(values[index]);
}

void GenericVector::clear()
{
  TypePtr type = getElementsType();
  for (size_t i = 0; i < values.size(); ++i)
    type->destroy(values[i]);
  values.clear();
}

void GenericVector::reserve(size_t size)
  {values.reserve(size);}

void GenericVector::resize(size_t size)
  {values.resize(size, getElementsType()->getMissingValue());}

void GenericVector::prepend(const Variable& value)
{
  if (checkType(value))
  {
    values.insert(values.begin(), getElementsType()->getMissingValue());
    value.copyTo(values.front());
  }
}

void GenericVector::append(const Variable& value)
{
  if (checkType(value))
  {
    values.push_back(getElementsType()->getMissingValue());
    value.copyTo(values.back());
  }
}

void GenericVector::remove(size_t index)
{
  jassert(index < values.size());
  TypePtr type = getElementsType();
  type->destroy(values[index]);
  values.erase(values.begin() + index);
}

void GenericVector::saveToXml(XmlExporter& exporter) const
{
  size_t n = getNumElements();

  TypePtr type = getElementsType();
  exporter.setAttribute(T("size"), (int)n);

  // enumeration vectors: as text
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if ((enumeration && enumeration->hasOneLetterCodes()) || type->inheritsFrom(doubleType))
  {
    exporter.addTextElement(toString());
    return;
  }

  // long builtin-type vectors: binary encoding
  if (n > 1000 && (type->inheritsFrom(integerType) || type->inheritsFrom(doubleType) || type->inheritsFrom(booleanType)))
  {
    juce::MemoryBlock block(&values[0], (int)(sizeof (VariableValue) * values.size()));
    exporter.setAttribute(T("binary"), T("true"));
    exporter.addTextElement(block.toBase64Encoding());
    return;
  }

  // other vectors: encore into XML child elements (default implementation)
  Container::saveToXml(exporter);
}

bool GenericVector::loadFromXml(XmlImporter& importer)
{
  TypePtr type = getElementsType();
  jassert(type);
  int size = importer.getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    importer.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  values.resize(size, type->getMissingValue());

  if (importer.getBoolAttribute(T("binary")))
  {
    if (!type->inheritsFrom(integerType) && !type->inheritsFrom(doubleType) && !type->inheritsFrom(booleanType))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Unexpected type for binary encoding"));
      return false;
    }

    juce::MemoryBlock block;
    if (!block.fromBase64Encoding(importer.getAllSubText().trim()))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Could not decode base 64"));
      return false;
    }

    if (block.getSize() != (int)(sizeof (VariableValue) * values.size()))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Invalid data size: found ") + String(block.getSize())
        + T(" expected ") + String((int)(sizeof (VariableValue) * values.size())));
      return false;
    }
    memcpy(&values[0], block.getData(), block.getSize());
    return true;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String text = importer.getAllSubText().trim();
    if (text.length() != size)
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(text.length()));
      return false;
    }
    
    for (size_t i = 0; i < values.size(); ++i)
    {
      int j = enumeration->findElementByOneLetterCode(text[(int)i]);
      if (j >= 0)
        values[i] = VariableValue(j);
      else
      {
        if (text[(int)i] != '_')
          importer.warningMessage(T("Vector::loadFromXml"), String(T("Could not recognize one letter code '")) + text[(int)i] + T("'"));
        values[i] = enumeration->getMissingValue();
      }
    }
    return true;
  }

  if (type->inheritsFrom(doubleType))
  {
    String text = importer.getAllSubText().trim();
    StringArray tokens;
    tokens.addTokens(text, T(" \t\r\n"), NULL);
    tokens.removeEmptyStrings(true);
    if (tokens.size() != size)
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(tokens.size()));
      return false;
    }
    for (size_t i = 0; i < values.size(); ++i)
      if (tokens[(int)i] != T("_"))
      {
        Variable value = Variable::createFromString(importer.getContext(), doubleType, tokens[(int)i]);
        if (!value.exists())
          return false;
        values[i] = value.getDouble();
      }
    return true;
  }

  // default implementation  
  return Container::loadFromXml(importer);
}

size_t GenericVector::getSizeInBytes(bool recursively) const
{
  size_t res = Container::getSizeInBytes(recursively);
  return res + getNumElements() * sizeof (VariableValue);
}

/*
** BooleanVector
*/
BooleanVector::BooleanVector(size_t initialSize, bool initialValue)
  : Vector(booleanVectorClass), v(initialSize, initialValue ? 1 : 0)
{
}

BooleanVector::BooleanVector(size_t initialSize)
  : Vector(booleanVectorClass), v(initialSize, 2)
{
}

String BooleanVector::toString() const
{
  String res = T("[");
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

size_t BooleanVector::getSizeInBytes(bool recursively) const
  {return Object::getSizeInBytes(recursively) + sizeof (v) + v.size() * sizeof (unsigned char);}

size_t BooleanVector::getNumElements() const
  {return v.size();}

static inline Variable byteToBooleanVariable(unsigned char b)
  {return b < 2 ? Variable(b == 1, booleanType) : Variable::missingValue(booleanType);}

static inline unsigned char booleanVariableToByte(const Variable& v)
  {jassert(v.isBoolean()); return v.isMissingValue() ? 2 : (v.getBoolean() ? 1 : 0);}

Variable BooleanVector::getElement(size_t index) const
{
  jassert(index < v.size());
  unsigned char b = v[index];
  if (b == 2)
    return Variable::missingValue(newBooleanClass);
  else
    return Variable(new NewBoolean(b == 1), newBooleanClass);
}

void BooleanVector::setElement(size_t index, const Variable& value)
{
  if (value.isObject())
  {
    NewBooleanPtr boolean = value.getObjectAndCast<NewBoolean>();
    v[index] = (boolean ? (boolean->get() ? 1 : 0) : 2);
  }
  else if (checkInheritance(value, booleanType))
    v[index] = booleanVariableToByte(value);
}

void BooleanVector::reserve(size_t size)
  {v.reserve(size);}

void BooleanVector::resize(size_t size)
  {v.resize(size);}

void BooleanVector::clear()
  {v.clear();}

void BooleanVector::prepend(const Variable& value)
  {v.insert(v.begin(), booleanVariableToByte(value));}

void BooleanVector::append(const Variable& value)
  {v.push_back(booleanVariableToByte(value));}

void BooleanVector::remove(size_t index)
  {v.erase(v.begin() + index);}

/*
** IntegerVector
*/
juce::int64 IntegerVector::missingValue = 0x0FEEFEEEFEEEFEEELL;

IntegerVector::IntegerVector(TypePtr elementsType, size_t initialSize, juce::int64 initialValue)
  : Vector(integerVectorClass(elementsType)), v(initialSize, initialValue)
{
}

IntegerVector::IntegerVector(TypePtr elementsType, size_t initialSize)
  : Vector(integerVectorClass(elementsType)), v(initialSize, missingValue)
{
}

size_t IntegerVector::getNumElements() const
  {return v.size();}

Variable IntegerVector::getElement(size_t index) const
{
  jassert(index < v.size());
  if (v[index] == missingValue)
    return Variable::missingValue(newIntegerClass);
  else
  {
    ClassPtr elementsType = getElementsType();
    if (elementsType.isInstanceOf<Enumeration>())
      return new NewEnumValue(elementsType, (size_t)v[index]);
    else
      return new NewInteger(elementsType, v[index]);
  }
}

void IntegerVector::setElement(size_t index, const Variable& value)
{
  NewIntegerPtr integer = value.getObjectAndCast<NewInteger>();
  v[index] = (integer ? integer->get() : missingValue);
}

void IntegerVector::reserve(size_t size)
  {v.reserve(size);}

void IntegerVector::resize(size_t size)
  {v.resize(size);}

void IntegerVector::clear()
  {v.clear();}

void IntegerVector::prepend(const Variable& value)
  {v.insert(v.begin(), value.getObject().staticCast<NewInteger>()->get());}

void IntegerVector::append(const Variable& value)
  {v.push_back(value.getObject() ? value.getObject().staticCast<NewInteger>()->get() : missingValue);}

void IntegerVector::remove(size_t index)
  {v.erase(v.begin() + index);}

/*
** ObjectVector
*/
ObjectVector::ObjectVector(TypePtr elementsType, size_t initialSize)
  : Vector(objectVectorClass(elementsType)), objects(new std::vector<ObjectPtr>(initialSize)), ownObjects(true)
{
}

ObjectVector::ObjectVector(ClassPtr thisClass)
  : Vector(thisClass), objects(new std::vector<ObjectPtr>()), ownObjects(true)
{
}

ObjectVector::ObjectVector(const std::vector<ObjectPtr>& reference, TypePtr elementsType)
  : Vector(objectVectorClass(elementsType ? elementsType : (TypePtr)(reference.size() ? reference[0]->getClass() : objectClass))),
    objects(const_cast<std::vector<ObjectPtr>* >(&reference)), ownObjects(false)
{
}

ObjectVector::ObjectVector(std::vector<ObjectPtr>& reference, TypePtr elementsType)
  : Vector(objectVectorClass(elementsType ? elementsType : (TypePtr)(reference.size() ? reference[0]->getClass() : objectClass))),
    objects(&reference), ownObjects(false)
{
}

ObjectVector::ObjectVector() : objects(NULL), ownObjects(false)
{
}

ObjectVector::~ObjectVector()
{
  if (ownObjects)
  {
    jassert(objects);
    delete objects;
  }
}

void ObjectVector::clear()
  {objects->clear();}

void ObjectVector::reserve(size_t size)
{
  if (!objects)
  {
    objects = new std::vector<ObjectPtr>();
    ownObjects = true;
  }
  objects->reserve(size);
}

void ObjectVector::resize(size_t size)
{
  if (objects)
    objects->resize(size);
  else
  {
    objects = new std::vector<ObjectPtr>(size);
    ownObjects = true;
  }
}

void ObjectVector::prepend(const Variable& value)
  {objects->insert(objects->begin(), value.getObject());}

void ObjectVector::append(const Variable& value)
{
  if (!objects)
    reserve(5);
  objects->push_back(value.getObject());
}

void ObjectVector::remove(size_t index)
  {objects->erase(objects->begin() + index);}

size_t ObjectVector::getNumElements() const
  {return objects ? objects->size() : 0;}

Variable ObjectVector::getElement(size_t index) const
{
  jassert(index < objects->size());
  const ObjectPtr& res = (*objects)[index];
  TypePtr elementsType = res ? (TypePtr)res->getClass() : getElementsType();
  return Variable(res, elementsType);
}

void ObjectVector::setElement(size_t index, const Variable& value)
  {jassert(index < objects->size()); (*objects)[index] = value.getObject();}

size_t ObjectVector::getSizeInBytes(bool recursively) const
{
  size_t res = Object::getSizeInBytes(recursively);
  if (objects && ownObjects)
  {
    // all the objects are assumed to have the same size
    size_t sizePerObject = 0;
    if (recursively)
    {
      for (size_t i = 0; i < objects->size(); ++i)
        if ((*objects)[i])
        {
          sizePerObject = (*objects)[i]->getSizeInBytes(recursively);
          break;
        }
    }
    else
      sizePerObject = sizeof (ObjectPtr);
    res += sizeof (*objects) + objects->size() * sizePerObject;
  }
  return res;
}

/*
** Vector Constructor Method
*/
VectorPtr lbcpp::vector(TypePtr elementsType, size_t initialSize)
{
  jassert(elementsType);
  if (elementsType->inheritsFrom(booleanType) || elementsType->inheritsFrom(newBooleanClass))
    return booleanVector(initialSize);
  else if (elementsType->inheritsFrom(doubleType) || elementsType->inheritsFrom(newDoubleClass))
  {
    if (elementsType->inheritsFrom(newDoubleClass))
      return new DenseDoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType), initialSize);
    else
      return new DenseDoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, elementsType), initialSize);
  }
  else if (elementsType->inheritsFrom(newIntegerClass) || elementsType.isInstanceOf<Enumeration>())
    return integerVector(elementsType, initialSize);
  else if (elementsType->inheritsFrom(objectClass))
    return objectVector(elementsType, initialSize);
  else
    return genericVector(elementsType, initialSize);
}

/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Vector.h>
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
    String oneLetterCodes = enumeration->getOneLetterCodes();
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getElement(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += oneLetterCodes[variable.getInteger()];
    }
    return value;
  }

  if (type->inheritsFrom(doubleType()))
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
        value += ' ';
    }
    return value;
  }

  return Container::toString();
}

bool Vector::loadFromXml(XmlElement* xml, MessageCallback& callback)
{
  int size = xml->getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    callback.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  resize(size);
  return Container::loadFromXml(xml, callback);
}

/*
** GenericVector
*/
GenericVector::GenericVector(TypePtr elementsType, size_t initialSize)
  : Vector(genericVectorClass(elementsType))
{
  jassert(elementsType != topLevelType());
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
  if (checkType(value))
  {
    jassert(index < values.size());
    value.copyTo(values[index]);
  }
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
    values.insert(values.begin(), VariableValue());
    value.copyTo(values.front());
  }
}

void GenericVector::append(const Variable& value)
{
  if (checkType(value))
  {
    values.push_back(VariableValue());
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

void GenericVector::saveToXml(XmlElement* xml) const
{
  size_t n = getNumElements();

  TypePtr type = getElementsType();
  xml->setAttribute(T("size"), (int)n);

  if (n > 1000 && (type->inheritsFrom(integerType()) || type->inheritsFrom(doubleType()) || type->inheritsFrom(booleanType())))
  {
    juce::MemoryBlock block(&values[0], sizeof (VariableValue) * values.size());
    xml->setAttribute(T("binary"), T("true"));
    xml->addTextElement(block.toBase64Encoding());
    return;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if ((enumeration && enumeration->hasOneLetterCodes()) || type->inheritsFrom(doubleType()))
    xml->addTextElement(toString());
  else
    Container::saveToXml(xml);
}

bool GenericVector::loadFromXml(XmlElement* xml, MessageCallback& callback)
{
  TypePtr type = getElementsType();
  jassert(type);
  int size = xml->getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    callback.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  values.resize(size, type->getMissingValue());

  if (xml->getBoolAttribute(T("binary")))
  {
    if (!type->inheritsFrom(integerType()) && !type->inheritsFrom(doubleType()) && !type->inheritsFrom(booleanType()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Unexpected type for binary encoding"));
      return false;
    }

    juce::MemoryBlock block;
    if (!block.fromBase64Encoding(xml->getAllSubText().trim()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Could not decode base 64"));
      return false;
    }

    if (block.getSize() != (int)(sizeof (VariableValue) * values.size()))
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Invalid data size: found ") + String(block.getSize())
        + T(" expected ") + String((int)(sizeof (VariableValue) * values.size())));
      return false;
    }
    memcpy(&values[0], block.getData(), block.getSize());
    return true;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String text = xml->getAllSubText().trim();
    if (text.length() != size)
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(text.length()));
      return false;
    }
    
    String oneLetterCodes = enumeration->getOneLetterCodes();
    for (size_t i = 0; i < values.size(); ++i)
    {
      int j = oneLetterCodes.indexOfChar(text[i]);
      if (j >= 0)
        values[i] = VariableValue(j);
      else
      {
        if (text[i] != '_')
          callback.warningMessage(T("Vector::loadFromXml"), String(T("Could not recognize one letter code '")) + text[i] + T("'"));
        values[i] = enumeration->getMissingValue();
      }
    }
    return true;
  }

  if (type->inheritsFrom(doubleType()))
  {
    String text = xml->getAllSubText().trim();
    StringArray tokens;
    tokens.addTokens(text, T(" \t\r\n"), NULL);
    if (tokens.size() != size)
    {
      callback.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(tokens.size()));
      return false;
    }
    for (size_t i = 0; i < values.size(); ++i)
      if (tokens[i] != T("_"))
      {
        Variable value = Variable::createFromString(doubleType(), tokens[i], callback);
        if (!value)
          return false;
        values[i] = value.getDouble();
      }
    return true;
  }

  // default implementation  
  return Container::loadFromXml(xml, callback);
}

/*
** BooleanVector
*/
BooleanVector::BooleanVector(size_t initialSize)
  : Vector(booleanVectorClass())
{
  if (initialSize)
    v.resize(initialSize, false);
}

String BooleanVector::toString() const
{
  String res = T("[");
  for (size_t i = 0; i < v.size(); ++i)
    res += v[i] ? '+' : '-';
  res += T("]");
  return res;
}

size_t BooleanVector::getNumElements() const
  {return v.size();}

Variable BooleanVector::getElement(size_t index) const
  {jassert(index < v.size()); return v[index];}

void BooleanVector::setElement(size_t index, const Variable& value)
{
  if (checkInheritance(value, booleanType()))
    v[index] = value.getBoolean();
}

void BooleanVector::reserve(size_t size)
  {v.reserve(size);}

void BooleanVector::resize(size_t size)
  {v.resize(size);}

void BooleanVector::clear()
  {v.clear();}

void BooleanVector::prepend(const Variable& value)
  {v.insert(v.begin(), value.getBoolean());}

void BooleanVector::append(const Variable& value)
  {v.push_back(value.getBoolean());}

void BooleanVector::remove(size_t index)
  {v.erase(v.begin() + index);}

/*
** ObjectVector
*/
ObjectVector::ObjectVector(TypePtr elementsType, size_t initialSize)
  : Vector(objectVectorClass(elementsType)), objects(initialSize)
{
}

void ObjectVector::clear()
  {objects.clear();}

void ObjectVector::reserve(size_t size)
  {objects.reserve(size);}

void ObjectVector::resize(size_t size)
  {objects.resize(size);}

void ObjectVector::prepend(const Variable& value)
  {objects.insert(objects.begin(), value.getObject());}

void ObjectVector::append(const Variable& value)
  {objects.push_back(value.getObject());}

void ObjectVector::remove(size_t index)
  {objects.erase(objects.begin() + index);}

size_t ObjectVector::getNumElements() const
  {return objects.size();}

Variable ObjectVector::getElement(size_t index) const
{
  jassert(index < objects.size());
  ObjectPtr res = objects[index];
  return res ? Variable(res) : Variable::missingValue(getElementsType());
}

void ObjectVector::setElement(size_t index, const Variable& value)
  {jassert(index < objects.size()); objects[index] = value.getObject();}


/*
** VariableVector
*/
VariableVector::VariableVector(size_t initialSize)
  : Vector(variableVectorClass()), variables(initialSize)
{
}

void VariableVector::clear()
  {variables.clear();}

void VariableVector::reserve(size_t size)
  {variables.reserve(size);}

void VariableVector::resize(size_t size)
  {variables.resize(size);}

void VariableVector::prepend(const Variable& value)
  {variables.insert(variables.begin(), value);}

void VariableVector::append(const Variable& value)
  {variables.push_back(value);}

void VariableVector::remove(size_t index)
  {variables.erase(variables.begin() + index);}

TypePtr VariableVector::getElementsType() const
  {return variableType();}

size_t VariableVector::getNumElements() const
  {return variables.size();}

Variable VariableVector::getElement(size_t index) const
  {jassert(index < variables.size()); return variables[index];}

void VariableVector::setElement(size_t index, const Variable& value)
  {jassert(index < variables.size()); variables[index] = value;}

/*
** Vector Constructor Method
*/
VectorPtr lbcpp::vector(TypePtr elementsType, size_t initialSize)
{
  if (elementsType->inheritsFrom(booleanType()))
    return booleanVector(initialSize);
  else if (elementsType->inheritsFrom(objectClass()))
    return objectVector(elementsType, initialSize);
  else if (elementsType == variableType())
    return variableVector(initialSize);
  else
    return genericVector(elementsType, initialSize);
}

/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VECTOR_H_
# define LBCPP_OBJECT_VECTOR_H_

# include "Container.h"

namespace lbcpp
{

class Vector : public Container
{
public:
  Vector(TypePtr elementsType, size_t initialSize = 0);
  Vector() {}

  virtual ~Vector()
    {clear();}

  virtual String toString() const;

  virtual TypePtr getElementsType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}

  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);

  void reserve(size_t size)
    {values.reserve(size);}

  void resize(size_t size);

  void clear();
  void prepend(const Variable& value);
  void append(const Variable& value);
  void remove(size_t index);

protected:
  std::vector<VariableValue> values;
  
  bool checkType(const Variable& value) const;
};

typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

extern ClassPtr vectorClass(TypePtr elementsType);

class BooleanVector : public Container
{
public:
  BooleanVector(size_t initialSize = 0)
    : v(initialSize, false) {}

  virtual String toString() const
  {
    String res = T("[");
    for (size_t i = 0; i < v.size(); ++i)
      res += v[i] ? '+' : '-';
    res += T("]");
    return res;
  }

  virtual TypePtr getElementsType() const
    {return booleanType();}

  virtual size_t getNumElements() const
    {return v.size();}

  virtual Variable getElement(size_t index) const
    {jassert(index < v.size()); return v[index];}

  virtual void setElement(size_t index, const Variable& value)
  {
    if (checkInheritance(value, booleanType()))
      v[index] = value.getBoolean();
  }

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value;}

  bool get(size_t index) const
    {jassert(index < v.size()); return v[index];}

protected:
  std::vector<bool> v;
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

template<class ImplementationType, class ObjectType>
class BuiltinVector : public Container
{
public:
  BuiltinVector(size_t initialSize, const ImplementationType& defaultValue)
    : values(initialSize, defaultValue) {}
  BuiltinVector(const std::vector<ImplementationType>& values)
    : values(values) {}
  BuiltinVector() {}

  virtual TypePtr getElementsType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumElements() const
    {return values.size();}

  virtual Variable getElement(size_t index) const
  {
    if (values[index].exists())
      return ObjectPtr(new ObjectType(values[index]));
    else
      return Variable::missingValue(getElementsType());
  }

  virtual void setElement(size_t index, const Variable& value)
  {
    ReferenceCountedObjectPtr<ObjectType> v = value.getObjectAndCast<ObjectType>();
    if (v)
      values[index] = ImplementationType();
    else
      values[index] = v->getValue();
  }

  virtual void saveToXml(XmlElement* xml) const
    {jassert(false);}
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback)
    {jassert(false); return false;}

  void reserve(size_t size)
    {values.reserve(size);}

  void clear()
    {values.clear();}

  void append(const ImplementationType& value)
    {values.push_back(value);}

protected:
  std::vector<ImplementationType> values;
};

class VariableVector : public Container
{
public:
  virtual String toString() const
    {return getClass()->getName();}

  virtual TypePtr getElementsType() const
    {return anyType();}

  virtual size_t getNumElements() const
    {return variables.size();}

  virtual Variable getElement(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

  virtual void setElement(size_t index, const Variable& value)
  {
    if (index >= variables.size())
      variables.resize(index + 1);
    variables[index] = value;
  }

  Variable& getElement(size_t index)
    {jassert(index < variables.size()); return variables[index];}

  void reserve(size_t size)
    {variables.reserve(size);}

  void clear()
    {variables.clear();}

  void append(const Variable& value)
    {variables.push_back(value);}

private:
  std::vector<Variable> variables;
};

typedef ReferenceCountedObjectPtr<VariableVector> VariableVectorPtr;

extern ClassPtr dynamicObjectClass();

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_

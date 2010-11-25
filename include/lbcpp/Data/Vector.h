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
  Vector(ClassPtr thisClass) : Container(thisClass) {}
  Vector() {}

  /*
  ** Vector
  */
  virtual void clear() = 0;
  virtual void reserve(size_t size) = 0;
  virtual void resize(size_t size) = 0;
  virtual void prepend(const Variable& value) = 0;
  virtual void append(const Variable& value) = 0;
  virtual void remove(size_t index) = 0;

  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  bool checkType(const Variable& value) const;
};

class GenericVector : public Vector
{
public:
  GenericVector(TypePtr elementsType, size_t initialSize);
  GenericVector() {}
  virtual ~GenericVector()
    {clear();}

  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);
  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  /*
  ** Object
  */
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

protected:
  std::vector<VariableValue> values;
};

class BooleanVector : public Vector
{
public:
  BooleanVector(size_t initialSize);
  BooleanVector() {}

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value;}

  bool get(size_t index) const
    {jassert(index < v.size()); return v[index];}

  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual TypePtr getElementsType() const
    {return booleanType;}

  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  /*
  ** Object
  */
  virtual String toString() const;

protected:
  std::vector<bool> v;
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

class ObjectVector : public Vector
{
public:
  ObjectVector(TypePtr elementsType, size_t initialSize);
  ObjectVector() {}
 
  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  ObjectPtr get(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

  template<class T>
  ReferenceCountedObjectPtr<T> getAndCast(size_t index) const
  {
    ObjectPtr res = get(index);
    return res.checkCast<T>(T("ObjectVector::getAndCast"));
  }

  void set(size_t index, ObjectPtr object)
    {objects[index] = object;}

protected:
  std::vector<ObjectPtr> objects;
};

typedef ReferenceCountedObjectPtr<ObjectVector> ObjectVectorPtr;

class VariableVector : public Vector
{
public:
  VariableVector(size_t initialSize);
  VariableVector() {}

  Variable& getElement(size_t index)
    {jassert(index < variables.size()); return variables[index];}

  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual TypePtr getElementsType() const;
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

protected:
  std::vector<Variable> variables;
};

typedef ReferenceCountedObjectPtr<VariableVector> VariableVectorPtr;

template<class ImplementationType, class ObjectType>
class BuiltinVector : public Vector
{
public:
  BuiltinVector(ClassPtr thisClass, size_t initialSize, const ImplementationType& defaultValue)
    : Vector(thisClass), values(initialSize, defaultValue) {}
  BuiltinVector(ClassPtr thisClass, const std::vector<ImplementationType>& values)
    : Vector(thisClass), values(values) {}
  BuiltinVector() {}

  virtual TypePtr getElementsType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumElements() const
    {return values.size();}

  virtual Variable getElement(size_t index) const
    {return new ObjectType(values[index]);}

  virtual void setElement(size_t index, const Variable& value)
    {values[index] = getImplementation(*(ExecutionContext* )0, value);} // FIXME : context

  virtual void saveToXml(XmlExporter& exporter) const
    {jassert(false);}

  virtual bool loadFromXml(XmlImporter& importer)
    {jassert(false); return false;}

  virtual void reserve(size_t size)
    {values.reserve(size);}

  virtual void resize(size_t size)
    {values.resize(size, ImplementationType());}

  virtual void clear()
    {values.clear();}

  virtual void prepend(const Variable& value)
    {values.insert(values.begin(), getImplementation(*(ExecutionContext* )0, value));} // FIXME : context

  virtual void append(const Variable& value)
    {values.push_back(getImplementation(*(ExecutionContext* )0, value));} // FIXME: context

  virtual void remove(size_t index)
    {values.erase(values.begin() + index);}

  size_t size() const
    {return values.size();}

  const ImplementationType& get(size_t index) const
    {jassert(index < values.size()); return values[index];}

  ImplementationType& get(size_t index)
    {jassert(index < values.size()); return values[index];}

  void set(size_t index, const ImplementationType& value)
    {jassert(index < values.size()); values[index] = value;}

  void append(const ImplementationType& value)
    {values.push_back(value);}

protected:
  std::vector<ImplementationType> values;

  static ImplementationType getImplementation(ExecutionContext& context, const Variable& value)
  {
    const ReferenceCountedObjectPtr<ObjectType>& v = value.getObjectAndCast<ObjectType>(context);
    return v ? v->getValue() : ImplementationType();
  }
};

extern ClassPtr vectorClass(TypePtr elementsType);
extern ClassPtr genericVectorClass(TypePtr elementsType);
extern ClassPtr objectVectorClass(TypePtr elementsType);
extern ClassPtr booleanVectorClass;
extern ClassPtr variableVectorClass;

extern VectorPtr vector(TypePtr elementsType, size_t initialSize = 0);
extern VectorPtr genericVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr booleanVector(size_t initialSize);
extern VectorPtr objectVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr variableVector(size_t initialSize);

template<class TT>
inline void variableToNative(ExecutionContext& context, std::vector<TT>& dest, const Variable& source)
{
  jassert(source.isObject());
  const VectorPtr& sourceVector = source.getObjectAndCast<Vector>(context);
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
      lbcpp::variableToNative(context, dest[i], sourceVector->getElement(i));
  }
  else
    dest.clear();
}

template<class TT>
inline void nativeToVariable(Variable& dest, const std::vector<TT>& source, TypePtr expectedType)
{
  dest = Variable::create(expectedType);
  const VectorPtr& destVector = dest.getObject().staticCast<Vector>();
  jassert(destVector);
  destVector->resize(source.size());
  TypePtr elementsType = destVector->getElementsType();
  for (size_t i = 0; i < source.size(); ++i)
  {
    Variable variable;
    nativeToVariable(variable, source[i], elementsType);
    if (variable.exists())
      destVector->setElement(i, variable);
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_

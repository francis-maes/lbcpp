/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VECTOR_H_
# define LBCPP_CORE_VECTOR_H_

# include "Container.h"
# include "Pair.h"

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
  virtual bool loadFromString(ExecutionContext& context, const String& stringValue);
  virtual bool loadFromXml(XmlImporter& importer);
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  /*
  ** Lua
  */
  static int resize(LuaState& state);
  static int append(LuaState& state);

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
  virtual size_t getSizeInBytes(bool recursively) const;

  const std::vector<VariableValue>& getValues() const
    {return values;}

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<VariableValue> values;
};

typedef ReferenceCountedObjectPtr<GenericVector> GenericVectorPtr;

class BooleanVector : public Vector
{
public:
  BooleanVector(size_t initialSize, bool initialValue);
  BooleanVector(size_t initialSize);
  BooleanVector() {}

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value ? 1 : 0;}

  bool get(size_t index) const
    {jassert(index < v.size() && v[index] != 2); return v[index] == 1;}

  bool flip(size_t index)
  {
    jassert(index < v.size() && v[index] != 2);
    v[index] = 1 - v[index];
    return v[index] == 1;
  }
  
  const unsigned char* getData() const
    {return &v[0];}

  unsigned char* getData()
    {return &v[0];}

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
  virtual size_t getSizeInBytes(bool recursively) const;

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<unsigned char> v; // 0 = false, 1 = true, 2 = missing
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

class IntegerVector : public Vector
{
public:
  IntegerVector(TypePtr elementsType, size_t initialSize, juce::int64 initialValue);
  IntegerVector(TypePtr elementsType, size_t initialSize);
  IntegerVector() {}

  // Vector
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  // Container
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  static juce::int64 missingValue;

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<juce::int64> v;
};

class ObjectVector : public Vector
{
public:
  ObjectVector(TypePtr elementsType, size_t initialSize);
  ObjectVector(ClassPtr thisClass);
  ObjectVector(const std::vector<ObjectPtr>& reference, TypePtr elementsType = TypePtr());
  ObjectVector(std::vector<ObjectPtr>& reference, TypePtr elementsType = TypePtr());
  ObjectVector();

  virtual ~ObjectVector();

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

  const ObjectPtr& get(size_t index) const
    {jassert(index < objects->size()); return (*objects)[index];}

  template<class T>
  const ReferenceCountedObjectPtr<T>& getAndCast(size_t index) const
    {const ObjectPtr& res = get(index); return res.staticCast<T>();}

  void set(size_t index, const ObjectPtr& object)
    {jassert(objects && index < objects->size()); (*objects)[index] = object;}

  template<class Type>
  void append(const ReferenceCountedObjectPtr<Type>& object)
    {objects->push_back(object);}

  template<class Type>
  void append(Type* object)
    {objects->push_back(ObjectPtr(object));}

  const std::vector<ObjectPtr>& getObjects() const
    {return *objects;}

  template<class T>
  const std::vector<ReferenceCountedObjectPtr<T> >& getObjectsAndCast() const
    {return *(const std::vector<ReferenceCountedObjectPtr<T> >* )objects;}

  std::vector<ObjectPtr>& getObjects()
    {return *objects;}

  virtual size_t getSizeInBytes(bool recursively) const;

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<ObjectPtr>* objects;
  bool ownObjects;
};

typedef ReferenceCountedObjectPtr<ObjectVector> ObjectVectorPtr;

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
    {values[index] = getImplementation(value);}

  virtual void reserve(size_t size)
    {values.reserve(size);}

  virtual void resize(size_t size)
    {values.resize(size, ImplementationType());}

  virtual void clear()
    {values.clear();}

  virtual void prepend(const Variable& value)
    {values.insert(values.begin(), getImplementation(value));}

  virtual void append(const Variable& value)
    {values.push_back(getImplementation(value));}

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

  void prepend(const ImplementationType& value)
    {values.insert(values.begin(), value);}

  void append(const ImplementationType& value)
    {values.push_back(value);}

  lbcpp_UseDebuggingNewOperator

protected:
  std::vector<ImplementationType> values;

  static ImplementationType getImplementation(const Variable& value)
  {
    const ReferenceCountedObjectPtr<ObjectType>& v = value.getObjectAndCast<ObjectType>();
    return v ? v->getValue() : ImplementationType();
  }
};

extern ClassPtr positiveIntegerPairVectorClass;

class PositiveIntegerPairVector : public BuiltinVector<impl::PositiveIntegerPair, PositiveIntegerPair>
{
public:
  typedef BuiltinVector<impl::PositiveIntegerPair, PositiveIntegerPair> BaseClass;

  PositiveIntegerPairVector(size_t length = 0)
    : BaseClass(positiveIntegerPairVectorClass, length, impl::PositiveIntegerPair(0, 0)) {}

  virtual TypePtr getElementsType() const
    {return positiveIntegerPairClass;}
  
  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr vectorClass(TypePtr elementsType = anyType);
extern ClassPtr genericVectorClass(TypePtr elementsType);
extern ClassPtr objectVectorClass(TypePtr elementsType);
extern ClassPtr booleanVectorClass;
extern ClassPtr integerVectorClass(TypePtr elementsType);

extern VectorPtr vector(TypePtr elementsType, size_t initialSize = 0);
extern VectorPtr genericVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr booleanVector(size_t initialSize);
extern VectorPtr integerVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr objectVector(TypePtr elementsType, size_t initialSize);

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
// I duplicated function for bool case because xcode convert bool to std::_Bit_reference Grrrr
inline void variableToNative(ExecutionContext& context, std::vector<bool>& dest, const Variable& source)
{
  jassert(source.isObject());
  const VectorPtr& sourceVector = source.getObjectAndCast<Vector>(context);
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
    {
      jassert(sourceVector->getElement(i).isBoolean());
      dest[i] = sourceVector->getElement(i).getBoolean();
    }
  }
  else
    dest.clear();
}

template<class TT>
inline Variable nativeToVariable(const std::vector<TT>& source, const TypePtr& expectedType)
{
  VectorPtr res = Vector::create(expectedType.staticCast<Class>()).staticCast<Vector>();

  res->resize(source.size());
  TypePtr elementsType = res->getElementsType();
  for (size_t i = 0; i < source.size(); ++i)
  {
    Variable variable = nativeToVariable(source[i], elementsType);
    if (variable.exists())
      res->setElement(i, variable);
  }
  return Variable(res, expectedType);
}

template<class TT>
inline void variableToNative(ExecutionContext& context, std::set<TT>& dest, const Variable& source)
{
  dest.clear();

  jassert(source.isObject());
  const ContainerPtr& sourceContainer = source.getObjectAndCast<Container>(context);
  if (sourceContainer)
  {
    size_t n = sourceContainer->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      TT value;
      lbcpp::variableToNative(context, value, sourceContainer->getElement(i));
      dest.insert(value);
    }
  }
}

template<class TT>
inline Variable nativeToVariable(const std::set<TT>& source, const TypePtr& expectedType)
{
  VectorPtr res = Vector::create(expectedType.staticCast<Class>()).staticCast<Vector>();

  res->resize(source.size());
  TypePtr elementsType = res->getElementsType();
  size_t i = 0;
  typedef typename std::set<TT>::const_iterator iterator;
  for (iterator it = source.begin(); it != source.end(); ++it, ++i)
  {
    Variable variable = nativeToVariable(*it, elementsType);
    if (variable.exists())
      res->setElement(i, variable);
  }
  return Variable(res, expectedType);
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VECTOR_H_

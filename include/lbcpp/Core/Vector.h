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
  virtual void prepend(const ObjectPtr& value) = 0;
  virtual void append(const ObjectPtr& value) = 0;
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

  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual TypePtr getElementsType() const
    {return booleanType;}

  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);

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

  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  // Container
  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);

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

  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);

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

extern ClassPtr vectorClass(TypePtr elementsType = anyType);
extern ClassPtr objectVectorClass(TypePtr elementsType);
extern ClassPtr booleanVectorClass;
extern ClassPtr integerVectorClass(TypePtr elementsType);

extern VectorPtr vector(TypePtr elementsType, size_t initialSize = 0);
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
      ObjectPtr elt = sourceVector->getElement(i);
      dest[i] = elt && NewBoolean::get(elt);
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
      res->setElement(i, variable.getObject());
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

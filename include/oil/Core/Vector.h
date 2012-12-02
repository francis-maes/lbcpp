/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VECTOR_H_
# define LBCPP_CORE_VECTOR_H_

# include "Boolean.h"
# include "Integer.h"
# include "Double.h"
# include "String.h"

namespace lbcpp
{

ClassPtr vectorClass(ClassPtr type1);
    
class Vector : public Object
{
public:
  Vector(ClassPtr thisClass) : Object(thisClass) {}
  Vector() {}

  ClassPtr getElementsType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}

  int findElement(const ObjectPtr& value) const;
  ClassPtr computeElementsCommonBaseType() const;

  static ClassPtr getTemplateParameter(ClassPtr type);
  static bool getTemplateParameter(ExecutionContext& context, ClassPtr type, ClassPtr& res);

  /*
  ** Vector
  */
  virtual void clear() = 0;
  virtual void reserve(size_t size) = 0;
  virtual void resize(size_t size) = 0;
  virtual void prepend(const ObjectPtr& value) = 0;
  virtual void append(const ObjectPtr& value) = 0;
  virtual void remove(size_t index) = 0;
  virtual size_t getNumElements() const = 0;
  virtual ObjectPtr getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const ObjectPtr& value) = 0;

  /*
  ** Object
  */
  virtual string toShortString() const;

  virtual bool loadFromString(ExecutionContext& context, const string& stringValue);
  virtual string toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  virtual int compare(const ObjectPtr& otherObject) const;

  /*
  ** Lua
  */
  virtual int __len(LuaState& state) const;
  virtual int __index(LuaState& state) const;
  virtual int __newIndex(LuaState& state);
  static int resize(LuaState& state);
  static int append(LuaState& state);

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr vectorClass(ClassPtr elementsType = objectClass);
extern VectorPtr vector(ClassPtr elementsType, size_t initialSize = 0);

template<class ExactClass, class NativeType>
class VectorT : public Vector
{
public:
  VectorT(ClassPtr elementsType, size_t initialSize, const NativeType& initialValue = ExactClass::missingValue)
    : Vector(vectorClass(elementsType)), v(initialSize, initialValue) {}
  VectorT() {}
  
  // Vector
  virtual void clear()
    {v.clear();}

  virtual void reserve(size_t size)
    {v.reserve(size);}

  virtual void resize(size_t size)
    {v.resize(size, ExactClass::missingValue);}

  virtual void prepend(const ObjectPtr& value)
    {v.insert(v.begin(), objectToNative(value));}

  virtual void append(const ObjectPtr& value)
    {v.push_back(objectToNative(value));}

  virtual void remove(size_t index)
    {v.erase(v.begin() + index);}

  virtual size_t getNumElements() const
    {return v.size();}

  virtual ObjectPtr getElement(size_t index) const
    {return nativeToObject(v[index]);}

  virtual void setElement(size_t index, const ObjectPtr& value)
    {jassert(index < v.size()); v[index] = objectToNative(value);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<ExactClass>()->v = v;}

  const NativeType& get(size_t index) const
    {jassert(index < v.size()); return v[index];}

  void set(size_t index, const NativeType& value)
    {jassert(index < v.size()); v[index] = value;}
  
  const std::vector<NativeType>& getNativeVector() const
    {return v;}

  std::vector<NativeType>& getNativeVector()
    {return v;}

  const NativeType* getDataPointer() const
    {return &v[0];}

  NativeType* getDataPointer()
    {return &v[0];}
  
protected:
  std::vector<NativeType> v;

  NativeType objectToNative(const ObjectPtr& value) const
    {return value ? ((ExactClass* )this)->objectToNativeImpl(value) : ExactClass::missingValue;}

  ObjectPtr nativeToObject(const NativeType& value) const
    {return value == ExactClass::missingValue ? ObjectPtr() : ((ExactClass* )this)->nativeToObjectImpl(value);}

  // to be overidded
  // NativeType missingValue
  NativeType objectToNativeImpl(const ObjectPtr& value) const
    {jassertfalse; return NativeType();}
  ObjectPtr nativeToObjectImpl(const NativeType& value) const
    {jassertfalse; return ObjectPtr();}
};

class BVector : public VectorT<BVector, unsigned char>
{
public:
  typedef VectorT<BVector, unsigned char> BaseClass;

  BVector(ClassPtr elementsType, size_t initialSize, unsigned char initialValue = 2)
    : BaseClass(elementsType, initialSize, initialValue) {}
  BVector(size_t initialSize = 0, unsigned char initialValue = 2)
    : BaseClass(booleanClass, initialSize, initialValue) {}

  virtual string toString() const;
  virtual size_t getSizeInBytes(bool recursively) const;

  enum {missingValue = 2};

  unsigned char objectToNativeImpl(const ObjectPtr& value) const
    {jassert(value); return Boolean::get(value) ? 1 : 0;}

  ObjectPtr nativeToObjectImpl(unsigned char value) const
    {jassert(value != 2); return new Boolean(BaseClass::getElementsType(), value == 1);}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<BVector> BVectorPtr;

class IVector : public VectorT<IVector, juce::int64>
{
public:
  typedef VectorT<IVector, juce::int64> BaseClass;

  IVector(ClassPtr elementsType, size_t initialSize, juce::int64 initialValue = missingValue)
    : BaseClass(elementsType, initialSize, initialValue) {}
  IVector(size_t initialSize = 0, juce::int64 initialValue = missingValue)
    : BaseClass(integerClass, initialSize, initialValue) {}

  static juce::int64 missingValue;

  juce::int64 objectToNativeImpl(const ObjectPtr& value) const
    {jassert(value); return Integer::get(value);}

  ObjectPtr nativeToObjectImpl(juce::int64 value) const
    {jassert(value != missingValue); return Integer::create(BaseClass::getElementsType(), value);}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<IVector> IVectorPtr;

class DVector : public VectorT<DVector, double>
{
public:
  typedef VectorT<DVector, double> BaseClass;

  DVector(ClassPtr elementsType, size_t initialSize, double initialValue = missingValue)
    : BaseClass(elementsType, initialSize, initialValue) {}
  DVector(size_t initialSize = 0, double initialValue = missingValue)
    : BaseClass(doubleClass, initialSize, initialValue) {}
  
  void append(double value)
    {v.push_back(value);}

  static double missingValue;

  double objectToNativeImpl(const ObjectPtr& value) const
    {jassert(value); return Double::get(value);}

  ObjectPtr nativeToObjectImpl(double value) const
    {jassert(value != missingValue); return Double::create(BaseClass::getElementsType(), value);}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<DVector> DVectorPtr;

class SVector : public VectorT<SVector, string>
{
public:
  typedef VectorT<SVector, string> BaseClass;

  SVector(ClassPtr elementsType, size_t initialSize, const string& initialValue = missingValue)
    : BaseClass(elementsType, initialSize, initialValue) {}
  SVector(size_t initialSize = 0, const string& initialValue = missingValue)
    : BaseClass(stringClass, initialSize, initialValue) {}

  static string missingValue;

  string objectToNativeImpl(const ObjectPtr& value) const
    {jassert(value); return String::get(value);}

  ObjectPtr nativeToObjectImpl(const string& value) const
    {jassert(value != missingValue); return String::create(BaseClass::getElementsType(), value);}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<SVector> SVectorPtr;

class OVector : public VectorT<OVector, ObjectPtr>
{
public:
  typedef VectorT<OVector, ObjectPtr> BaseClass;

  OVector(ClassPtr elementsType, size_t initialSize, ObjectPtr initialValue = ObjectPtr())
    : BaseClass(elementsType, initialSize, initialValue) {}
  OVector(size_t initialSize = 0, ObjectPtr initialValue = ObjectPtr())
    : BaseClass(objectClass, initialSize, initialValue) {}

  template<class T>
  const ReferenceCountedObjectPtr<T>& getAndCast(size_t index) const
    {const ObjectPtr& res = get(index); return res.staticCast<T>();}

  template<class Type>
  void append(const ReferenceCountedObjectPtr<Type>& object)
    {v.push_back(object);}

  template<class Type>
  void append(Type* object)
    {v.push_back(ObjectPtr(object));}

  template<class T>
  const std::vector<ReferenceCountedObjectPtr<T> >& getObjectsAndCast() const
    {return *(const std::vector<ReferenceCountedObjectPtr<T> >* )v;}

  virtual size_t getSizeInBytes(bool recursively) const;

  static ObjectPtr missingValue;

  ObjectPtr objectToNativeImpl(const ObjectPtr& value) const
    {return value;}

  ObjectPtr nativeToObjectImpl(const ObjectPtr& value) const
    {return value;}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<OVector> OVectorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VECTOR_H_

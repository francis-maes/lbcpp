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
# include "Boolean.h"
# include "Integer.h"
# include "Double.h"

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
  virtual ClassPtr getElementsType() const
    {return booleanClass;}

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
  IntegerVector(ClassPtr elementsType, size_t initialSize, juce::int64 initialValue);
  IntegerVector(ClassPtr elementsType, size_t initialSize);
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
  ObjectVector(ClassPtr elementsType, size_t initialSize);
  ObjectVector(ClassPtr thisClass);
  ObjectVector(const std::vector<ObjectPtr>& reference, ClassPtr elementsType = ClassPtr());
  ObjectVector(std::vector<ObjectPtr>& reference, ClassPtr elementsType = ClassPtr());
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

extern ClassPtr vectorClass(ClassPtr elementsType = objectClass);
extern ClassPtr objectVectorClass(ClassPtr elementsType);
extern ClassPtr booleanVectorClass;
extern ClassPtr integerVectorClass(ClassPtr elementsType);

extern VectorPtr vector(ClassPtr elementsType, size_t initialSize = 0);
extern VectorPtr booleanVector(size_t initialSize);
extern VectorPtr integerVector(ClassPtr elementsType, size_t initialSize);
extern VectorPtr objectVector(ClassPtr elementsType, size_t initialSize);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VECTOR_H_

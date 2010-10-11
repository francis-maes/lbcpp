/*-----------------------------------------.---------------------------------.
| Filename: TestMoveConstructor.cpp        | Test the move Constructor       |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/common.h>

class ReferenceCountedObject
{
public:
  /** Creates the reference-counted object (with an initial ref count of zero). */
  ReferenceCountedObject() : refCount(0) {}

  /** Destructor. */
  virtual ~ReferenceCountedObject()
  {
    // it's dangerous to delete an object that's still referenced by something else!
    jassert(refCount == 0 || refCount == staticAllocationRefCountValue);
  }

  /** Returns the object's current reference count. */
  int getReferenceCount() const
    {return refCount;}

  void setStaticAllocationFlag()
    {refCount = staticAllocationRefCountValue;}

protected:
  template<class T>
  friend class ReferenceCountedObjectPtr; /*!< */
  template<class T>
  friend struct StaticallyAllocatedReferenceCountedObjectPtr; /*!< */
  friend struct VariableValue;

  enum {staticAllocationRefCountValue = -0x7FFFFFFF};

  int refCount;              /*!< The object's reference count */

#ifdef JUCE_WIN32 // msvc compiler bug: the template friend class ReferenceCountedObjectPtr does not work
public:
#endif

  /** Increments the object's reference count.  */
  inline void incrementReferenceCounter()
    {juce::atomicIncrement(refCount);}

  /** Decrements the object's reference count.  */
  inline void decrementReferenceCounter()
  {
    if (juce::atomicDecrementAndReturn(refCount) == 0)
      delete this;
  }
};

/**
    Used to point to an object of type ReferenceCountedObject.

    It's wise to use a typedef instead of typing out the templated name
    each time - e.g.

    typedef ReferenceCountedObjectPtr<MyClass> MyClassPtr;

    @see ReferenceCountedObject
*/
template <class T>
class ReferenceCountedObjectPtr
{
public:
  /** Copies another pointer.

      This will increment the object's reference-count (if it is non-null).
  */
  template<class O>
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<O>& other) : ptr(static_cast<T* >(other.get()))
  {
    std::cout << "ReferenceCountedObjectPtr::copy-ctor-cast()" << std::endl;
    if (ptr != 0) cast(ptr).incrementReferenceCounter();
  }

  /** Copies another pointer.

      This will increment the object's reference-count (if it is non-null).
  */
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<T>& other) : ptr(other.get())
  {
    std::cout << "ReferenceCountedObjectPtr::copy-ctor()" << std::endl;
    if (ptr != 0) cast(ptr).incrementReferenceCounter();
  }

  ReferenceCountedObjectPtr(ReferenceCountedObjectPtr<T>&& other)
  {
    std::cout << "ReferenceCountedObjectPtr::move-ctor()" << std::endl;
    ptr = other.ptr;
    other.ptr = NULL;
  }

  /** Creates a pointer to an object.

      This will increment the object's reference-count if it is non-null.
  */
  ReferenceCountedObjectPtr(T* ptr) : ptr(ptr)
  {
    std::cout << "ReferenceCountedObjectPtr::native-ctor()" << std::endl;
    if (ptr != 0) cast(ptr).incrementReferenceCounter();
  }

  /** Creates a pointer to a null object. */
  ReferenceCountedObjectPtr() : ptr(NULL)
    {std::cout << "ReferenceCountedObjectPtr::empty-ctor()" << std::endl;}

  /** Destructor.

      This will decrement the object's reference-count, and may delete it if it
      gets to zero.
  */
  ~ReferenceCountedObjectPtr()
  {
    if (ptr) std::cout << "ReferenceCountedObjectPtr::dtor()" << std::endl;
    if (ptr) cast(ptr).decrementReferenceCounter();
  }

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<T>& other)
  {
    std::cout << "ReferenceCountedObjectPtr::assign()" << std::endl;
    changePtr(other.get()); return *this;
  }
  
  ReferenceCountedObjectPtr<T>& operator =(ReferenceCountedObjectPtr<T>&& other)
  {
    std::cout << "ReferenceCountedObjectPtr::move-assign()" << std::endl;
    if (ptr) cast(ptr).decrementReferenceCounter();
    ptr = other.ptr;
    other.ptr = NULL;
    return *this;
  }

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  template<class O>
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<O>& other)
  {
    std::cout << "ReferenceCountedObjectPtr::assign-cast()" << std::endl;
    changePtr(static_cast<T* >(other.get())); return *this;
  }

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  ReferenceCountedObjectPtr<T>& operator= (T* newT)
  {
    std::cout << "ReferenceCountedObjectPtr::assign-native()" << std::endl;
    changePtr(newT); return *this;
  }


  /** Sets this pointer to null.

    This will decrement the object's reference-count, and may delete it if it
      gets to zero.
  */
  void clear()
    {changePtr(NULL);}

  /** Returns true if this pointer is non-null. */
  bool exists() const
    {return ptr != NULL;}

  /** Returns true if this pointer is non-null. */
  operator bool () const
  {
    std::cout << "ReferenceCountedObjectPtr::cast-to-double()" << std::endl;
    return ptr != NULL;
  }

  /** Returns true if this pointer refers to the given object. */
  bool operator ==(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr == other.ptr;}

  /** Returns true if this pointer doesn't refer to the given object. */
  bool operator !=(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr != other.ptr;}

  /** Returns true if this pointer is smaller to the pointer of the given object. */
  bool operator <(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr < other.ptr;}

  /** Returns the object that this pointer references.

      The pointer returned may be zero, of course.
  */
  T* get() const
    {return ptr;}

  /** Returns the object that this pointer references.

      The pointer returned may be zero, of course.
  */
  T* operator -> () const
    {return ptr;}

  /** Returns a reference to the object that this pointer references. */
  T& operator * () const
    {jassert(ptr); return *ptr;}

  /** Dynamic cast the object that this pointer references.

    If the cast is invalid, this function returns a null pointer.
  */
  template<class O>
  inline ReferenceCountedObjectPtr<O> dynamicCast() const
  {
    if (ptr)
    {
      O* res = dynamic_cast<O* >(ptr);
      if (res)
        return ReferenceCountedObjectPtr<O>(res);
    }
    return ReferenceCountedObjectPtr<O>();
  }

  /** Static cast the object that this pointer references.

     This cast is unchecked, so be sure of what you are doing.
  */
  template<class O>
  inline ReferenceCountedObjectPtr<O> staticCast() const
  {
    if (ptr)
    {
      jassert(dynamic_cast<O* >(ptr));
      return ReferenceCountedObjectPtr<O>(static_cast<O* >(ptr));
    }
    return ReferenceCountedObjectPtr<O>();
  }

  /** Returns true if the object that this pointer references is an instance of the given class. */
  template<class O>
  inline bool isInstanceOf() const
    {return dynamic_cast<O* >(ptr) != NULL;}

private:
  T* ptr;                       /*!< */

  static inline ReferenceCountedObject& cast(T* ptr)
    {return **(ReferenceCountedObject** )(&ptr);}

  inline void changePtr(T* newPtr)
  {
    if (ptr != newPtr)
    {
      if (newPtr) cast(newPtr).incrementReferenceCounter();
      T* oldPtr = ptr;
      ptr = newPtr;
      if (oldPtr) cast(oldPtr).decrementReferenceCounter();
    }
  }
};

class Object : public ReferenceCountedObject
{
};

typedef ReferenceCountedObjectPtr<Object> ObjectPtr;

ObjectPtr f()
{
  ObjectPtr res = new Object();
  static int counter = 0;
  if (counter++ % 2)
    return ObjectPtr();
  else
    return res;
}

int main()
{
  ObjectPtr object = f();
  return 0;
}

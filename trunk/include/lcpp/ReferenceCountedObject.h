/*-----------------------------------------.---------------------------------.
| Filename: ReferenceCountedObject.h       | Base class for reference        |
| Author  : Francis Maes                   |  counted objects                |
| Started : 08/03/2009 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LCPP_REFERENCE_COUNTED_OBJECT_H_
# define LCPP_REFERENCE_COUNTED_OBJECT_H_

namespace lcpp
{

class ReferenceCountedObject
{
public:
  ReferenceCountedObject() : refCount(0) {}
  virtual ~ReferenceCountedObject()
    {assert(refCount == 0);}

  int getReferenceCount() const
    {return refCount;}

protected:
  template<class T>
  friend struct ReferenceCountedObjectPtr;
  template<class T>
  friend struct StaticallyAllocatedReferenceCountedObjectPtr;

  size_t refCount;

  inline void incrementReferenceCounter()
    {++refCount;}
  
  inline void decrementReferenceCounter()
  {
    assert(refCount > 0);
    --refCount;
    if (refCount == 0)
      delete this;
  }
};

template <class T>
struct ReferenceCountedObjectPtr
{
  template<class O>
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<O>& other) : ptr((T* )other.get())
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<T>& other) : ptr(other.get())
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  ReferenceCountedObjectPtr(T* ptr) : ptr(ptr)
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  ReferenceCountedObjectPtr() : ptr(NULL)
    {}
    
  ~ReferenceCountedObjectPtr()
    {if (ptr) cast(ptr).decrementReferenceCounter();}
  
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<T>& other)
    {changePtr(other.get()); return *this;}
  
  template<class O>
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<O>& other)
    {changePtr(static_cast<T* >(other.get())); return *this;}

  ReferenceCountedObjectPtr<T>& operator= (T* newT)
    {changePtr(newT); return *this;}
  
  void clear()
    {changePtr(NULL);}
    
  bool exists() const
    {return ptr != NULL;}
  
  operator bool () const
    {return ptr != NULL;}
  
  bool operator ==(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr == other.ptr;}
    
  bool operator !=(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr != other.ptr;}

  bool operator <(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr < other.ptr;}

  T* get() const
    {return ptr;}
    
  T* operator -> () const
    {return ptr;}
    
  T& operator * () const
    {assert(ptr); return *ptr;}

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
  
  template<class O>
  inline bool isInstanceOf() const
    {return dynamic_cast<O* >(ptr) != NULL;}

private:
  T* ptr;
  
  // enables the use of ReferenceCountedObjectPtrs on predeclared objects
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

template<class T>
struct StaticallyAllocatedReferenceCountedObjectPtr
{
  StaticallyAllocatedReferenceCountedObjectPtr(T& ptr) : ptr(ptr)
    {++(ptr.refCount);}
    
  ~StaticallyAllocatedReferenceCountedObjectPtr()
    {--(ptr.refCount);}
    
  operator ReferenceCountedObjectPtr<T>() const
    {return ReferenceCountedObjectPtr<T>(&ptr);}

  T* operator -> () const
    {return &ptr;}

private:
  T& ptr;
};

}; /* namespace lcpp */

#endif // !LCPP_REFERENCE_COUNTED_OBJECT_H_

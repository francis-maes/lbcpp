/*-----------------------------------------.---------------------------------.
| Filename: ReferenceCountedObject.h       | Base class for reference        |
| Author  : Francis Maes                   |  counted objects                |
| Started : 08/03/2009 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ReferenceCountedObject.h
**@author Francis MAES
**@date   Sat Jun 13 17:50:17 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_REFERENCE_COUNTED_OBJECT_H_
# define LBCPP_REFERENCE_COUNTED_OBJECT_H_

namespace lbcpp
{

/*!
** @class ReferenceCountedObject
** @brief
*/
class ReferenceCountedObject
{
public:
  /*!
  **
  **
  */
  ReferenceCountedObject() : refCount(0) {}

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~ReferenceCountedObject()
    {assert(refCount == 0);}

  /*!
  **
  **
  **
  ** @return
  */
  int getReferenceCount() const
    {return refCount;}

protected:
  template<class T>
  friend struct ReferenceCountedObjectPtr; /*!< */
  template<class T>
  friend struct StaticallyAllocatedReferenceCountedObjectPtr; /*!< */

  size_t refCount;              /*!< */

  /*!
  **
  **
  */
  inline void incrementReferenceCounter()
    {++refCount;}

  /*!
  **
  **
  */
  inline void decrementReferenceCounter()
  {
    assert(refCount > 0);
    --refCount;
    if (refCount == 0)
      delete this;
  }
};

/*!
** @struct ReferenceCountedObjectPtr
** @brief
*/
template <class T>
struct ReferenceCountedObjectPtr
{

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  template<class O>
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<O>& other) : ptr(static_cast<T* >(other.get()))
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<T>& other) : ptr(other.get())
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /*!
  **
  **
  ** @param ptr
  **
  ** @return
  */
  ReferenceCountedObjectPtr(T* ptr) : ptr(ptr)
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /*!
  **
  **
  **
  ** @return
  */
  ReferenceCountedObjectPtr() : ptr(NULL)
    {}

  /*!
  **
  **
  **
  ** @return
  */
  ~ReferenceCountedObjectPtr()
  {if (ptr) cast(ptr).decrementReferenceCounter();}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<T>& other)
  {changePtr(other.get()); return *this;}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  template<class O>
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<O>& other)
  {changePtr(static_cast<T* >(other.get())); return *this;}

  /*!
  **
  **
  ** @param newT
  **
  ** @return
  */
  ReferenceCountedObjectPtr<T>& operator= (T* newT)
    {changePtr(newT); return *this;}

  /*!
  **
  **
  */
  void clear()
    {changePtr(NULL);}

  /*!
  **
  **
  **
  ** @return
  */
  bool exists() const
    {return ptr != NULL;}

  /*!
  **
  **
  **
  ** @return
  */
  operator bool () const
    {return ptr != NULL;}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  bool operator ==(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr == other.ptr;}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  bool operator !=(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr != other.ptr;}

  /*!
  **
  **
  ** @param other
  **
  ** @return
  */
  bool operator <(const ReferenceCountedObjectPtr<T>& other) const
    {return ptr < other.ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  T* get() const
    {return ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  T* operator -> () const
    {return ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  T& operator * () const
    {assert(ptr); return *ptr;}

  /*!
  **
  **
  **
  ** @return
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

  /*!
  **
  **
  **
  ** @return
  */
  template<class O>
  inline ReferenceCountedObjectPtr<O> staticCast() const
  {
    if (ptr)
    {
      assert(dynamic_cast<O* >(ptr));
      return ReferenceCountedObjectPtr<O>(static_cast<O* >(ptr));
    }
    return ReferenceCountedObjectPtr<O>();
  }

  /*!
  **
  **
  **
  ** @return
  */
  template<class O>
  inline bool isInstanceOf() const
    {return dynamic_cast<O* >(ptr) != NULL;}

private:
  T* ptr;                       /*!< */

  // enables the use of ReferenceCountedObjectPtrs on predeclared objects
  /*!
  **
  **
  ** @param ptr
  **
  ** @return
  */
  static inline ReferenceCountedObject& cast(T* ptr)
    {return **(ReferenceCountedObject** )(&ptr);}

  /*!
  **
  **
  ** @param newPtr
  */
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

/*!
** @struct StaticallyAllocatedReferenceCountedObjectPtr
** @brief
*/
template<class T>
struct StaticallyAllocatedReferenceCountedObjectPtr
{
  /*!
  **
  **
  ** @param ptr
  **
  ** @return
  */
  StaticallyAllocatedReferenceCountedObjectPtr(T& ptr) : ptr(ptr)
    {++(ptr.refCount);}

  /*!
  **
  **
  **
  ** @return
  */
  ~StaticallyAllocatedReferenceCountedObjectPtr()
    {--(ptr.refCount);}

  /*!
  **
  **
  **
  ** @return
  */
  operator ReferenceCountedObjectPtr<T>() const
    {return ReferenceCountedObjectPtr<T>(&ptr);}

  /*!
  **
  **
  **
  ** @return
  */
  T* operator -> () const
    {return &ptr;}

private:
  T& ptr;                       /*!< */
};

}; /* namespace lbcpp */

#endif // !LBCPP_REFERENCE_COUNTED_OBJECT_H_

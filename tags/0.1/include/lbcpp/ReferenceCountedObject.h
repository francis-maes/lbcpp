/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

/**
    Adds reference-counting to an object.

    To add reference-counting to a class, derive it from this class, and
    use the ReferenceCountedObjectPtr class to point to it.

    e.g. @code
    class MyClass : public ReferenceCountedObject
    {
    public:
        void foo();
    };
    typedef ReferenceCountedObjectPtr<MyClass> MyClassPtr;

    MyClassPtr p = new MyClass();
    MyClassPtr p2 = p;
    p = MyClassPtr();
    p2->foo();
    @endcode

    Once a new ReferenceCountedObject has been assigned to a pointer, be
    careful not to delete the object manually.

    @see ReferenceCountedObjectPtr
*/
class ReferenceCountedObject
{
public:
  /** Creates the reference-counted object (with an initial ref count of zero). */
  ReferenceCountedObject() : refCount(0) {}

  /** Destructor. */
  virtual ~ReferenceCountedObject()
  {
    // it's dangerous to delete an object that's still referenced by something else!
    assert(refCount == 0);
  }

  /** Returns the object's current reference count. */
  int getReferenceCount() const
    {return refCount;}

protected:
  template<class T>
  friend struct ReferenceCountedObjectPtr; /*!< */
  template<class T>
  friend struct StaticallyAllocatedReferenceCountedObjectPtr; /*!< */

  size_t refCount;              /*!< The object's reference count */

  /** Increments the object's reference count.  */
  inline void incrementReferenceCounter()
    {++refCount;}

  /** Decrements the object's reference count.  */
  inline void decrementReferenceCounter()
  {
    assert(refCount > 0);
    --refCount;
    if (refCount == 0)
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
struct ReferenceCountedObjectPtr
{
  /** Copies another pointer.

      This will increment the object's reference-count (if it is non-null).
  */
  template<class O>
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<O>& other) : ptr(static_cast<T* >(other.get()))
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /** Copies another pointer.

      This will increment the object's reference-count (if it is non-null).
  */
  ReferenceCountedObjectPtr(const ReferenceCountedObjectPtr<T>& other) : ptr(other.get())
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /** Creates a pointer to an object.

      This will increment the object's reference-count if it is non-null.
  */
  ReferenceCountedObjectPtr(T* ptr) : ptr(ptr)
    {if (ptr != 0) cast(ptr).incrementReferenceCounter();}

  /** Creates a pointer to a null object. */
  ReferenceCountedObjectPtr() : ptr(NULL)
    {}

  /** Destructor.

      This will decrement the object's reference-count, and may delete it if it
      gets to zero.
  */
  ~ReferenceCountedObjectPtr()
    {if (ptr) cast(ptr).decrementReferenceCounter();}

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<T>& other)
    {changePtr(other.get()); return *this;}

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  template<class O>
  ReferenceCountedObjectPtr<T>& operator =(const ReferenceCountedObjectPtr<O>& other)
    {changePtr(static_cast<T* >(other.get())); return *this;}

  /** Changes this pointer to point at a different object.

      The reference count of the old object is decremented, and it might be
      deleted if it hits zero. The new object's count is incremented.
  */
  ReferenceCountedObjectPtr<T>& operator= (T* newT)
    {changePtr(newT); return *this;}

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
    {return ptr != NULL;}

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
    {assert(ptr); return *ptr;}

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
      assert(dynamic_cast<O* >(ptr));
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

}; /* namespace lbcpp */

#endif // !LBCPP_REFERENCE_COUNTED_OBJECT_H_

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
| Filename: ObjectContainer.h              | Object RandomAccess Containers  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 15:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CONTAINER_H_
# define LBCPP_OBJECT_CONTAINER_H_

# include "../ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class ObjectContainer
** @brief Base class for object containers with random access.
** @see ObjectStream
*/
class ObjectContainer : public NameableObject
{
public:
  ObjectContainer(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  /**
  ** Returns the class name of the objects contained by this stream.
  **
  ** If objects from multiple classes are mixed in this stream, this
  ** functions returns the highest base-class that is common between
  ** these classes.
  **
  ** @return content class name (String).
  */
  virtual String getContentClassName() const
    {return "Object";}

  /**
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const = 0;

  /**
  ** Element getter (by index).
  **
  ** @param index : element index (must be from range [0, size()[).
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const = 0;

  virtual bool hasObject(size_t index) const
    {return get(index) != ObjectPtr();}


  // only for read/write containers:
  virtual void resize(size_t newSize)
    {jassert(false);}

  virtual void set(size_t index, ObjectPtr object)
    {jassert(false);}

  /**
  ** Checks if the container is empty or not.
  **
  ** @return size == 0.
  */
  bool empty() const
    {return size() == 0;}

  int findObject(ObjectPtr object) const
  {
    for (size_t i = 0; i < size(); ++i)
      if (get(i) == object)
        return (int)i;
    return -1;
  }

  /**
  ** Gets an element from the container and casts it.
  **
  ** @param index : element index (must be from range [0, size()[).
  **
  ** @return an object pointer.
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> getAndCast(size_t index) const
  {
    ObjectPtr res = get(index);
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /**
  ** Converts to a VectorObjectContainer.
  **
  ** VectorObjectContainer is represented internally thanks to
  ** a 'std::vector<ObjectPtr>'. It enables full read/write access
  ** and can be used to iterate efficiently over all elements.
  **
  ** @return a VectorObjectContainer pointer.
  */
  virtual VectorObjectContainerPtr toVector() const;

  /**
  ** Creates an object stream from the current container.
  **
  ** @return an object stream pointer.
  ** @see ObjectStream
  */
  virtual ObjectStreamPtr toStream() const;

  /**
  ** Creates a graph sequential representation of the current container.
  **
  ** @return an graph of objects.
  ** @see ObjectGraph
  */
  virtual ObjectGraphPtr toGraph() const;

  /**
  ** Applies a @a function to all item contained into the object container.
  **
  ** @param function : function to apply to the current object container.
  ** @param lazyCompute : specify if the function is applicated
  ** immediately on all container items (True), or at each call
  ** to get() or getAndCast() (False).
  **
  ** @return an object container pointer.
  ** @see ObjectFunction
  */
  ObjectContainerPtr apply(ObjectFunctionPtr function, bool lazyCompute = true);

public:
  /**
  ** Creates a randomized version of a dataset.
  **
  ** The randomization is lazy: only a mapping between old
  ** indices and new ones is stored and no data is copied.
  **
  ** @returns the randomized ObjectContainer that has an internal
  ** reference to this one.
  */
  ObjectContainerPtr randomize();

  /**
  ** Creates a set where each instance is duplicated multiple times.
  **
  ** This function creates a new ObjectContainer that references this
  ** one. Each element of this ObjectContainer is duplicated @a count
  ** times in the new ObjectContainer. For example, if this ObjectContainer contains
  ** three examples A, B and C, duplicate(4) will contain A, B, C, A,
  ** B, C, A, B, C, A, B and C.
  **
  ** The duplication is lazy and does not consume any memory nor cpu
  ** time.
  **
  ** Duplicating an ObjectContainer can for example be useful to perform
  ** stochastic training. The following code shows how to perform
  ** 10 passes on the data with a single call to the trainStochastic function:
  ** @code
  ** ClassifierPtr aStochasticClassifier = ...;
  ** ObjectContainerPtr trainingData = ...;
  **
  ** ObjectContainerPtr trainingData10Passes = trainingData->duplicate(10);
  ** if (randomize)
  **  trainingData10Passes = trainingData10Passes->randomize();
  ** aStochasticClassifier->trainStochastic(trainingData10Passes);
  ** @endcode
  **
  ** Note that one can alternatively use a loop that calls ten
  ** times the trainStochastic function.
  **
  ** @param count : The number of times the instance set is
  ** duplicated.
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr duplicate(size_t count);

  /**
  ** Creates a fold of instances.
  **
  ** This function is typically used in the context of
  ** cross-validation.
  ** The instances are splitted into @a numFolds groups. This
  ** method construct a new ObjectContainer that refers to the
  ** @a fold 's group.
  **
  ** @param fold : the number of the fold. This number should be in
  ** the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr fold(size_t fold, size_t numFolds);

  /**
  ** Excludes a fold of instances.
  **
  ** This function compute the complementary set of the one
  ** returned by fold().
  ** The returned ObjectContainer contains all instances except the
  ** one from the @a fold 's group.
  **
  ** @param fold : the number of the fold that will be excluded from
  ** data. This number should be in the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr invFold(size_t fold, size_t numFolds);

  /**
  ** Selects a range of instances.
  **
  ** This functions creates an instance set that refer to a range
  ** of examples of this instance set. The range is defined through
  ** the begin and end indices that respectively correspond to the
  ** first instance inside the range and the first instance below the
  ** range.
  **
  **  @param begin : the index of the first instance inside the
  **  range.
  **  @param end : the index of the first instance below the range.
  **
  **  @returns a new ObjectContainer that refers to this internally
  **  and whose size is (end - begin).
  **  @see fold
  */
  ObjectContainerPtr range(size_t begin, size_t end);

  /** Excludes a range of instances.

      This functions creates the complementary instance set
      of range(begin, end), <i>i.e.</i> the instance set
      containing all the examples of this except those belonging
      to the range (begin, end).

      @param begin The index of the first instance inside the range to exclude.
      @param end The index of the first instance below the range to exclude.
      @returns a new InstanceSet that refers to this internally and
        whose size is (size() - (end - begin)).
      @see range, invFold
    */
    ObjectContainerPtr invRange(size_t begin, size_t end);
};

/**
** Appends two object containers.
**
** This operation is implemented by using
** references and does not perform any data copy.
**
** @param left : first object container.
** @param right : second object container.
**
** @return a new ObjectContainer containing all the elements
** of @a left followed by all the elements of @a right.
*/
extern ObjectContainerPtr append(ObjectContainerPtr left, ObjectContainerPtr right);

/**
** @class VectorObjectContainer
** @brief An object container that provides read/write access.
*/
class VectorObjectContainer : public ObjectContainer
{
public:
  /**
  ** Constructor.
  **
  ** @param objects : object vector.
  ** @param contentClassName : content class name.
  */
  VectorObjectContainer(const std::vector<ObjectPtr>& objects, const String& contentClassName = "Object")
    : objects(objects), contentClassName(contentClassName) {}

  /**
  ** Constructor.
  **
  ** @param contentClassName : content class name.
  **
  ** @return a VectorObjectContainer.
  */
  VectorObjectContainer(const String& contentClassName = "Object")
    : contentClassName(contentClassName) {}

  /**
  ** Content class name getter.
  **
  ** @return content class name.
  */
  virtual String getContentClassName() const
    {return contentClassName;}

  /**
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const
    {return objects.size();}

  /**
  ** Element getter.
  **
  ** @param index : element index.
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

  // resize
  virtual void resize(size_t newSize)
    {objects.resize(newSize);}

  // set
  virtual void set(size_t index, ObjectPtr object)
    {jassert(index < objects.size()); objects[index] = object;}

  /**
  ** Returns a reference on itself.
  **
  ** @return a reference on itself.
  */
  virtual VectorObjectContainerPtr toVector() const
    {return VectorObjectContainerPtr(const_cast<VectorObjectContainer* >(this));}

  /**
  ** Reserves @a size space.
  **
  ** @param size : space to reserve.
  */
  void reserve(size_t size)
    {objects.reserve(size);}

  /**
  ** Appends an object to the container.
  **
  ** @param object : object to append to the container.
  */
  void append(ObjectPtr object)
    {objects.push_back(object);}

  void prepend(ObjectPtr object)
    {objects.insert(objects.begin(), object);}

  void clear()
    {objects.clear();}

  /**
  ** Data getter.
  **
  ** @return data vector.
  */
  const std::vector<ObjectPtr>& getData() const
    {return objects;}

  /**
  ** Data getter.
  **
  ** @return data vector.
  */
  std::vector<ObjectPtr>& getData()
    {return objects;}

protected:
  std::vector<ObjectPtr> objects; /*!< Object list.*/
  String contentClassName; /*!< Content class name. */

  virtual bool load(InputStream& istr)
    {return ObjectContainer::load(istr) && lbcpp::read(istr, objects) && lbcpp::read(istr, contentClassName);}

  virtual void save(OutputStream& ostr) const
    {ObjectContainer::save(ostr); lbcpp::write(ostr, objects); lbcpp::write(ostr, contentClassName);}
};


/**
** @class DecoratorObjectContainer
** @brief Base class for ObjectContainer decorators.
** This base class is used internally to implement
** the ObjectContainer::fold(), ObjectContainer::invFold(),
** ObjectContainer::range(), ObjectContaner::invRange() and
** ObjectContainer::randomize() functions.
*/
class DecoratorObjectContainer : public ObjectContainer
{
public:
  /**
  ** Constructor.
  **
  ** @param target : the decorated ObjectContainer.
  */
  DecoratorObjectContainer(const String& name, ObjectContainerPtr target)
    : ObjectContainer(name), target(target) {}

  DecoratorObjectContainer() {}

  /**
  ** Content class name getter.
  **
  ** @return content class name.
  */
  virtual String getContentClassName() const
    {return target->getContentClassName();}

  /**
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const
    {return target->size();}

  /**
  ** Gets an element.
  **
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const
    {return target->get(index);}

protected:
  ObjectContainerPtr target;    /*!< A pointer to the decorated ObjectContainer. */

protected:
  virtual bool load(InputStream& istr)
    {return ObjectContainer::load(istr) && lbcpp::read(istr, target);}

  virtual void save(OutputStream& ostr) const
    {ObjectContainer::save(ostr); lbcpp::write(ostr, target);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONTAINER_H_

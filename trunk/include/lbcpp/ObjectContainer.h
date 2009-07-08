/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainer.h              | Object RandomAccess Containers  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 15:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ObjectContainer.h
**@author Francis MAES
**@date   Fri Jun 12 19:11:19 2009
**
**@brief  Random acces object container base classe.
**
**
*/

#ifndef LBCPP_OBJECT_CONTAINER_H_
# define LBCPP_OBJECT_CONTAINER_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{


/*!
** @class ObjectContainer
** @brief Random acces object container base classe.
*/
class ObjectContainer : public Object
{
public:
  /*!
  ** Content class name getter.
  **
  ** @return the name of the base class which is common for all
  ** objects contained in this container .
  */
  virtual std::string getContentClassName() const
    {return "Object";}

  /*!
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const = 0;

  /*!
  ** Item getter (by index).
  **
  ** @param index : item index (must be from range [0, size()[).
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const = 0;

  /*!
  ** Checks if the container is empty or not.
  **
  ** @return size == 0.
  */
  bool empty() const
    {return size() == 0;}

  /*!
  ** Gets (by index) and casts item from the container.
  **
  ** @param index : item index (must be from range [0, size()[).
  **
  ** @return an object pointer.
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> getAndCast(size_t index) const
  {
    ObjectPtr res = get(index);
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /*!
  ** Converts to a VectorObjectContainer (based on a
  ** 'std::vector<ObjectPtr>' container).
  **
  ** @return a VectorObjectContainer pointer.
  */
  virtual VectorObjectContainerPtr toVector() const;

  /*!
  ** Creates an object stream from the current container.
  **
  ** @see ObjectStream
  ** @return an object stream pointer.
  */
  virtual ObjectStreamPtr toStream() const;

  /*!
  ** Applies a @a function to all item contained into the object container.
  **
  ** @param function : function to apply to the current object container.
  ** @param lazyCompute : specify if the function is applicated
  ** immediately on all container items (True), or at each call
  ** to get() or getAndCast() (False).
  **
  ** @see ObjectFunction
  **
  ** @return an object container pointer.
  */
  ObjectContainerPtr apply(ObjectFunctionPtr function, bool lazyCompute = true);

public:
  /*!
  ** Creates a randomized version of a dataset.
  **
  ** The randomization is lazy: only a mapping table between old
  ** indices and new ones is stored and no data is copied.
  **
  ** @returns the randomized ObjectContainer that has an internal
  ** reference to this one.
  */
  ObjectContainerPtr randomize();

  /*!
  ** Creates a set where each instance is duplicated multiple times.
  **
  ** This function creates a new ObjectContainer that references this
  ** one. Each element of this ObjectContainer is duplicated <i>count</i>
  ** times in the new ObjectContainer. For example, if this ObjectContainer contains
  ** three examples A, B and C, <i>duplicate(4)</i> will contain A, B, C, A,
  ** B, C, A, B, C, A, B and C.
  **
  ** The duplication is lazy and does not consume any memory nor cpu
  ** time.
  **
  ** Duplicating an ObjectContainer can for example be useful when
  ** training online learning machines. Indeed, with online learning,
  ** one call to LearningMachine::train performs a single pass on the
  ** dataset. The following code shows how to perform 10 passes on the
  ** data with a single call to the train function:
  ** @code
  ** LearningMachine anOnlineMachine = ...;
  ** ObjectContainer trainingData = ...;
  **
  ** ObjectContainer trainingData10Passes =
  ** trainingData.duplicate(10);
  ** if (randomize)
  **  trainingData10Passes = trainingData10Passes.randomize();
  ** anOnlineMachine.train(trainingData10Passes);
  ** @endcode
  **
  ** Note that one can alternatively use a loop that calls ten
  ** times the train function.
  **
  ** @param count : The number of times the instance set is
  ** duplicated.
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr duplicate(size_t count);

  /*!
  ** Creates a fold of instances.
  **
  ** This function is typically used in the context of
  ** cross-validation.
  ** The instances are splitted into <i>numFolds</i> groups. This
  ** method construct a new ObjectContainer that refers to the
  ** <i>fold</i>'s group.
  **
  ** @param fold : the number of the fold. This number should be in
  ** the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr fold(size_t fold, size_t numFolds);

  /*!
  ** Excludes a fold of instances.
  **
  ** This function compute the complementary set of the one
  ** returned by fold().
  ** The returned ObjectContainer contains all instances except the
  ** one from the <i>fold</i>'s group.
  **
  ** @param fold : the number of the fold that will be excluded from
  ** data. This number should be in the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new ObjectContainer that refers this one internally.
  */
  ObjectContainerPtr invFold(size_t fold, size_t numFolds);

  /*!
  ** Select a range.
  **
  ** @param begin : first item index.
  ** @param end : last item index.
  **
  ** @return an object container pointer of the items between indexes
  ** @a begin and @a end.
  */
  ObjectContainerPtr range(size_t begin, size_t end);

  /*!
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
  */
  ObjectContainerPtr invRange(size_t begin, size_t end);
};

/*!
** Appends two object containers.
**
** @param left : first object container.
** @param right : second object container.
**
** @return an object container pointer (left + right).
*/
extern ObjectContainerPtr append(ObjectContainerPtr left, ObjectContainerPtr right);


/*!
** @class VectorObjectContainer
** @brief Object container (std::vector).
*/
class VectorObjectContainer : public ObjectContainer
{
public:
  /*!
  ** Constructor.
  **
  ** @param objects : object vector.
  ** @param contentClassName : content class name.
  **
  ** @return a VectorObjectContainer.
  */
  VectorObjectContainer(const std::vector<ObjectPtr>& objects, const std::string& contentClassName = "Object")
    : objects(objects), contentClassName(contentClassName) {}

  /*!
  ** Constructor.
  **
  ** @param contentClassName : content class name.
  **
  ** @return a VectorObjectContainer.
  */
  VectorObjectContainer(const std::string& contentClassName = "Object")
    : contentClassName(contentClassName) {}

  /*!
  ** Content class name getter.
  **
  ** @return content class name.
  */
  virtual std::string getContentClassName() const
    {return contentClassName;}

  /*!
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const
    {return objects.size();}

  /*!
  ** Item getter.
  **
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const
    {assert(index < objects.size()); return objects[index];}

  /*!
  ** Returns a reference on itself.
  **
  ** @return a reference on itself.
  */
  virtual VectorObjectContainerPtr toVector() const
    {return VectorObjectContainerPtr(const_cast<VectorObjectContainer* >(this));}

  /*!
  ** Reserves @a size space.
  **
  ** @param size : space to reserve.
  */
  void reserve(size_t size)
    {objects.reserve(size);}

  /*!
  ** Appends @a object to the container.
  **
  ** @param object : object to append to the container.
  */
  void append(ObjectPtr object)
    {objects.push_back(object);}

  /*!
  ** Data getter.
  **
  ** @return data vector.
  */
  const std::vector<ObjectPtr>& getData() const
    {return objects;}

  /*!
  ** Data getter.
  **
  ** @return data vector.
  */
  std::vector<ObjectPtr>& getData()
    {return objects;}


protected:
  std::vector<ObjectPtr> objects; /*!< Object list.*/
  std::string contentClassName; /*!< Content class name. */
};


/*!
** @class DecoratorObjectContainer
** @brief Object container base classe (design pattern Decorator).
*/
class DecoratorObjectContainer : public ObjectContainer
{
public:
  /*!
  ** Constructor.
  **
  ** @param target : target.
  **
  ** @return a DecoratorObjectContainer.
  */
  DecoratorObjectContainer(ObjectContainerPtr target)
    : target(target) {}

  /*!
  ** Content class name getter.
  **
  ** @return content class name.
  */
  virtual std::string getContentClassName() const
    {return target->getContentClassName();}

  /*!
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t size() const
    {return target->size();}

  /*!
  ** Gets @a index item.
  **
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const
    {return target->get(index);}

protected:
  ObjectContainerPtr target;    /*!< Object container pointer (target). */
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONTAINER_H_

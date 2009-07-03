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
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_OBJECT_CONTAINER_H_
# define LBCPP_OBJECT_CONTAINER_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

// -> reprend les fonctionalites de nieme::InstanceSet, voir la doc de InstanceSet
/*!
** @class ObjectContainer
** @brief
*/
class ObjectContainer : public Object
{
public:
  /*!
  ** Content class name getter.
  **
  ** @return content class name.
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
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual ObjectPtr get(size_t index) const = 0;

  /*!
  ** Check if the container is empty or not.
  **
  ** @return size == 0.
  */
  bool empty() const
    {return size() == 0;}

  /*!
  ** Get (by index) and cast item from the container.
  **
  ** @param index : item index.
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
  ** Convert to a VectorObjectContainer (based on a
  ** 'std::vector<ObjectPtr>' container).
  **
  ** @return a VectorObjectContainer pointer.
  */
  virtual VectorObjectContainerPtr toVector() const;

  /*!
  ** Convert to an Object Stream.
  **
  ** @return an object stream pointer.
  */
  virtual ObjectStreamPtr toStream() const;

  /*!
  ** Apply a @a function to all item contained into the object container.
  **
  ** @param function : function to apply to the current object container.
  ** @param lazyCompute : specify if the machine is powerful enought
  ** or not.
  **
  ** @return an object container pointer.
  */
  ObjectContainerPtr apply(ObjectFunctionPtr function, bool lazyCompute = true);

public:
  /*!
  ** Create a randomized version of a dataset.
  **
  ** @return an object container pointer.
  */
  ObjectContainerPtr randomize();

  /*!
  ** Create a set where each instance is duplicated @a count times.
  **
  ** @param count : number of times an instance is duplicated.
  **
  ** @return an object container pointer.
  */
  ObjectContainerPtr duplicate(size_t count);

  /*!
  ** Select a fold in the object container.
  **
  ** @param fold : fold index.
  ** @param numFolds : number of folds.
  **
  ** @return an object container pointer of the fold
  ** [@a fold * meanFoldSize, (@a fold + 1) * meanFoldSize], where
  ** meanFoldSize == size() / @a numFolds.
  */
  ObjectContainerPtr fold(size_t fold, size_t numFolds);

  /*!
  ** Exclude a fold.
  **
  ** @param fold : fold index to exclude.
  ** @param numFolds : number of folds.
  **
  ** @return an object container pointer of all folds without the fold
  ** [@a fold * meanFoldSize, (@a fold + 1) * meanFoldSize], where
  ** meanFoldSize == size() / @a numFolds.
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
  ** Excludes a range.
  **
  ** @param begin : first item index.
  ** @param end : last item index.
  **
  ** @return an object container pointer of the all items precudling
  ** items between @a begin and @a end.
  */
  ObjectContainerPtr invRange(size_t begin, size_t end);
};

/*!
** Append two object containers.
**
** @param left : first object container.
** @param right : second object container.
**
** @return an object container pointer (left + right).
*/
extern ObjectContainerPtr append(ObjectContainerPtr left, ObjectContainerPtr right);


/*!
** @class VectorObjectContainer
** @brief Object container (vector).
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
  ** Convert to vector.
  **
  ** @return a VectorObjectContainer pointer.
  */
  virtual VectorObjectContainerPtr toVector() const
    {return VectorObjectContainerPtr(const_cast<VectorObjectContainer* >(this));}

  /*!
  ** Reserve @a size space.
  **
  ** @param size : space to reserve.
  */
  void reserve(size_t size)
    {objects.reserve(size);}

  /*!
  ** Append @a object to the container.
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
** @brief
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
  ** Get @a index item.
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

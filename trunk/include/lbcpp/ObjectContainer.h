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
  **
  **
  **
  ** @return
  */
  virtual std::string getContentClassName() const
    {return "Object";}

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t size() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ObjectPtr get(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  bool empty() const
    {return size() == 0;}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> getAndCast(size_t index) const
  {
    ObjectPtr res = get(index);
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  // Convert to a VectorObjectContainer (based on a 'std::vector<ObjectPtr>' container).
  /*!
  **
  **
  **
  ** @return
  */
  virtual VectorObjectContainerPtr toVector() const;

  // Convert to an Object Stream.
  /*!
  **
  **
  **
  ** @return
  */
  virtual ObjectStreamPtr toStream() const;

  /*!
  **
  **
  ** @param function
  ** @param lazyCompute
  **
  ** @return
  */
  ObjectContainerPtr apply(ObjectFunctionPtr function, bool lazyCompute = true);

public:
  // Creates a randomized version of a dataset.
  /*!
  **
  **
  **
  ** @return
  */
  ObjectContainerPtr randomize();

  // Creates a set where each instance is duplicated multiple times.
  /*!
  **
  **
  ** @param count
  **
  ** @return
  */
  ObjectContainerPtr duplicate(size_t count);

  // Selects a fold.
  /*!
  **
  **
  ** @param fold
  ** @param numFolds
  **
  ** @return
  */
  ObjectContainerPtr fold(size_t fold, size_t numFolds);

  // Excludes a fold.
  /*!
  **
  **
  ** @param fold
  ** @param numFolds
  **
  ** @return
  */
  ObjectContainerPtr invFold(size_t fold, size_t numFolds);

  // Selects a range.
  /*!
  **
  **
  ** @param begin
  ** @param end
  **
  ** @return
  */
  ObjectContainerPtr range(size_t begin, size_t end);

  // Excludes a range.
  /*!
  **
  **
  ** @param begin
  ** @param end
  **
  ** @return
  */
  ObjectContainerPtr invRange(size_t begin, size_t end);
};

// Append two object containers.
/*!
**
**
** @param left
** @param right
**
** @return
*/
extern ObjectContainerPtr append(ObjectContainerPtr left, ObjectContainerPtr right);


/*!
** @class VectorObjectContainer
** @brief
*/
class VectorObjectContainer : public ObjectContainer
{
public:
  /*!
  **
  **
  ** @param objects
  ** @param contentClassName
  **
  ** @return
  */
  VectorObjectContainer(const std::vector<ObjectPtr>& objects, const std::string& contentClassName = "Object")
    : objects(objects), contentClassName(contentClassName) {}

  /*!
  **
  **
  ** @param contentClassName
  **
  ** @return
  */
  VectorObjectContainer(const std::string& contentClassName = "Object")
    : contentClassName(contentClassName) {}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string getContentClassName() const
    {return contentClassName;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t size() const
    {return objects.size();}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ObjectPtr get(size_t index) const
    {assert(index < objects.size()); return objects[index];}

  /*!
  **
  **
  **
  ** @return
  */
  virtual VectorObjectContainerPtr toVector() const
    {return VectorObjectContainerPtr(const_cast<VectorObjectContainer* >(this));}

  /*!
  **
  **
  ** @param size
  */
  void reserve(size_t size)
    {objects.reserve(size);}

  /*!
  **
  **
  ** @param object
  */
  void append(ObjectPtr object)
    {objects.push_back(object);}

  /*!
  **
  **
  **
  ** @return
  */
  const std::vector<ObjectPtr>& getData() const
    {return objects;}

  /*!
  **
  **
  **
  ** @return
  */
  std::vector<ObjectPtr>& getData()
    {return objects;}


protected:
  std::vector<ObjectPtr> objects; /*!< */
  std::string contentClassName; /*!< */
};


/*!
** @class DecoratorObjectContainer
** @brief
*/
class DecoratorObjectContainer : public ObjectContainer
{
public:
  /*!
  **
  **
  ** @param target
  **
  ** @return
  */
  DecoratorObjectContainer(ObjectContainerPtr target)
    : target(target) {}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string getContentClassName() const
    {return target->getContentClassName();}

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t size() const
    {return target->size();}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ObjectPtr get(size_t index) const
    {return target->get(index);}

protected:
  ObjectContainerPtr target;    /*!< */
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONTAINER_H_

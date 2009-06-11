/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainer.h              | Object RandomAccess Containers  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 15:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_OBJECT_CONTAINER_H_
# define LBCPP_OBJECT_CONTAINER_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

// -> reprend les fonctionalites de nieme::InstanceSet, voir la doc de InstanceSet
class ObjectContainer : public Object
{
public:
  virtual std::string getContentClassName() const
    {return "Object";}
  
  virtual size_t size() const = 0;
  virtual ObjectPtr get(size_t index) const = 0;
  
  bool empty() const
    {return size() == 0;}
  
  template<class T>
  inline ReferenceCountedObjectPtr<T> getCast(size_t index) const
  {
    ObjectPtr res = get(index);
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  // Convert to a VectorObjectContainer (based on a 'std::vector<ObjectPtr>' container).
  virtual VectorObjectContainerPtr toVector() const;
  
  // Convert to an Object Stream.
  virtual ObjectStreamPtr toStream() const;
  
  ObjectContainerPtr apply(ObjectFunctionPtr function, bool lazyCompute = true);
  
public:
  // Creates a randomized version of a dataset.
  ObjectContainerPtr randomize();
 	
  // Creates a set where each instance is duplicated multiple times.
  ObjectContainerPtr duplicate(size_t count);
 	
  // Selects a fold.
  ObjectContainerPtr fold(size_t fold, size_t numFolds);

  // Excludes a fold.
  ObjectContainerPtr invFold(size_t fold, size_t numFolds);

  // Selects a range.
  ObjectContainerPtr range(size_t begin, size_t end);
  
  // Excludes a range.
  ObjectContainerPtr invRange(size_t begin, size_t end);
};

// Append two object containers.
extern ObjectContainerPtr append(ObjectContainerPtr left, ObjectContainerPtr right);

class VectorObjectContainer : public ObjectContainer
{
public:
  VectorObjectContainer(const std::vector<ObjectPtr>& objects, const std::string& contentClassName = "Object")
    : objects(objects), contentClassName(contentClassName) {}
    
  VectorObjectContainer(const std::string& contentClassName = "Object")
    : contentClassName(contentClassName) {}
    
  virtual std::string getContentClassName() const
    {return contentClassName;}

  virtual size_t size() const
    {return objects.size();}
    
  virtual ObjectPtr get(size_t index) const
    {assert(index < objects.size()); return objects[index];}

  virtual VectorObjectContainerPtr toVector() const
    {return VectorObjectContainerPtr(const_cast<VectorObjectContainer* >(this));}

  void reserve(size_t size)
    {objects.reserve(size);}
    
  void append(ObjectPtr object)
    {objects.push_back(object);}

  const std::vector<ObjectPtr>& getData() const
    {return objects;}

  std::vector<ObjectPtr>& getData()
    {return objects;}
    
  
protected:
  std::vector<ObjectPtr> objects;
  std::string contentClassName;
};

class DecoratorObjectContainer : public ObjectContainer
{
public:
  DecoratorObjectContainer(ObjectContainerPtr target)
    : target(target) {}
    
  virtual std::string getContentClassName() const
    {return target->getContentClassName();}
  
  virtual size_t size() const
    {return target->size();}
    
  virtual ObjectPtr get(size_t index) const
    {return target->get(index);}

protected:
  ObjectContainerPtr target;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONTAINER_H_

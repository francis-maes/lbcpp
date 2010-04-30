/*-----------------------------------------.---------------------------------.
| Filename: ObjectPair.h                   | Object Pair                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COMMON_OBJECT_PAIR_H_
# define LBCPP_COMMON_OBJECT_PAIR_H_

# include "ObjectStream.h"

namespace lbcpp
{

class ObjectPair : public ObjectContainer
{
public:
  ObjectPair(const String& name, ObjectPtr first, ObjectPtr second)
    : ObjectContainer(name), first(first), second(second) {}
  ObjectPair(ObjectPtr first, ObjectPtr second)
    : first(first), second(second) {}
  ObjectPair() {}

  // todo: toString/read/write/clone

  ObjectPtr getFirst() const
    {return first;}

  void setFirst(ObjectPtr object)
    {first = object;}

  ObjectPtr getSecond() const
    {return second;}

  void setSecond(ObjectPtr object)
    {second = object;}

  /*
  ** ObjectContainer
  */
  virtual size_t size() const
    {return 2;}

  virtual ObjectPtr get(size_t index) const
    {return index ? second : first;}

  virtual void set(size_t index, ObjectPtr object)
    {index ? (second = object) : (first = object);}

protected:
  ObjectPtr first;
  ObjectPtr second;
};

typedef ReferenceCountedObjectPtr<ObjectPair> ObjectPairPtr;

class ObjectToObjectPairFunction : public ObjectFunction
{
public:
  ObjectToObjectPairFunction(bool copyObjectIntoFirst = true, bool copyObjectIntoSecond = true)
    : copyObjectIntoFirst(copyObjectIntoFirst), copyObjectIntoSecond(copyObjectIntoSecond) {}

  virtual String getInputClassName() const
    {return T("Object");}
  
  virtual String getOutputClassName(const String& ) const
    {return T("ObjectPair");}

  virtual ObjectPtr function(ObjectPtr object) const
    {return new ObjectPair(object->getName() + T(" pair"), copyObjectIntoFirst ? object : ObjectPtr(), copyObjectIntoSecond ? object : ObjectPtr());}

private:
  bool copyObjectIntoFirst;
  bool copyObjectIntoSecond;
};

}; /* namespace lbcpp */

#endif //!LBCPP_COMMON_OBJECT_PAIR_H_

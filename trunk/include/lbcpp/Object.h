/*-----------------------------------------.---------------------------------.
| Filename: Object.h                       | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_OBJECT_H_
# define LBCPP_OBJECT_H_

# include "ContainerTraits.h"
# include "ReferenceCountedObject.h"
# include "Utilities.h"

namespace lbcpp
{

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;
class ObjectGraph;
typedef ReferenceCountedObjectPtr<ObjectGraph> ObjectGraphPtr;
class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

class Object : public ReferenceCountedObject
{
public:
  virtual ~Object() {}
  
  typedef Object* (*Constructor)();
    
  static void declare(const std::string& className, Constructor constructor);
  static Object* create(const std::string& className);
  
  static ObjectPtr loadFromStream(std::istream& istr);
  static ObjectPtr loadFromFile(const std::string& fileName);
  
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromStreamAndCast(std::istream& istr)
    {return checkCast<T>("Object::loadFromStreamAndCast", loadFromStream(istr));}

  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromFileAndCast(const std::string& fileName)
    {return checkCast<T>("Object::loadFromFileAndCast", loadFromFile(fileName));}

  std::string getClassName() const;
  virtual std::string getName() const
    {return getClassName() + "::getName() unimplemented";}
    
  virtual std::string toString() const
    {return getClassName() + "::toString() unimplemented";}

  virtual ObjectGraphPtr toGraph() const
    {return ObjectGraphPtr();}
    
  virtual TablePtr toTable() const
    {return TablePtr();}
  
  virtual ObjectPtr clone() const
    {assert(false); return ObjectPtr();}
    
  template<class T>
  ReferenceCountedObjectPtr<T> cloneAndCast() const
    {return checkCast<T>("Object::cloneAndCast", clone());}
  
  bool saveToFile(const std::string& fileName) const;
  void saveToStream(std::ostream& ostr) const;

  static void error(const std::string& where, const std::string& what)
    {ErrorHandler::error(where, what);}
    
  static void warning(const std::string& where, const std::string& what)
    {ErrorHandler::warning(where, what);}
  
protected:
  template<class T>
  friend struct ObjectTraits;

  template<class T>
  static ReferenceCountedObjectPtr<T> checkCast(const std::string& where, ObjectPtr object)
  {
    ReferenceCountedObjectPtr<T> res;
    if (object)
    {
      res = object.dynamicCast<T>();
      if (!res)
        error(where, "Could not cast object into '" + lbcpp::toString(typeid(*res)) + "'");
    }
    return res;
  }

  virtual bool load(std::istream& istr) {return true;}
  virtual void save(std::ostream& ostr) const {}
};

template<class Type>
struct ClassDeclarator
{
  ClassDeclarator()           {Object::declare(toString(typeid(Type)), construct);}
  static Object* construct()  {return new Type();}
};

#define LBCPP_DECLARE_CLASS(Name) lbcpp::ClassDeclarator<Name> __##Name##decl__

template<class T>
struct ObjectPtrTraits
{
public:
  static inline std::string toString(const ReferenceCountedObjectPtr<T> value)
    {return value ? value->toString() : "null";}
  static inline void write(std::ostream& ostr, const ReferenceCountedObjectPtr<T> value)
  {
    if (value)
      value->saveToStream(ostr);
    else
      lbcpp::write(ostr, std::string("__null__"));
  }
  static inline bool read(std::istream& istr, ReferenceCountedObjectPtr<T>& result)
  {
    result = Object::loadFromStreamAndCast<T>(istr);
    return true;
  }
};

template<class T>
struct ObjectTraits
{
public:
  static inline std::string toString(const T& value)
    {return value.toString();}
  static inline void write(std::ostream& ostr, const T& value)
    {value.save(ostr);}
  static inline bool read(std::istream& istr, T& result)
    {return result.load(istr);}
};

template<>
struct Traits<ObjectPtr> : public ObjectPtrTraits<Object> {};

template<class T>
struct Traits< ReferenceCountedObjectPtr<T> > : public ObjectPtrTraits<T> {};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_H_

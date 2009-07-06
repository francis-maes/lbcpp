/*-----------------------------------------.---------------------------------.
| Filename: Object.h                       | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Object.h
**@author Francis MAES
**@date   Mon Jun 15 23:43:59 2009
**
**@brief  A base class for serializable objects.
**
**
*/

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

/*!
** @class Object
** @brief Object factory.
*/
class Object : public ReferenceCountedObject
{
public:
  /*!
  ** Destructor.
  */
  virtual ~Object() {}

  typedef Object* (*Constructor)();

  /*!
  ** Object declaration. A @a className object with a defaut
  ** @a contructor may be declare into the Object factory. After what,
  ** you can use Object::create() (@see Object::create) function to
  ** dynamically create that kind of objects.
  **
  ** @param className : class name.
  ** @param constructor : class constructor.
  */
  static void declare(const std::string& className, Constructor constructor);

  /*!
  ** Creates an object of class @a className. You should declare class
  ** @a className with Object::declare() (@see Object::declare)
  ** function before using the Object factory.
  **
  ** @param className : class name.
  **
  ** @return an instance of @a className object.
  */
  static Object* create(const std::string& className);

  /*!
  ** Loads an object from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded object or ObjectPtr if an error
  ** occurs.
  */
  static ObjectPtr loadFromStream(std::istream& istr);

  /*!
  ** Loads an object from a file.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded object or ObjectPtr if any error
  ** occurs.
  */
  static ObjectPtr loadFromFile(const std::string& fileName);

  /*!
  ** Loads an object from a stream and cast it.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded object or NULL if invalid cast.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromStreamAndCast(std::istream& istr)
    {return checkCast<T>("Object::loadFromStreamAndCast", loadFromStream(istr));}

  /*!
  ** Loads an object from a file and cast it.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded object or NULL if invalid cast.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromFileAndCast(const std::string& fileName)
    {return checkCast<T>("Object::loadFromFileAndCast", loadFromFile(fileName));}

  /*!
  ** Class name getter.
  **
  ** @return class name.
  */
  std::string getClassName() const;

  /*!
  ** Name getter.
  **
  ** @return object name.
  */
  virtual std::string getName() const
    {return getClassName() + "::getName() unimplemented";}

  /*!
  ** Converts the current object to a string.
  **
  ** @return the current object (string form).
  */
  virtual std::string toString() const
    {return getClassName() + "::toString() unimplemented";}

  /*!
  ** Converts the current object to graph (optional).
  **
  ** @return the current object (graph form) or ObjectGraphPtr if any
  ** error occurs.
  */
  virtual ObjectGraphPtr toGraph() const
    {return ObjectGraphPtr();}

  /*!
  ** Converts the current object to table (optional).
  **
  ** @return the current object (table form) or TablePtr if any error
  ** occurs. .
  */
  virtual TablePtr toTable() const
    {return TablePtr();}

  /*!
  ** Clones the current object.
  **
  ** @return a copy of the current object.
  */
  virtual ObjectPtr clone() const
    {assert(false); return ObjectPtr();}

  /*!
  ** Clones and cast the current object.
  **
  ** @return a casted copy of the current object.
  */
  template<class T>
  ReferenceCountedObjectPtr<T> cloneAndCast() const
    {return checkCast<T>("Object::cloneAndCast", clone());}

  /*!
  ** Saves the current object to the file @a filename.
  **
  ** @param fileName : output file name.
  **
  ** @return False if any error occurs.
  */
  bool saveToFile(const std::string& fileName) const;

  /*!
  ** Saves the current object to a stream.
  **
  ** @param ostr : output stream.
  */
  void saveToStream(std::ostream& ostr) const;

  /*!
  ** Error manager.
  **
  ** @see ErrorHandler (in Utilities.h)
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  static void error(const std::string& where, const std::string& what)
    {ErrorHandler::error(where, what);}

  /*!
  ** Warning manager.
  **
  ** @see ErrorHandler (in Utilities.h)
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
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

/*!
** Loads an object from the file @a filename.
**
** @see Object::saveToStream
** @see Object::saveToFile
** @param filename : file name.
**
** @return an object pointer.
*/
inline ObjectPtr loadObject(const std::string& filename)
  {return Object::loadFromFile(filename);}

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

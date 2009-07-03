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
**@brief  #FIXME: all
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
** @brief
*/
class Object : public ReferenceCountedObject
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ~Object() {}

  typedef Object* (*Constructor)();

  /*!
  ** Object declaration.
  **
  ** @param className : class name.
  ** @param constructor : constructor.
  */
  static void declare(const std::string& className, Constructor constructor);

  /*!
  ** Create an object of class @a className.
  **
  ** @param className : class name.
  **
  ** @return a pointer on a @a className object.
  */
  static Object* create(const std::string& className);

  /*!
  ** Load an object from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded (if any) object.
  */
  static ObjectPtr loadFromStream(std::istream& istr);

  /*!
  ** Load an object from a file.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded (if any) object.
  */
  static ObjectPtr loadFromFile(const std::string& fileName);

  /*!
  ** Load an object from a stream an cast it.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded (if any) object.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromStreamAndCast(std::istream& istr)
    {return checkCast<T>("Object::loadFromStreamAndCast", loadFromStream(istr));}

  /*!
  ** Load an object from a file and cast it.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded( if any) object.
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
  ** Convert the current object to string.
  **
  ** @return the current object (string form).
  */
  virtual std::string toString() const
    {return getClassName() + "::toString() unimplemented";}

  /*!
  ** Convert the current object to graph.
  **
  ** @return the current object (graph form).
  */
  virtual ObjectGraphPtr toGraph() const
    {return ObjectGraphPtr();}

  /*!
  ** Convert the current object to table.
  **
  ** @return the current object (table form).
  */
  virtual TablePtr toTable() const
    {return TablePtr();}

  /*!
  ** Clone the current object.
  **
  ** @return a copy of the current object.
  */
  virtual ObjectPtr clone() const
    {assert(false); return ObjectPtr();}

  /*!
  ** Clone and cast the current object.
  **
  ** @return a casted copy of the current object.
  */
  template<class T>
  ReferenceCountedObjectPtr<T> cloneAndCast() const
    {return checkCast<T>("Object::cloneAndCast", clone());}

  /*!
  ** Save the current object to the file @a filename.
  **
  ** @param fileName : output file name.
  **
  ** @return False if any error occurs.
  */
  bool saveToFile(const std::string& fileName) const;

  /*!
  ** Save the current object to a stream.
  **
  ** @param ostr : output stream.
  */
  void saveToStream(std::ostream& ostr) const;

  /*!
  ** Error manager.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  static void error(const std::string& where, const std::string& what)
    {ErrorHandler::error(where, what);}

  /*!
  ** Warning manager.
  **
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
** Load an object from the file @a filename.
**
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

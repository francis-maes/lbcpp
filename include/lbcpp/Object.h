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

/*!
** @class Object
** @brief Object is the base class of nearly all classes of LBC++ library.
**   Object provides three main features:
**    - Support for reference counting: Object override from ReferenceCountedObject,
**      so that Objects are reference counted.
**    - Support for serialization: Objects can be saved and loaded
**      into C++ streams. Objects can be created dynamically given their class name.
**    - Support for conversion: Objects provide conversion functions to strings,
**      to tables and to graphs.
*/
class Object : public ReferenceCountedObject
{
public:
  /**
  ** Destructor.
  */
  virtual ~Object() {}

  /** Pointer to a function that create objects
  */
  typedef Object* (*Constructor)();

  /** Declares a class to enable dynamic creation of this class.
  **
  ** Objects rely internally on an Object factory that keeps a trace of classes
  ** that can be created dynamically. This function registers a new class
  ** to this ObjectFactory.
  **
  ** @param className : class name.
  ** @param constructor : class constructor.
  ** @see Object::create
  */
  static void declare(const String& className, Constructor constructor);

  /** Creates dynamically an object of class @a className.
  **
  ** The class @a className must be declared with Object::declare()
  ** before being able to instantiate it dynamically.
  **
  ** @param className : class name.
  **
  ** @return an instance of @a className object.
  ** @see Object::declare
  */
  static Object* create(const String& className);

  /**
  ** Loads an object from a C++ stream.
  **
  ** @param istr : C++ input stream.
  **
  ** @return a pointer on the loaded object or ObjectPtr() if an error
  ** occurs.
  ** @see saveToStream
  */
  static ObjectPtr loadFromStream(InputStream& istr);

  /**
  ** Loads an object from a file.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded object or ObjectPtr() if any error
  ** occurs.
  ** @see saveToFile
  */
  static ObjectPtr loadFromFile(const File& file);

  /**
  ** Loads an object from a stream and cast it.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded object or a null pointer if the cast fails.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromStreamAndCast(InputStream& istr)
    {return checkCast<T>(T("Object::loadFromStreamAndCast"), loadFromStream(istr));}

  /**
  ** Loads an object from a file and cast it.
  **
  ** @param fileName : file name.
  **
  ** @return a pointer on the loaded object or a null pointer if the cast fails.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromFileAndCast(const File& file)
    {return checkCast<T>(T("Object::loadFromFileAndCast"), loadFromFile(file));}

  /**
  ** Class name getter.
  **
  ** @return class name.
  */
  String getClassName() const;

  /**
  ** Name getter.
  **
  ** Note that not all Objects implement this function. Furthermore,
  ** there is not particular semantic assigned to the name of an Object:
  ** The getName() function may be used in different ways depending
  ** on the kinds of Objects.
  **
  ** @return object name.
  */
  virtual String getName() const
    {return getClassName() + T("::getName() unimplemented");}

  /**
  ** Converts the current object to a string.
  **
  ** @return the current object (string form).
  */
  virtual String toString() const
    {return getClassName() + T("::toString() unimplemented");}

  /**
  ** Converts the current object to a graph.
  **
  ** Try to convert the object into a graph (for serialization
  ** or visualization purpose).
  **
  ** @return the current object (graph form) or ObjectGraphPtr() if the
  ** conversion to graph is undefined for this object.
  */
  virtual ObjectGraphPtr toGraph() const
    {return ObjectGraphPtr();}

  /**
  ** Converts the current object to a table.
  **
  ** Try to convert the object into a table (for serialization
  ** or visualization purpose).
  **
  ** @return the current object (table form) or TablePtr() if the
  ** conversion to table is undefined for this object.
  */
  virtual TablePtr toTable() const
    {return TablePtr();}

  /**
  ** Clones the current object.
  **
  ** Note that the clone() function is not defined on all objects.
  **
  ** @return a copy of the current object or ObjectPtr() if
  ** the clone() operation is undefined for this object.
  */
  virtual ObjectPtr clone() const
    {jassert(false); return ObjectPtr();}

  /**
  ** Clones and cast the current object.
  **
  ** @return a casted copy of the current object.
  */
  template<class T>
  ReferenceCountedObjectPtr<T> cloneAndCast() const
    {return checkCast<T>(T("Object::cloneAndCast"), clone());}

  /**
  ** Saves the current object to the file @a filename.
  **
  ** @param fileName : output file name.
  **
  ** @return False if any error occurs.
  ** @see loadFromFile
  */
  bool saveToFile(const File& file) const;

  /**
  ** Saves the current object to a C++ stream.
  **
  ** @param ostr : output stream.
  ** @see loadFromStream
  */
  void saveToStream(OutputStream& ostr) const;

  /**
  ** Error manager.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  ** @see ErrorHandler (in Utilities.h)
  */
  static void error(const String& where, const String& what)
    {ErrorHandler::error(where, what);}

  /**
  ** Warning manager.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  ** @see ErrorHandler (in Utilities.h)
  */
  static void warning(const String& where, const String& what)
    {ErrorHandler::warning(where, what);}

protected:
  template<class T>
  friend struct ObjectTraits;

  /** Checks if a cast is valid and throw an error if not.
  **
  ** @param where : a description of the caller function
  ** that will be used in case of an error.
  ** @param object : to object to cast.
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the ErrorManager.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> checkCast(const String& where, ObjectPtr object)
  {
    ReferenceCountedObjectPtr<T> res;
    if (object)
    {
      res = object.dynamicCast<T>();
      if (!res)
        error(where, T("Could not cast object into '") + lbcpp::toString(typeid(*res)) + T("'"));
    }
    return res;
  }

  /**
  ** Override this function to load the object from a C++ stream.
  **
  ** @param istr : a C++ input stream.
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the ErrorManager.
  */
  virtual bool load(InputStream& istr)
    {return true;}

  /**
  ** Override this function to save the object to a C++ stream.
  **
  ** @param ostr : a C++ output stream.
  */
  virtual void save(OutputStream& ostr) const
    {}
};

/**
** Loads an object from the file @a filename.
**
** @see Object::saveToStream
** @see Object::saveToFile
** @param filename : file name.
**
** @return an object pointer.
*/
inline ObjectPtr loadObject(const File& file)
  {return Object::loadFromFile(file);}

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
  static inline String toString(const ReferenceCountedObjectPtr<T> value)
    {return value ? value->toString() : T("null");}
    
  static inline void write(OutputStream& ostr, const ReferenceCountedObjectPtr<T> value)
  {
    if (value)
      value->saveToStream(ostr);
    else
      lbcpp::write(ostr, T("__null__"));
  }
  
  static inline bool read(InputStream& istr, ReferenceCountedObjectPtr<T>& result)
  {
    result = Object::loadFromStreamAndCast<T>(istr);
    return true;
  }
};

template<class T>
struct ObjectTraits
{
public:
  static inline String toString(const T& value)
    {return value.toString();}
    
  static inline void write(OutputStream& ostr, const T& value)
    {value.save(ostr);}
    
  static inline bool read(InputStream& istr, T& result)
    {return result.load(istr);}
};

template<>
struct Traits<ObjectPtr> : public ObjectPtrTraits<Object> {};

template<class T>
struct Traits< ReferenceCountedObjectPtr<T> > : public ObjectPtrTraits<T> {};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_H_

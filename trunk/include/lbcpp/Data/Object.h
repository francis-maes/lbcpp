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

# include "../ContainerTraits.h"
# include "ReferenceCountedObject.h"
# include "../Object/ErrorHandler.h"

namespace lbcpp
{

class Variable;
class VariableReference;

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;
class Type;
typedef ReferenceCountedObjectPtr<Type> TypePtr;
class Class;
typedef ReferenceCountedObjectPtr<Class> ClassPtr;
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
  Object(ClassPtr thisClass)
    : thisClass(thisClass) {}
  Object() {}
  
  /**
  ** Destructor.
  */
  virtual ~Object() {}

  /*
  ** Introspection
  */
  virtual ClassPtr getClass() const;
  String getClassName() const;

  virtual size_t getNumVariables() const;
  virtual TypePtr getVariableType(size_t index) const;
  virtual String getVariableName(size_t index) const;
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);
  
  /*
  ** Dynamic creation
  **  deprecated functions, call directly the methods of Type
  */
  //static ObjectPtr create(const String& className);

/*  template<class T>
  static ReferenceCountedObjectPtr<T> createAndCast(const String& className)
    {return checkCast<T>(T("Object::createAndCast"), create(className));}*/

  /**
  ** Loads an object from a C++ stream.
  **
  ** @param istr : C++ input stream.
  **
  ** @return a pointer on the loaded object or ObjectPtr() if an error
  ** occurs.
  ** @see saveToStream
  */
  static ObjectPtr createFromStream(InputStream& istr, bool doLoading = true);

  /**
  ** Loads an object from a stream and cast it.
  **
  ** @param istr : input stream.
  **
  ** @return a pointer on the loaded object or a null pointer if the cast fails.
  */
  template<class T>
  static ReferenceCountedObjectPtr<T> createFromStreamAndCast(InputStream& istr)
    {return checkCast<T>(T("Object::createFromStreamAndCast"), createFromStream(istr));}


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
  virtual String toString() const;
  virtual String getShortSummary() const;

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
  virtual ObjectPtr clone() const;
  virtual void clone(ObjectPtr target) const;

  virtual int compare(ObjectPtr otherObject) const
    {return (int)(this - otherObject.get());}

  virtual ObjectPtr multiplyByScalar(double scalar)
    {jassert(false); return ObjectPtr(this);}
  
  virtual ObjectPtr addWeighted(const Variable& value, double weight)
    {jassert(false); return ObjectPtr(this);}

  /**
  ** Override this function to save the object to an XML tree
  **
  ** @param xml : the target XML tree
  */
  virtual void saveToXml(XmlElement* xml) const;

  /**
  ** Override this function to load the object from an XML tree
  **
  ** @param xml : an XML tree
  ** @param callback : a callback that can receive errors and warnings
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the callback.
  */
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);

  /**
  ** Override this function to load the object from a String
  **
  ** @param str : a String
  ** @param callback : a callback that can receive errors and warnings
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the callback.
  */
  virtual bool loadFromString(const String& str, ErrorHandler& callback);

  // tmp
  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
    {}

  /**
  ** Clones and cast the current object.
  **
  ** @return a casted copy of the current object.
  */
  template<class T>
  ReferenceCountedObjectPtr<T> cloneAndCast() const
    {return checkCast<T>(T("Object::cloneAndCast"), clone());}

  /**
  ** Saves the current object to a C++ stream.
  **
  ** @param ostr : output stream.
  ** @see createFromStream
  */
  virtual void saveToStream(OutputStream& ostr) const;

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

  // user interface
  virtual juce::Component* createComponent() const
    {return NULL;}

protected:
  friend class Class;
  
  ClassPtr thisClass;
  
  template<class T>
  friend struct ObjectTraits;

  virtual bool load(InputStream& istr) {jassert(false); return false;}
  virtual void save(OutputStream& ostr) const {jassert(false);}

  // utilities
  String variablesToString(const String& separator, bool includeTypes = true) const;
  XmlElement* variableToXml(size_t index) const;
  void saveVariablesToXmlAttributes(XmlElement* xml) const;
  bool loadVariablesFromXmlAttributes(XmlElement* xml, ErrorHandler& callback);
};

class NameableObject : public Object
{
public:
  NameableObject(const String& name = T("Unnamed"))
    : name(name) {}

  virtual String getName() const
    {return name;}

  virtual String toString() const
    {return getClassName() + T(" ") + name;}

  virtual void setName(const String& name)
    {this->name = name;}

protected:
  friend class NameableObjectClass;

  String name;
};

extern ClassPtr nameableObjectClass();

typedef ReferenceCountedObjectPtr<NameableObject> NameableObjectPtr;

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
    {
      static const String nullString(T("__null__"));
      lbcpp::write(ostr, nullString);
    }
  }
  
  static inline bool read(InputStream& istr, ReferenceCountedObjectPtr<T>& result)
  {
    result = Object::createFromStreamAndCast<T>(istr);
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

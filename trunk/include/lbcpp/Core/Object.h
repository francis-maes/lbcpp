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

#ifndef LBCPP_CORE_OBJECT_H_
# define LBCPP_CORE_OBJECT_H_

# include "predeclarations.h"

namespace lbcpp
{

/*!
** @class Object
** @brief Object is the base class of nearly all classes of LBC++ library.
**   Object provides three main features:
**    - Support for reference counting: Object override from ReferenceCountedObject,
**      so that Objects are reference counted.
**    - Support for serialization: Objects can be saved and loaded
**      to/from XML files. Objects can be created dynamically given their class name.
**    - Support for introspection: It is possible to generically access to Object variables.
*/
class Object : public ReferenceCountedObject
{
public:
  Object(ClassPtr thisClass = ClassPtr());
  virtual ~Object();
  
  static ObjectPtr create(ClassPtr objectClass);

  static void displayObjectAllocationInfo(std::ostream& ostr);

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
  virtual String toShortString() const;

  /**
  ** Clones the current object.
  **
  ** Note that the clone() function is not defined on all objects.
  **
  ** @return a copy of the current object or ObjectPtr() if
  ** the clone() operation is undefined for this object.
  */
  virtual ObjectPtr clone(ExecutionContext& context) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  ObjectPtr deepClone(ExecutionContext& context) const;
  ObjectPtr cloneToNewType(ExecutionContext& context, ClassPtr newType) const;

  /**
  ** Clones and cast the current object.
  **
  ** @return a casted copy of the current object.
  */
  template<class Type>
  ReferenceCountedObjectPtr<Type> cloneAndCast(ExecutionContext& context) const
  {
    ObjectPtr res = clone(context);
    return res.staticCast<Type>();
  }

  template<class Type>
  ReferenceCountedObjectPtr<Type> cloneAndCast() const
    {ObjectPtr res = clone(lbcpp::defaultExecutionContext()); return res.staticCast<Type>();}

  /*
  ** Compare
  */
  virtual int compare(ObjectPtr otherObject) const
    {return (int)(this - otherObject.get());}

  /**
  ** Override this function to save the object to an XML tree
  **
  ** @param xml : the target XML tree
  */
  virtual void saveToXml(XmlExporter& exporter) const;

  /**
  ** Override this function to load the object from an XML tree
  **
  ** @param xml : an XML tree
  ** @param callback : a callback that can receive errors and warnings
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the callback.
  */
  virtual bool loadFromXml(XmlImporter& importer);

  /**
  ** Override this function to load the object from a String
  **
  ** @param str : a String
  ** @param callback : a callback that can receive errors and warnings
  ** @return false is the loading fails, true otherwise. If loading fails,
  ** load() is responsible for declaring an error to the callback.
  */
  virtual bool loadFromString(ExecutionContext& context, const String& str);

  /*
  ** High level serialisation
  */
  void saveToFile(ExecutionContext& context, const File& file) const;
  static ObjectPtr createFromFile(ExecutionContext& context, const File& file);

  /*
  ** Introspection: Class
  */
  virtual ClassPtr getClass() const;
  String getClassName() const;

  void setThisClass(ClassPtr thisClass);

  /*
  ** Introspection: Variables
  */
  size_t getNumVariables() const;
  TypePtr getVariableType(size_t index) const;
  String getVariableName(size_t index) const;
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  class VariableIterator
  {
  public:
    virtual ~VariableIterator() {}

    virtual bool exists() const = 0;
    virtual Variable getCurrentVariable(size_t& index) const = 0;
    virtual void next() = 0;
  };

  // optional specialized variable iterator
  virtual VariableIterator* createVariablesIterator() const
    {return NULL;}

  /*
  ** Introspection: User Interface
  */
#ifdef LBCPP_UI
  virtual juce::Component* createComponent() const
    {return NULL;}
#endif
  
  lbcpp_UseDebuggingNewOperator

protected:
  friend class ObjectClass;
  
  ClassPtr thisClass;
#ifdef LBCPP_DEBUG_OBJECT_ALLOCATION
  String classNameUnderWhichThisIsKnown;
#endif // LBCPP_DEBUG_OBJECT_ALLOCATION
  
  template<class T>
  friend struct ObjectTraits;

  // utilities
  String defaultToStringImplementation(bool useShortString) const;
  String variablesToString(const String& separator, bool includeTypes = true) const;
  void saveVariablesToXmlAttributes(XmlExporter& exporter) const;
  bool loadVariablesFromXmlAttributes(XmlImporter& importer);
  int compareVariables(ObjectPtr otherObject) const;
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

  virtual String toShortString() const
    {return name;}

  virtual void setName(const String& name)
    {this->name = name;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class NameableObjectClass;

  String name;
};

extern ClassPtr nameableObjectClass;

typedef ReferenceCountedObjectPtr<NameableObject> NameableObjectPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_OBJECT_H_

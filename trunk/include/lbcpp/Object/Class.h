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
| Filename: Class.h                        | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CLASS_H_
# define LBCPP_OBJECT_CLASS_H_

# include "Object.h"

namespace lbcpp
{

class Class : public NameableObject
{
public:
  Class(const String& className, ClassPtr baseClass)
    : NameableObject(className), baseClass(baseClass) {}

  static void declare(ClassPtr classInstance);
  static ClassPtr get(const String& className);
  static bool doClassNameExists(const String& className);
  
  /** Creates dynamically an object of class @a className.
  **
  ** The class @a className must be declared with Class::declare()
  ** and must have a default constructor before being able
  ** to instantiate it dynamically.
  **
  ** @param className : class name.
  **
  ** @return an instance of @a className object.
  ** @see Object::declare
  */
  static ObjectPtr createInstance(const String& className);

  template<class T>
  static ReferenceCountedObjectPtr<T> createInstanceAndCast(const String& className)
    {return checkCast<T>(T("Class::createInstanceAndCast"), create(className));}

  /*
  ** Default Constructor
  */
  typedef ObjectPtr (*DefaultConstructor)();

  DefaultConstructor getDefaultConstructor() const
    {return defaultConstructor;}

  /*
  ** Base class
  */
  ClassPtr getBaseClass() const
    {return baseClass;}

  /*
  ** Variables
  */
  size_t getNumVariables() const;
  ClassPtr getVariableType(size_t index) const;
  const String& getVariableName(size_t index) const;
  int findVariable(const String& name) const;

protected:
  DefaultConstructor defaultConstructor;
  ClassPtr baseClass;
  std::vector< std::pair<ClassPtr, String> > variables;
  CriticalSection variablesLock;

  void addVariable(ClassPtr type, const String& name);
  void addVariable(const String& typeName, const String& name)
    {addVariable(Class::get(typeName), name);}
};

/*
** The top-level base class
*/
class ObjectClass : public Class
{
public:
  ObjectClass() : Class(T("Object"), ClassPtr()) {}

  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Object")); return res;}
};

/*
** Builtin-types
*/
class IntegerClass : public Class
{
public:
  IntegerClass() : Class(T("Integer"), ObjectClass::getInstance()) {}

  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Integer")); return res;}
};

class DoubleClass : public Class
{
public:
  DoubleClass() : Class(T("Double"), ObjectClass::getInstance()) {}
  
  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Double")); return res;}
};

class StringClass : public Class
{
public:
  StringClass() : Class(T("String"), ObjectClass::getInstance()) {}
  
  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("String")); return res;}
};

/*
** Enumeration
*/
class Enumeration : public Class
{
public:
  Enumeration(const String& name)
    : Class(name, ObjectClass::getInstance()) {}

  virtual size_t getNumElements() const = 0;
  virtual String getElementName(size_t index) const = 0;

  virtual ClassPtr getBaseClass() const
    {return IntegerClass::getInstance();}
};

typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class StaticEnumeration : public Enumeration
{
public:
  StaticEnumeration(const String& name, const juce::tchar** elements)
    : Enumeration(name), elements(elements)
  {
    for (numElements = 0; elements[numElements]; ++numElements)
      ;
  }
  
  virtual size_t getNumElements() const
    {return numElements;}

  virtual String getElementName(size_t index) const
    {jassert(index < numElements); return elements[index];}

private:
  const juce::tchar** elements;
  size_t numElements;
};

/*
** Collection
*/
class Collection : public Class
{
public:
  Collection(const String& name) : Class(name, ObjectClass::getInstance()) {}

  virtual size_t getNumElements() const = 0;
  virtual ObjectPtr getElement(size_t index) const = 0;
};

typedef ReferenceCountedObjectPtr<Collection> CollectionPtr;

class VectorBasedCollection : public Collection
{
public:
  VectorBasedCollection(const String& name)
    : Collection(name)
    {}

  virtual size_t getNumElements() const
    {return objects.size();}

  virtual ObjectPtr getElement(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

protected:
  void addElement(ObjectPtr object)
    {objects.push_back(object);}

private:
  std::vector<ObjectPtr> objects;
};

/*
** Minimalistic C++ classes Wrapper
*/
template<class Type>
class DefaultClass_ : public Class
{
public:
  DefaultClass_(ClassPtr baseClass = ObjectClass::getInstance())
    : Class(lbcpp::toString(typeid(Type)), baseClass)
    {Class::defaultConstructor = defaultCtor;}

  static ObjectPtr defaultCtor()
    {return ObjectPtr(new Type());}
};

#define LBCPP_DECLARE_CLASS(Name) lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>()))

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_

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

class BuiltinTypeClass : public Class
{
public:
  BuiltinTypeClass(const String& name, ClassPtr baseClass = ClassPtr())
    : Class(name, baseClass ? baseClass : ObjectClass::getInstance()) {}
};

/*
** Builtin-types
*/
class BooleanClass : public BuiltinTypeClass
{
public:
  BooleanClass() : BuiltinTypeClass(T("Boolean")) {}
  
  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Boolean")); return res;}
};

class IntegerClass : public BuiltinTypeClass
{
public:
  IntegerClass(const String& className, ClassPtr baseClass)
    : BuiltinTypeClass(className, baseClass) {}
  IntegerClass() : BuiltinTypeClass(T("Integer")) {}

  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Integer")); return res;}

  virtual String toString(int value) const
    {return lbcpp::toString(value);}
};

typedef ReferenceCountedObjectPtr<IntegerClass> IntegerClassPtr;

class DoubleClass : public BuiltinTypeClass
{
public:
  DoubleClass() : BuiltinTypeClass(T("Double")) {}
  
  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("Double")); return res;}
};

class StringClass : public BuiltinTypeClass
{
public:
  StringClass() : BuiltinTypeClass(T("String")) {}
  
  static ClassPtr getInstance()
    {static ClassPtr res = Class::get(T("String")); return res;}
};

/*
** Enumeration
*/
class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class Enumeration : public IntegerClass
{
public:
  Enumeration(const String& name, const juce::tchar** elements);
  Enumeration(const String& name, const String& elementChars);
  Enumeration(const String& name);

  static EnumerationPtr get(const String& className)
    {return checkCast<Enumeration>(T("Enumeration::get"), Class::get(className));}

  virtual String toString(int value) const
    {return value >= 0 && value < (int)getNumElements() ? getElementName(value) : T("N/A");}

  size_t getNumElements() const
    {return elements.size();}

  String getElementName(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

protected:
  void addElement(const String& elementName);

private:
  std::vector<String> elements;
};

/*
** Collection
*/
class Collection;
typedef ReferenceCountedObjectPtr<Collection> CollectionPtr;

class Collection : public Class
{
public:
  Collection(const String& name)
    : Class(name, ObjectClass::getInstance()) {}

  static CollectionPtr get(const String& className)
    {return checkCast<Collection>(T("Collection::get"), Class::get(className));}

  size_t getNumElements() const
    {return objects.size();}

  ObjectPtr getElement(size_t index) const
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
class DefaultAbstractClass_ : public Class
{
public:
  DefaultAbstractClass_(ClassPtr baseClass = ObjectClass::getInstance())
    : Class(lbcpp::toString(typeid(Type)), baseClass)
    {}
};

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

#define LBCPP_DECLARE_ABSTRACT_CLASS(Name, BaseClass) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultAbstractClass_<Name>(lbcpp::Class::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS(Name, BaseClass) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>(lbcpp::Class::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS_LEGACY(Name) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>()))

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_

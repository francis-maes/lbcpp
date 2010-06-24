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
  Class(const String& className)
    : NameableObject(className) {}

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

  virtual DefaultConstructor getDefaultConstructor() const
    {return NULL;}

  /*
  ** Base class
  */
  virtual ClassPtr getBaseClass() const
    {return ClassPtr();}

  /*
  ** Variables
  */
  size_t getNumVariables() const;
  ClassPtr getVariableType(size_t index) const;
  const String& getVariableName(size_t index) const;
  int findVariable(const String& name) const;

protected:
  std::vector< std::pair<ClassPtr, String> > variables;
  CriticalSection variablesLock;

  void addVariable(ClassPtr type, const String& name);
};

template<class Type>
class DefaultClass_ : public Class
{
public:
  DefaultClass_(ClassPtr baseClass = ClassPtr())
    : Class(lbcpp::toString(typeid(Type))), baseClass(baseClass)
    {}

  virtual DefaultConstructor getDefaultConstructor() const
    {return defaultConstructor;}

  static ObjectPtr defaultConstructor()
    {return ObjectPtr(new Type());}
  
  virtual ClassPtr getBaseClass() const
    {return baseClass;}

protected:
  ClassPtr baseClass;
};

#define LBCPP_DECLARE_CLASS(Name) lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>()))

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_

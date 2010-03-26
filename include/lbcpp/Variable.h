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
| Filename: Variable.h                     | A class that encapsulates a     |
| Author  : Francis Maes                   |     C++ variable                |
| Started : 12/03/2009 14:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Variable.h
**@author Francis MAES
**@date   Sat Jun 13 18:10:51 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_VARIABLE_H_
# define LBCPP_VARIABLE_H_

# include "ObjectPredeclarations.h"
# include "ChooseFunction.h"

namespace lbcpp
{

/*!
** @class Variable
** @brief
*/
class Variable : public Object
{
public:
  /*!
  **
  **
  ** @param ptr
  ** @param typeName
  ** @param name
  **
  ** @return
  */
  Variable(void* ptr, const String& typeName = "", const String& name = "")
    : ptr(ptr), typeName(typeName), name(name) {}

  /*!
  **
  **
  **
  ** @return
  */
  template<class T>
  inline T& getReference()
    {return *(T* )ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  template<class T>
  inline const T& getConstReference() const
    {return *(const T* )ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  template<class T>
  inline T getCopy() const
    {return *(const T* )ptr;}

  /*!
  **
  **
  **
  ** @return
  */
  inline const void* getUntypedPointer() const
    {return this ? ptr : NULL;}

  /*!
  **
  **
  **
  ** @return
  */
  inline void*& getUntypedPointer()
    {return ptr;}

  /*!
  **
  **
  ** @param value
  ** @param typeName
  ** @param name
  **
  ** @return
  */
  template<class T>
  static VariablePtr create(const T& value, const String& typeName = "", const String& name = "");

  /*!
  **
  **
  ** @param value
  ** @param typeName
  ** @param name
  **
  ** @return
  */
  static VariablePtr createFromPointer(void* value, const String& typeName = "", const String& name = "")
    {return VariablePtr(new Variable(value, typeName, name));}

  /*!
  **
  **
  ** @param value
  ** @param typeName
  ** @param name
  **
  ** @return
  */
  template<class T>
  static VariablePtr createFromPointer(T* value, const String& typeName = "", const String& name = "")
    {return create(*value, typeName, name);}

  // Object
  /*!
  **
  **
  **
  ** @return
  */
  virtual String getName() const
    {return name;}

  /*!
  **
  **
  ** @param otherVariable
  **
  ** @return
  */
  virtual bool equals(const VariablePtr otherVariable) const
    {assert(false); return false;}

  /*!
  **
  **
  **
  ** @return
  */
  String getTypeName() const
    {return typeName;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual String toString() const
    {return "Variable " + getTypeName() + " " + getName();}

protected:
  void* ptr;                    /*!< */
  String typeName;         /*!< */
  String name;             /*!< */
};


/*!
** @class VariableIterator
** @brief
*/
class VariableIterator : public ReferenceCountedObject
{
public:
  /*!
  **
  **
  ** @param value
  **
  ** @return
  */
  template<class T>
  static VariableIteratorPtr create(const T& value);

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool exists() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual VariablePtr get() const = 0;

  /*!
  **
  **
  */
  virtual void next() = 0;

  /*!
  **
  **
  **
  ** @return
  */
  String toString() const
    {return exists() ? get()->toString() : "<null>";}
};

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_H_

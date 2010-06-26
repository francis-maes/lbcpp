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
| Filename: Variable.h                     | Variable                        |
| Author  : Francis Maes                   |  (builtin-type or pointer to    |
| Started : 15/06/2010 20:30               |      object)                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VARIABLE_H_
# define LBCPP_OBJECT_VARIABLE_H_

# include "Object.h"
# include "Class.h"

namespace lbcpp
{

class Variable
{
public:
  Variable(bool boolValue, ClassPtr type = booleanClass());
  Variable(int intValue, ClassPtr type = integerClass());
  Variable(double doubleValue, ClassPtr type = doubleClass());
  Variable(const String& stringValue, ClassPtr type = stringClass());
  Variable(ObjectPtr object);
  Variable(Object* object);
  Variable(ClassPtr type, const VariableValue& value) : type(type), value(value) {}

  template<class T>
  Variable(ReferenceCountedObjectPtr<T> object);
  Variable(const Variable& otherVariant);
  Variable() {}

  static Variable pair(const Variable& variable1, const Variable& variable2);

  ~Variable();

  void clear();

  Variable& operator =(const Variable& otherVariant);

  ClassPtr getType() const;

  operator bool() const;
  operator ObjectPtr() const;

  bool isNil() const;

  bool isBoolean() const;
  bool getBoolean() const;

  bool isInteger() const;
  int getInteger() const;

  bool isDouble() const;
  double getDouble() const;

  bool isString() const;
  String getString() const;

  bool isObject() const;
  ObjectPtr getObject() const;

  template<class O>
  ReferenceCountedObjectPtr<O> dynamicCast() const;

  /*
  ** Operations
  */
  String toString() const;
  bool equals(const Variable& otherValue) const;

  size_t size() const;
  Variable operator [](size_t index) const;

private:
  ClassPtr type;
  VariableValue value;
};

}; /* namespace lbcpp */

# include "impl/Variable.hpp"

#endif // !LBCPP_OBJECT_VARIABLE_H_

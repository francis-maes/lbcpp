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
# include "Type.h"

namespace lbcpp
{

class Variable
{
public:
  Variable(bool boolValue, TypePtr type = booleanType());
  Variable(size_t intValue, TypePtr type = integerType());
  Variable(int intValue, TypePtr type = integerType());
  Variable(double doubleValue, TypePtr type = doubleType());
  Variable(const String& stringValue, TypePtr type = stringType());
  Variable(ObjectPtr object);
  Variable(Object* object);

  template<class T>
  Variable(ReferenceCountedObjectPtr<T> object);
  Variable(const Variable& other);
  Variable();
  
  static Variable create(TypePtr type);
  static Variable createFromString(TypePtr type, const String& value, ErrorHandler& callback = ErrorHandler::getInstance());
  static Variable createFromXml(XmlElement* xml, ErrorHandler& callback = ErrorHandler::getInstance());
  static Variable createFromFile(const File& file, ErrorHandler& callback = ErrorHandler::getInstance());

  static Variable missingValue(TypePtr type);

  static Variable pair(const Variable& variable1, const Variable& variable2);
  static Variable copyFrom(TypePtr type, const VariableValue& value);
  void copyTo(VariableValue& dest) const;
    
  ~Variable();

  void clear();

  TypePtr getType() const;
  String getTypeName() const;

  operator bool() const;
  operator ObjectPtr() const;

  bool isMissingValue() const;

  bool isNil() const;

  bool isBoolean() const;
  bool getBoolean() const;

  bool isInteger() const;
  int getInteger() const;

  bool isEnumeration() const;

  bool isDouble() const;
  double getDouble() const;

  bool isString() const;
  String getString() const;

  bool isObject() const;
  ObjectPtr getObject() const;
  template<class O>
  ReferenceCountedObjectPtr<O> getObjectAndCast() const;

  template<class O>
  ReferenceCountedObjectPtr<O> dynamicCast() const;

  /*
  ** Const Operations
  */
  String toString() const;
  XmlElement* toXml(const String& tagName = T("var"), const String& name = String::empty) const;

  bool saveToFile(const File& file, ErrorHandler& callback = ErrorHandler::getInstance()) const;

  int compare(const Variable& otherValue) const;
  bool equals(const Variable& otherValue) const;
  
  bool operator ==(const Variable& otherVariable) const
    {return equals(otherVariable);}

  bool operator !=(const Variable& otherVariable) const
    {return !equals(otherVariable);}

  bool operator <(const Variable& otherVariable) const
    {return compare(otherVariable) < 0;}

  bool operator <=(const Variable& otherVariable) const
    {return compare(otherVariable) <= 0;}

  bool operator >=(const Variable& otherVariable) const
    {return compare(otherVariable) >= 0;}

  bool operator >(const Variable& otherVariable) const
    {return compare(otherVariable) > 0;}

  size_t size() const;
  Variable operator [](size_t index) const;

  friend std::ostream& operator <<(std::ostream& ostr, const Variable& variable)
    {return ostr << variable.toString();}

  /*
  ** Non-const operations
  */
  Variable& operator =(const Variable& other);
  void multiplyByScalar(double scalar);
  void addWeighted(const Variable& other, double weight);

  Variable& operator *=(double scalar)
    {multiplyByScalar(scalar); return *this;}

  Variable& operator /=(double scalar)
    {jassert(scalar); multiplyByScalar(1.0 / scalar); return *this;}
  
  Variable& operator +=(const Variable& other)
    {addWeighted(other, 1.0); return *this;}

  Variable& operator -=(const Variable& other)
    {addWeighted(other, -1.0); return *this;}

private:
  Variable(TypePtr type, const VariableValue& value) : type(type), value(value) {}

  TypePtr type;
  VariableValue value;
};

}; /* namespace lbcpp */

# include "impl/Variable.hpp"

#endif // !LBCPP_OBJECT_VARIABLE_H_

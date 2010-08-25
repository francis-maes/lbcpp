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
# include "TemplateType.h"

namespace lbcpp
{

class Variable
{
public:
  Variable(bool boolValue, TypePtr type = booleanType());
  Variable(size_t intValue, TypePtr type = integerType());
  Variable(juce::int64 intValue, TypePtr type = integerType());
  Variable(int intValue, TypePtr type = integerType());
  Variable(double doubleValue, TypePtr type = doubleType());
  Variable(const String& stringValue, TypePtr type = stringType());
  Variable(const File& fileValue, TypePtr type = fileType());
  Variable(ObjectPtr object);
  Variable(Object* object);

  template<class T>
  Variable(ReferenceCountedObjectPtr<T> object);
  Variable(const Variable& other);
  Variable();
  
  /** Creates dynamically an object of type @a type.
  **
  ** The type @a type must be declared with Type::declare()
  ** and must have a default constructor to be able
  ** to instantiate it dynamically.
  **
  ** @param type : type
  **
  ** @return an instance of @a type Variable.
  ** @see Type::declare
  */
  static Variable create(TypePtr type);

  static Variable createFromString(TypePtr type, const String& value, ErrorHandler& callback = ErrorHandler::getInstance());
  static Variable createFromXml(XmlElement* xml, ErrorHandler& callback = ErrorHandler::getInstance());

  /**
  ** Loads a variable from a file.
  **
  ** @param file : the file to load
  **
  ** @return the loaded Variable or Nil if any error occurs.
  ** @see saveToFile
  */
  static Variable createFromFile(const File& file, ErrorHandler& callback = ErrorHandler::getInstance());

  static Variable missingValue(TypePtr type);

  static Variable pair(const Variable& variable1, const Variable& variable2);
  static Variable copyFrom(TypePtr type, const VariableValue& value);
  void copyTo(VariableValue& dest) const;
    
  ~Variable();

  void clear();

  TypePtr getType() const;
  String getTypeName() const;
  
  bool inheritsFrom(TypePtr baseType) const
    {return getType()->inheritsFrom(baseType);}

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

  bool isFile() const;
  File getFile() const;

  bool isObject() const;
  ObjectPtr getObject() const;
  template<class O>
  ReferenceCountedObjectPtr<O> getObjectAndCast(ErrorHandler& callback = ErrorHandler::getInstance()) const;

  template<class O>
  ReferenceCountedObjectPtr<O> dynamicCast() const;

  /*
  ** Const Operations
  */
  String toString() const;
  String getShortSummary() const;
  XmlElement* toXml(const String& tagName = T("variable"), const String& name = String::empty) const;

  /**
  ** Saves variable to a file
  **
  ** @param file : output file
  ** @param callback : A callback that can receive errors and warnings
  **
  ** @return false if any error occurs.
  ** @see createFromFile
  */
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

  friend std::ostream& operator <<(std::ostream& ostr, const Variable& variable)
    {return ostr << (const char* )variable.toString();}

  /*
  ** Dynamic variables
  */
  size_t size() const;
  String getName(size_t index) const;
  Variable operator [](size_t index) const;

  void printRecursively(std::ostream& ostr, int maxDepth = -1, bool displayMissingValues = true);

  /*
  ** Non-const operations
  */
  Variable& operator =(const Variable& other);

  juce_UseDebuggingNewOperator

private:
  Variable(TypePtr type, const VariableValue& value) : type(type), value(value) {}

  TypePtr type;
  VariableValue value;
};

}; /* namespace lbcpp */

# include "impl/Variable.hpp"

#endif // !LBCPP_OBJECT_VARIABLE_H_

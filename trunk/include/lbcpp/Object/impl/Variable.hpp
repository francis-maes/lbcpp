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
| Filename: Variable.hpp                   | Variable                        |
| Author  : Francis Maes                   |  (builtin-type or pointer to    |
| Started : 15/06/2010 23:32               |      object)                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_IMPL_VARIABLE_HPP_
# define LBCPP_OBJECT_IMPL_VARIABLE_HPP_

# include "../Variable.h"

namespace lbcpp
{

/*
** VariableValue
*/
inline VariableValue::VariableValue(bool boolValue)
  {u.boolValue = boolValue;}

inline VariableValue::VariableValue(int intValue)
  {u.intValue = intValue;} 

inline VariableValue::VariableValue(double doubleValue)
  {u.doubleValue = doubleValue;}

inline VariableValue::VariableValue(const String& stringValue)
  {u.stringValue = new String(stringValue);}

inline VariableValue::VariableValue(Object* objectValue)
{
  u.objectValue = objectValue;
  if (objectValue)
    objectValue->incrementReferenceCounter();
}

template<class T>
inline VariableValue::VariableValue(ReferenceCountedObjectPtr<T> objectValue)
{
  u.objectValue = objectValue.get();
  if (objectValue)
    objectValue->incrementReferenceCounter();
}

inline VariableValue::VariableValue(const VariableValue& other)
  {memcpy(this, &other, sizeof (VariableValue));}

inline VariableValue::VariableValue()
  {memset(this, 0, sizeof (VariableValue));}

inline void VariableValue::clear(ClassPtr type)
{
  if (type)
  {
    if (type.isInstanceOf<StringClass>())
      clearString();
    else if (!type.isInstanceOf<BuiltinTypeClass>())
      clearObject();
  }
}

inline void VariableValue::clearObject()
{
  if (u.objectValue)
  {
    u.objectValue->decrementReferenceCounter();
    u.objectValue = NULL;
  }
}

inline void VariableValue::clearString()
{
  delete u.stringValue;
  u.stringValue = NULL;
}

inline bool VariableValue::getBoolean() const
  {return u.boolValue;}

inline int VariableValue::getInteger() const
  {return u.intValue;}

inline double VariableValue::getDouble() const
  {return u.doubleValue;}

inline const String& VariableValue::getString() const
  {return *u.stringValue;}

inline ObjectPtr VariableValue::getObject() const
  {return u.objectValue ? ObjectPtr(u.objectValue) : ObjectPtr();}

inline Object* VariableValue::getObjectPointer() const
  {return u.objectValue ? u.objectValue : NULL;}

/*
** Variable
*/
inline Variable::Variable(bool boolValue, ClassPtr type)
  : type(type), value(boolValue) {jassert(isBoolean());}

inline Variable::Variable(int intValue, ClassPtr type)
  : type(type), value(intValue) {jassert(isInteger());} 

inline Variable::Variable(double doubleValue, ClassPtr type)
  : type(type), value(doubleValue) {jassert(isDouble());}

inline Variable::Variable(const String& stringValue, ClassPtr type)
  : type(type), value(stringValue) {jassert(isString());}

inline Variable::Variable(ObjectPtr object)
  : type(object ? object->getClass() : ClassPtr()), value(object) {jassert(type || !object);}

inline Variable::Variable(Object* object)
  : type(object ? object->getClass() : ClassPtr()), value(object) {jassert(type || !object);}

template<class T>
inline Variable::Variable(ReferenceCountedObjectPtr<T> object)
  : type(object ? object->getClass() : ClassPtr()), value(object) {jassert(type || !object);}

inline Variable::Variable(const Variable& otherVariant)
  {*this = otherVariant;}

inline Variable::~Variable()
  {clear();}

inline void Variable::clear()
  {value.clear(type); type = ClassPtr();}

inline Variable& Variable::operator =(const Variable& otherVariant)
{
  clear();
  type = otherVariant.getType();
  if (isNil())
    return *this;
  else if (isBoolean())
    value = VariableValue(otherVariant.getBoolean());
  else if (isInteger())
    value = VariableValue(otherVariant.getInteger());
  else if (isDouble())
    value = VariableValue(otherVariant.getDouble());
  else if (isString())
    value = VariableValue(otherVariant.getString());
  else if (isObject())
    value = VariableValue(otherVariant.getObject());
  else
  {
    Object::error(T("Variable::operator ="), T("Unrecognized type of variant"));
    jassert(false);
  }
  return *this;
}

inline ClassPtr Variable::getType() const
  {return type;}

inline Variable::operator bool() const
  {return type;}

inline Variable::operator ObjectPtr() const
  {return isNil() ? ObjectPtr() : getObject();}

inline bool Variable::isNil() const
  {return !type;}

inline bool Variable::isBoolean() const
  {return type && type.isInstanceOf<BooleanClass>();}

inline bool Variable::getBoolean() const
  {jassert(isBoolean()); return value.getBoolean();}

inline bool Variable::isInteger() const
  {return type && type.isInstanceOf<IntegerClass>();}

inline int Variable::getInteger() const
  {jassert(isInteger()); return value.getInteger();}

inline bool Variable::isDouble() const
  {return type && type.isInstanceOf<DoubleClass>();}

inline double Variable::getDouble() const
  {jassert(isDouble()); return value.getDouble();}

inline bool Variable::isString() const
  {return type && type.isInstanceOf<StringClass>();}

inline String Variable::getString() const
  {jassert(isString()); return value.getString();}

inline bool Variable::isObject() const
  {return type && !type.isInstanceOf<BuiltinTypeClass>();}

inline ObjectPtr Variable::getObject() const
  {jassert(isObject()); return value.getObject();}

template<class O>
inline ReferenceCountedObjectPtr<O> Variable::dynamicCast() const
{
  if (isNil())
    return ReferenceCountedObjectPtr<O>();
  jassert(isObject());
  Object* ptr = value.getObjectPointer();
  if (ptr)
  {
    O* res = dynamic_cast<O* >(ptr);
    if (res)
      return ReferenceCountedObjectPtr<O>(res);
  }
  return ReferenceCountedObjectPtr<O>();
}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_HPP_

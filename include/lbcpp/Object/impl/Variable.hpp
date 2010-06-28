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
{
  if (type)
  {
    type->destroy(value);
    type = ClassPtr();
  }
}

inline void Variable::copyTo(VariableValue& dest) const
{
  if (type)
  {
    type->destroy(dest);
    type->copy(dest, value);
  }
}

inline Variable& Variable::operator =(const Variable& otherVariant)
{
  clear();
  if (type != otherVariant.type)
    type = otherVariant.type;
  if (type)
    type->copy(value, otherVariant.value);
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
  {return type && type->inheritsFrom(booleanClass());}

inline bool Variable::getBoolean() const
  {jassert(isBoolean()); return value.getBoolean();}

inline bool Variable::isInteger() const
  {return type && type->inheritsFrom(integerClass());}

inline int Variable::getInteger() const
  {jassert(isInteger()); return value.getInteger();}

inline bool Variable::isDouble() const
  {return type && type->inheritsFrom(doubleClass());}

inline double Variable::getDouble() const
  {jassert(isDouble()); return value.getDouble();}

inline bool Variable::isString() const
  {return type && type->inheritsFrom(stringClass());}

inline String Variable::getString() const
  {jassert(isString()); return value.getString();}

inline bool Variable::isObject() const
  {return type && type->inheritsFrom(objectClass());}

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

template<class O>
inline ReferenceCountedObjectPtr<O> Variable::getObjectAndCast() const
{
  if (isObject())
    return checkCast<O>(T("Variable::getObjectAndCast"), getObject());
  else
  {
    Object::error(T("Variable::getObjectAndCast"), T("This variable is not an object"));
    return ReferenceCountedObjectPtr<O>();
  }
}

inline String Variable::toString() const
  {return type ? type->toString(value) : T("Nil");}

inline bool Variable::equals(const Variable& otherValue) const
{
  if (type)
    return otherValue.getType() == type && type->equals(value, otherValue.value);
  else
    return !otherValue.getType();
}

inline size_t Variable::size() const
  {return type ? type->getNumSubVariables(value) : 0;}

inline Variable Variable::operator [](size_t index) const
  {return type ? type->getSubVariable(value, index) : Variable();}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_HPP_

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

inline Variable::Variable(bool boolValue, TypePtr type)
  : type(type), value(boolValue) {jassert(isBoolean());}

inline Variable::Variable(int intValue, TypePtr type)
  : type(type), value(intValue) {jassert(isInteger());} 

inline Variable::Variable(size_t intValue, TypePtr type)
  : type(type), value((int)intValue) {jassert(isInteger());} 

inline Variable::Variable(double doubleValue, TypePtr type)
  : type(type), value(doubleValue) {jassert(isDouble());}

inline Variable::Variable(const String& stringValue, TypePtr type)
  : type(type), value(stringValue) {jassert(isString());}

inline Variable::Variable(ObjectPtr object)
  : type(object ? object->getClass() : nilType()), value(object) {jassert(type || !object);}

inline Variable::Variable(Object* object)
  : type(object ? object->getClass() : nilType()), value(object) {jassert(type || !object);}

template<class T>
inline Variable::Variable(ReferenceCountedObjectPtr<T> object)
  : type(object ? object->getClass() : nilType()), value(object) {jassert(type || !object);}

inline Variable::Variable(const Variable& otherVariant)
  : type(nilType()), value()
  {*this = otherVariant;}

inline Variable::Variable()
  : type(nilType()), value() {}

inline Variable::~Variable()
  {clear();}

inline void Variable::clear()
{
  if (type)
    type->destroy(value);
  type = nilType();
}

inline Variable Variable::create(TypePtr type)
  {return Variable(type, type->create());}

inline void Variable::copyTo(VariableValue& dest) const
{
  type->destroy(dest);
  type->copy(dest, value);
}

inline TypePtr Variable::getType() const
  {return type;}

inline String Variable::getTypeName() const
  {return type->getName();}

inline Variable::operator bool() const
  {return !isNil();}

inline Variable::operator ObjectPtr() const
  {return isNil() ? ObjectPtr() : getObject();}

inline bool Variable::isNil() const
  {return getType() == nilType();}

inline bool Variable::isBoolean() const
  {return type->inheritsFrom(booleanType());}

inline bool Variable::getBoolean() const
  {jassert(isBoolean()); return value.getBoolean();}

inline bool Variable::isInteger() const
  {return type->inheritsFrom(integerType());}

inline int Variable::getInteger() const
  {jassert(isInteger()); return value.getInteger();}

inline bool Variable::isEnumeration() const
  {return type.dynamicCast<Enumeration>();}

inline bool Variable::isDouble() const
  {return type->inheritsFrom(doubleType());}

inline double Variable::getDouble() const
  {jassert(isDouble()); return value.getDouble();}

inline bool Variable::isString() const
  {return type->inheritsFrom(stringType());}

inline String Variable::getString() const
  {jassert(isString()); return value.getString();}

inline bool Variable::isObject() const
  {return type->inheritsFrom(objectClass());}

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
    if (!isNil())
      Object::error(T("Variable::getObjectAndCast"), T("This variable is not an object"));
    return ReferenceCountedObjectPtr<O>();
  }
}

inline String Variable::toString() const
  {return type->toString(value);}

inline bool Variable::equals(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.getType();
  return type == type2 && type->compare(value, otherValue.value) == 0;
}

inline int Variable::compare(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.getType();
  if (type != type2)
  {
    if (type->inheritsFrom(type2))
      return type2->compare(value, otherValue.value);
    else if (type2->inheritsFrom(type))
      return type->compare(value, otherValue.value);
    else
      return getTypeName().compare(otherValue.getTypeName());
  }
  return type->compare(value, otherValue.value);
}

inline size_t Variable::size() const
  {return type->getNumSubVariables(value);}

inline Variable Variable::operator [](size_t index) const
  {return type->getSubVariable(value, index);}

inline Variable& Variable::operator =(const Variable& otherVariant)
{
  clear();
  if (type != otherVariant.type)
    type = otherVariant.type;
  type->copy(value, otherVariant.value);
  return *this;
}

inline void Variable::multiplyByScalar(double scalar)
{
  if (isNil() || scalar == 1.0)
    return;
  if (scalar == 0.0)
    clear();
  else
    type = type->multiplyByScalar(value, scalar);
}

inline void Variable::addWeighted(const Variable& other, double weight)
{
  if (isNil())
  {
    *this = other;
    multiplyByScalar(weight);
  }
  else
    type = type->addWeighted(value, other, weight);
}

inline bool checkInheritance(const Variable& variable, TypePtr baseType)
  {return variable.isNil() || checkInheritance(variable.getType(), baseType);}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_HPP_

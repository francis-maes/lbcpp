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

inline Variable::Variable(juce::int64 intValue, TypePtr type)
  : type(type), value((int)intValue) {jassert(isInteger());}

inline Variable::Variable(int intValue, TypePtr type)
  : type(type), value(intValue) {jassert(isInteger());} 

inline Variable::Variable(size_t intValue, TypePtr type)
  : type(type), value((int)intValue) {jassert(isInteger());} 

inline Variable::Variable(double doubleValue, TypePtr type)
  : type(type), value(doubleValue) {jassert(isDouble());}

inline Variable::Variable(const String& stringValue, TypePtr type)
  : type(type), value(stringValue) {jassert(isString());}

inline Variable::Variable(const File& fileValue, TypePtr type)
  : type(type), value(fileValue.getFullPathName()) {jassert(isString());}

inline Variable::Variable(ObjectPtr object)
  : type(object ? (TypePtr)object->getClass() : nilType), value(object) {jassert(type || !object);}

inline Variable::Variable(Object* object)
  : type(object ? (TypePtr)object->getClass() : nilType), value(object) {jassert(type || !object);}

template<class T>
inline Variable::Variable(ReferenceCountedObjectPtr<T> object, TypePtr expectedType)
  : type(object ? (TypePtr)object->getClass() : expectedType), value(object)
  {jassert(type || !object);} // this object's class has not been declared


inline Variable::Variable(const Variable& otherVariant)
  : type(otherVariant.getType())
  {type->copy(value, otherVariant.value);}

inline Variable::Variable()
  : type(nilType), value() {}

inline Variable::~Variable()
  {type->destroy(value);}

inline void Variable::clear()
  {type->destroy(value); type = nilType;}

inline Variable Variable::create(TypePtr type)
  {jassert(type && type->isInitialized()); return Variable(type, type->create());}

inline Variable Variable::missingValue(TypePtr type)
  {jassert(type); return Variable(type, type->getMissingValue());}

inline void Variable::copyTo(VariableValue& dest) const
  {type->destroy(dest); type->copy(dest, value);}

inline TypePtr Variable::getType() const
  {return type;}

inline String Variable::getTypeName() const
  {return type->getName();}

inline Variable::operator bool() const
  {return !isNil() && !isMissingValue();}

inline Variable::operator ObjectPtr() const
  {return isNil() ? ObjectPtr() : getObject();}

inline bool Variable::isMissingValue() const
  {return !isNil() && type->isMissingValue(value);}

inline bool Variable::isNil() const
  {return type == nilType;}

inline bool Variable::isBoolean() const
  {return type->inheritsFrom(booleanType());}

inline bool Variable::getBoolean() const
  {jassert(isBoolean()); return value.getBoolean();}

inline bool Variable::isInteger() const
  {return type->inheritsFrom(integerType());}

inline int Variable::getInteger() const
  {jassert(isInteger()); return (int)value.getInteger();}

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

inline bool Variable::isFile() const
  {return type->inheritsFrom(fileType());}

inline File Variable::getFile() const
  {jassert(isFile()); return File(value.getString());}

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
inline ReferenceCountedObjectPtr<O> Variable::getObjectAndCast(MessageCallback& callback) const
{
  ReferenceCountedObjectPtr<O> res;
  if (isNil())
  {
    callback.errorMessage(T("Variable::getObjectAndCast"), T("Variable is nil"));
    return res;
  }
  if (!isObject())
  {
    callback.errorMessage(T("Variable::getObjectAndCast"), T("This variable is not an object"));
    return res;
  }
  return value.getObjectAndCast<O>(callback);
}

inline String Variable::toString() const
  {return type->isMissingValue(value) ? T("Missing") : type->toString(value);}

inline String Variable::toShortString() const
  {return type->isMissingValue(value) ? T("Missing") : type->toShortString(value);}

inline bool Variable::equals(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.getType();
  return type == type2 && type->compare(value, otherValue.value) == 0;
}

inline size_t Variable::size() const
  {return type->getNumElements(value);}

inline String Variable::getName(size_t index) const
  {return type->getElementName(value, index);}

inline Variable Variable::operator [](size_t index) const
  {return type->getElement(value, index);}

inline Variable& Variable::operator =(const Variable& otherVariant)
{
  jassert(type);
//  if (type) // type may be NULL when allocated from TupleType (which bypass the constructor of Variable)
  clear();
  type = otherVariant.type;
  type->copy(value, otherVariant.value);
  return *this;
}

/*
** Variable => C++ Native
*/
inline void variableToNative(bool& dest, const Variable& source)
  {jassert(source.isBoolean()); dest = source.getBoolean();}

inline void variableToNative(int& dest, const Variable& source)
  {jassert(source.isInteger()); dest = source.getInteger();}

inline void variableToNative(juce::int64& dest, const Variable& source)
  {jassert(source.isInteger()); dest = source.getInteger();}

inline void variableToNative(size_t& dest, const Variable& source)
  {jassert(source.isInteger() && source.getInteger() >= 0); dest = (size_t)source.getInteger();}

inline void variableToNative(double& dest, const Variable& source)
  {jassert(source.isDouble()); dest = source.getDouble();}

inline void variableToNative(String& dest, const Variable& source)
  {jassert(source.isString()); dest = source.getString();}

inline void variableToNative(File& dest, const Variable& source)
  {jassert(source.isString()); dest = File(source.getString());}

inline void variableToNative(ObjectPtr& dest, const Variable& source)
  {jassert(source.isObject()); dest = source.getObject();}

template<class TT>
inline void variableToNative(ReferenceCountedObjectPtr<TT>& dest, const Variable& source)
  {jassert(source.isObject()); dest = source.getObjectAndCast<TT>();}

inline void variableToNative(Variable& dest, const Variable& source)
  {dest = source;}

/*
** C++ Native => Variable
*/
inline void nativeToVariable(Variable& dest, const Variable& source, TypePtr )
  {dest = source;}

template<class TT>
inline void nativeToVariable(Variable& dest, const ReferenceCountedObjectPtr<TT>& source, TypePtr expectedType)
  {dest = Variable(source, expectedType);}

template<class TT>
inline void nativeToVariable(Variable& dest, const TT& source, TypePtr expectedType)
  {dest = Variable(source, expectedType);}

/*
** Inheritance check
*/
#ifdef JUCE_DEBUG
inline bool checkInheritance(TypePtr type, TypePtr baseType, MessageCallback& callback = MessageCallback::getInstance())
{
  jassert(baseType);
  if (!type || !type->inheritsFrom(baseType))
  {
    callback.errorMessage(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
    return false;
  }
  return true;
}
inline bool checkInheritance(const Variable& variable, TypePtr baseType, MessageCallback& callback = MessageCallback::getInstance())
  {jassert(baseType); return variable.isNil() || checkInheritance(variable.getType(), baseType, callback);}

#else
#define checkInheritance(type, baseType) true
#endif // JUCE_DEBUG

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_HPP_

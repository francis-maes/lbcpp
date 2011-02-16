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

inline Variable::Variable(const juce::tchar* stringValue, TypePtr type)
  : type(type), value(stringValue) {jassert(isString());}

inline Variable::Variable(const String& stringValue, TypePtr type)
  : type(type), value(stringValue) {jassert(isString());}

inline Variable::Variable(const File& fileValue, TypePtr type)
  : type(type), value(fileValue.getFullPathName()) {jassert(isString());}

inline Variable::Variable(const ObjectPtr& object)
  : type(object ? (TypePtr)object->getClass() : nilType), value(object) {jassert(type || !object);}

inline Variable::Variable(Object* object)
  : type(object ? (TypePtr)object->getClass() : nilType), value(object) {jassert(type || !object);}

template<class T>
inline Variable::Variable(const ReferenceCountedObjectPtr<T>& object, TypePtr expectedType)
  : type(object ? (TypePtr)object->getClass() : expectedType), value(object)
  {jassert(type || !object);} // this object's class has not been declared

inline Variable::Variable(const Variable& otherVariable)
  : type(otherVariable.type)
  {type->copy(value, otherVariable.value);}

inline Variable::Variable()
  : type(nilType), value() {}

inline Variable::~Variable()
  {type->destroy(value);}

inline void Variable::clear()
  {type->destroy(value); type = nilType;}

inline Variable Variable::create(TypePtr type)
  {jassert(type && type->isInitialized()); return type->create(defaultExecutionContext());}

inline Variable Variable::missingValue(TypePtr type)
  {jassert(type); return Variable(type, type->getMissingValue());}

inline void Variable::copyTo(VariableValue& dest) const
  {type->destroy(dest); type->copy(dest, value);}

inline TypePtr Variable::getType() const
  {return *(const TypePtr* )&type;}

inline String Variable::getTypeName() const
  {return type->getName();}

inline bool Variable::exists() const
  {return !isNil() && !isMissingValue();}

inline bool Variable::isMissingValue() const
  {return !isNil() && type->isMissingValue(value);}

inline bool Variable::isNil() const
  {return type == nilType;}

inline bool Variable::isBoolean() const
  {return type->inheritsFrom(booleanType);}

inline bool Variable::getBoolean() const
  {jassert(isBoolean()); return value.getBoolean();}

inline bool Variable::isInteger() const
  {return type->inheritsFrom(integerType);}

inline int Variable::getInteger() const
  {jassert(isInteger()); return (int)value.getInteger();}

inline bool Variable::isEnumeration() const
  {return type.isInstanceOf<Enumeration>();}

inline bool Variable::isDouble() const
  {return type->inheritsFrom(doubleType);}

inline double Variable::getDouble() const
  {jassert(isDouble()); return value.getDouble();}

inline bool Variable::isString() const
  {return type->inheritsFrom(stringType);}

inline String Variable::getString() const
  {jassert(isString()); return value.getString();}

inline bool Variable::isFile() const
  {return type->inheritsFrom(fileType);}

inline File Variable::getFile() const
  {jassert(isFile()); return File(value.getString());}

inline bool Variable::isObject() const
  {return type->inheritsFrom(objectClass);}

inline const ObjectPtr& Variable::getObject() const
  {jassert(isNil() || isObject()); return value.getObject();}

template<class O>
inline ReferenceCountedObjectPtr<O> Variable::dynamicCast() const
{
  if (isObject())
  {
    Object* ptr = value.getObjectPointer();
    if (ptr)
    {
      O* res = dynamic_cast<O* >(ptr);
      if (res)
        return res;
    }
  }
  return ReferenceCountedObjectPtr<O>();
}

template<class O>
inline const ReferenceCountedObjectPtr<O>& Variable::getObjectAndCast() const
{
#ifdef JUCE_DEBUG
  static ReferenceCountedObjectPtr<O> empty;
  jassert(isNil() || isObject());
  if (isNil() || !isObject())
    return empty;
#endif
  return value.getObjectAndCast<O>();
}

template<class O>
inline const ReferenceCountedObjectPtr<O>& Variable::getObjectAndCast(ExecutionContext& context) const
{
#ifdef JUCE_DEBUG
  static ReferenceCountedObjectPtr<O> empty;
  if (isNil())
  {
    context.errorCallback(T("Variable::getObjectAndCast"), T("Variable is nil"));
    return empty;
  }
  if (!isObject())
  {
    context.errorCallback(T("Variable::getObjectAndCast"), T("This variable is not an object"));
    return empty;
  }
#endif
  return value.getObjectAndCast<O>(context);
}

inline String Variable::toString() const
  {return type->isMissingValue(value) ? T("Missing") : type->toString(value);}

inline String Variable::toShortString() const
  {return type->isMissingValue(value) ? T("Missing") : type->toShortString(value);}

inline bool Variable::equals(const Variable& otherValue) const
{
  TypePtr type2 = otherValue.type;
  return type == type2 && type->compare(value, otherValue.value) == 0;
}

inline Variable& Variable::operator =(const Variable& otherVariable)
{
  jassert(type);
  clear();
  type = otherVariable.type;
  type->copy(value, otherVariable.value);
  return *this;
}

/*
** Variable => C++ Native
*/
inline void variableToNative(ExecutionContext& context, bool& dest, const Variable& source)
  {jassert(source.isBoolean()); dest = source.getBoolean();}

inline void variableToNative(ExecutionContext& context, int& dest, const Variable& source)
  {jassert(source.isInteger()); dest = source.getInteger();}

inline void variableToNative(ExecutionContext& context, juce::int64& dest, const Variable& source)
  {jassert(source.isInteger()); dest = source.getInteger();}

inline void variableToNative(ExecutionContext& context, size_t& dest, const Variable& source)
  {jassert(source.isInteger() && source.getInteger() >= 0); dest = (size_t)source.getInteger();}

inline void variableToNative(ExecutionContext& context, double& dest, const Variable& source)
  {jassert(source.isDouble()); dest = source.getDouble();}

inline void variableToNative(ExecutionContext& context, String& dest, const Variable& source)
  {jassert(source.isString()); dest = source.getString();}

inline void variableToNative(ExecutionContext& context, File& dest, const Variable& source)
  {jassert(source.isString()); dest = File(source.getString());}

inline void variableToNative(ExecutionContext& context, ObjectPtr& dest, const Variable& source)
  {jassert(source.isObject()); dest = source.getObject();}

template<class TT>
inline void variableToNative(ExecutionContext& context, ReferenceCountedObjectPtr<TT>& dest, const Variable& source)
  {jassert(source.isObject()); dest = source.getObjectAndCast<TT>(context);}

inline void variableToNative(ExecutionContext& context, Variable& dest, const Variable& source)
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
inline bool checkInheritance(TypePtr type, TypePtr baseType)
{
  jassert(baseType);
  bool res = type && type->inheritsFrom(baseType);
  jassert(res);
  return res;
}
inline bool checkInheritance(const Variable& variable, TypePtr baseType)
  {jassert(baseType); return variable.isNil() || checkInheritance(variable.getType(), baseType);}
#else
inline bool checkInheritance(TypePtr type, TypePtr baseType)
  {return true;}

inline bool checkInheritance(const Variable& variable, TypePtr baseType)
  {return true;}
#endif // JUCE_DEBUG

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_HPP_

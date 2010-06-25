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

struct VariableValue
{
  VariableValue(bool boolValue)
    {u.boolValue = boolValue;}
  VariableValue(int intValue)
    {u.intValue = intValue;} 
  VariableValue(double doubleValue)
    {u.doubleValue = doubleValue;}
  VariableValue(const String& stringValue)
    {u.stringValue = new String(stringValue);}

  template<class T>
  VariableValue(ReferenceCountedObjectPtr<T> objectValue)
  {
    u.objectValue = objectValue.get();
    if (objectValue)
      objectValue->incrementReferenceCounter();
  }
  VariableValue(const VariableValue& other)
    {memcpy(this, &other, sizeof (VariableValue));}
  VariableValue()
    {memset(this, 0, sizeof (VariableValue));}

  void clear(ClassPtr type)
  {
    if (type)
    {
      if (type.isInstanceOf<StringClass>())
        clearString();
      else if (!type.isInstanceOf<BuiltinTypeClass>())
        clearObject();
    }
  }

  void clearObject()
  {
    if (u.objectValue)
    {
      u.objectValue->decrementReferenceCounter();
      u.objectValue = NULL;
    }
  }

  void clearString()
  {
    delete u.stringValue;
    u.stringValue = NULL;
  }

  bool getBoolean() const
    {return u.boolValue;}

  int getInteger() const
    {return u.intValue;}

  double getDouble() const
    {return u.doubleValue;}

  const String& getString() const
    {return *u.stringValue;}

  ObjectPtr getObject() const
    {return u.objectValue ? ObjectPtr(u.objectValue) : ObjectPtr();}

  Object* getObjectPointer() const
    {return u.objectValue ? u.objectValue : NULL;}

  ObjectPtr toObject(ClassPtr type) const
  {
    if (type.isInstanceOf<BuiltinTypeClass>())
    {
      jassert(false);
      return ObjectPtr();
    }
    else
      return getObject();
  }

private:
  union
  {
    bool boolValue;
    int intValue;
    double doubleValue;
    String* stringValue;
    Object* objectValue;
  } u;
};

class Variable
{
public:
  Variable(ClassPtr type, bool boolValue)
    : type(type), value(boolValue) {jassert(isBoolean());}
  Variable(ClassPtr type, int intValue)
    : type(type), value(intValue) {jassert(isInteger());} 
  Variable(ClassPtr type, double doubleValue)
    : type(type), value(doubleValue) {jassert(isDouble());}
  Variable(double doubleValue)
    : type(DoubleClass::getInstance()), value(doubleValue) {}
  Variable(ClassPtr type, const String& stringValue)
    : type(type), value(stringValue) {jassert(isString());}
  Variable(ClassPtr type, ObjectPtr objectValue)
    : type(type), value(objectValue) {jassert(isObject());}
  Variable(ObjectPtr object)
    : type(object ? object->getClass() : ClassPtr()), value(object) {jassert(type || !object);}
  template<class T>
  Variable(ReferenceCountedObjectPtr<T> object)
    : type(object ? object->getClass() : ObjectPtr()), value(object) {jassert(type || !object);}
  Variable(const Variable& otherVariant)
    {*this = otherVariant;}
  Variable() {}
  
  ~Variable()
    {clear();}

  void clear()
    {value.clear(type); type = ClassPtr();}

  Variable& operator =(const Variable& otherVariant)
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

  ClassPtr getType() const
    {return type;}

  operator bool() const
    {return type;}

  operator ObjectPtr() const
    {return isNil() ? ObjectPtr() : getObject();}

  bool isNil() const
    {return !type;}

  bool isBoolean() const
    {return type && type.isInstanceOf<BooleanClass>();}

  bool getBoolean() const
    {jassert(isBoolean()); return value.getBoolean();}

  bool isInteger() const
    {return type && type.isInstanceOf<IntegerClass>();}

  int getInteger() const
    {jassert(isInteger()); return value.getInteger();}

  bool isDouble() const
    {return type && type.isInstanceOf<DoubleClass>();}

  double getDouble() const
    {jassert(isDouble()); return value.getDouble();}

  bool isString() const
    {return type && type.isInstanceOf<StringClass>();}

  String getString() const
    {jassert(isString()); return value.getString();}

  bool isObject() const
    {return type && !type.isInstanceOf<BuiltinTypeClass>();}

  ObjectPtr getObject() const
    {jassert(isObject()); return value.getObject();}

  template<class O>
  inline ReferenceCountedObjectPtr<O> dynamicCast() const
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

private:
  ClassPtr type;
  VariableValue value;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VARIABLE_H_

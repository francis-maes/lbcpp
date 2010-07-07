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
| Filename: VariableValue.hpp              | VariableValue                   |
| Author  : Francis Maes                   |  (builtin-type or pointer to    |
| Started : 26/06/2010 13:51               |      object)                    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_IMPL_VARIABLE_VALUE_HPP_
# define LBCPP_OBJECT_IMPL_VARIABLE_VALUE_HPP_

namespace lbcpp
{

struct VariableValue
{
  VariableValue(bool boolValue)
    {u.boolValue = boolValue;}

  VariableValue(size_t intValue)
    {u.intValue = (int)intValue;}

  VariableValue(int intValue)
    {u.intValue = intValue;} 

  VariableValue(double doubleValue)
    {u.doubleValue = doubleValue;}

  VariableValue(const String& stringValue)
    {u.stringValue = new String(stringValue);}

  template<class T>
  VariableValue(ReferenceCountedObjectPtr<T> objectValue);
  VariableValue(Object* objectValue)
    {u.objectValue = NULL; setObject(objectValue);}
  
  VariableValue(char* rawData)
    {u.rawDataValue = rawData;}

  VariableValue(const VariableValue& other)
    {memcpy(this, &other, sizeof (VariableValue));}

  VariableValue()
    {memset(this, 0, sizeof (VariableValue));}

  void clearBuiltin();
  void clearObject();
  void clearString();
  void clearRawData();

  bool getBoolean() const
    {return u.boolValue;}

  void setBoolean(bool value)
    {u.boolValue = value;}
  
  int getInteger() const
    {return u.intValue;}

  void setInteger(int value)
    {u.intValue = value;}

  double getDouble() const
    {return u.doubleValue;}

  void setDouble(double value)
    {u.doubleValue = value;}

  const String& getString() const
    {return *u.stringValue;}

  void setString(const String& str)
    {jassert(!u.stringValue); u.stringValue = new String(str);}

  ObjectPtr getObject() const
    {return u.objectValue ? ObjectPtr(u.objectValue) : ObjectPtr();}

  Object* getObjectPointer() const
    {return u.objectValue ? u.objectValue : NULL;}

  void setObject(Object* pointer)
    {jassert(!u.objectValue); u.objectValue = pointer; if (pointer) pointer->incrementReferenceCounter();}
  
  void setObject(ObjectPtr pointer)
    {setObject(pointer.get());}

  const char* getRawData() const
    {return u.rawDataValue;}
  
  char* getRawData()
    {return u.rawDataValue;}

  void setRawData(char* data)
    {jassert(!u.rawDataValue); u.rawDataValue = data;}

private:
  union
  {
    bool boolValue;
    int intValue;
    double doubleValue;
    String* stringValue;
    Object* objectValue;
    char* rawDataValue;
  } u;
};

template<class T>
inline VariableValue::VariableValue(ReferenceCountedObjectPtr<T> objectValue)
{
  u.objectValue = objectValue.get();
  if (objectValue)
    objectValue->incrementReferenceCounter();
}

inline void VariableValue::clearBuiltin()
  {memset(this, 0, sizeof (*this));}

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
  if (u.stringValue)
  {
    delete u.stringValue;
    u.stringValue = NULL;
  }
}

inline void VariableValue::clearRawData()
{
  if (u.rawDataValue)
  {
    delete [] u.rawDataValue;
    u.rawDataValue = NULL;
  }
}


}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_IMPL_VARIABLE_VALUE_HPP_

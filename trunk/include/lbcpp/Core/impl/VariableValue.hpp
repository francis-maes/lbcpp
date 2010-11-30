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
    {u.intValue = (juce::int64)intValue;}

  VariableValue(int intValue)
    {u.intValue = (juce::int64)intValue;} 

  VariableValue(juce::int64 intValue)
    {u.intValue = intValue;}

  VariableValue(double doubleValue)
    {u.doubleValue = doubleValue;}

  VariableValue(const String& stringValue)
    {u.stringValue = new String(stringValue);}

  template<class T>
  VariableValue(const ReferenceCountedObjectPtr<T>& objectValue)
    {setObject(objectValue.get());}

  template<class T>
  VariableValue(const NativePtr<T>& objectValue)
    {setObject(objectValue.get());}

#ifdef LBCPP_ENABLE_CPP0X_RVALUES
  template<class T>
  VariableValue(ReferenceCountedObjectPtr<T>&& objectValue)
  {
    setObject(objectValue.get());
    objectValue.setPointerToNull();
  }
#endif // LBCPP_ENABLE_CPP0X_RVALUES

  VariableValue(Object* objectValue)
    {setObject(objectValue);}
  
  VariableValue(char* rawData)
    {u.rawDataValue = rawData;}

  VariableValue(const VariableValue& other)
    {jassert(sizeof (*this) == sizeof (u.intValue)); u.intValue = other.u.intValue;}

  VariableValue()
    {jassert(sizeof (*this) == sizeof (u.intValue));  u.intValue = 0;}

  void clearBuiltin();
  void clearObject();
  void clearString();
  void clearRawData();

  bool getBoolean() const
    {return u.boolValue;}

  void setBoolean(bool value)
    {u.boolValue = value;}
  
  juce::int64 getInteger() const
    {return u.intValue;}

  void setInteger(juce::int64 value)
    {u.intValue = value;}

  void setInteger(int value)
    {u.intValue = (int)value;}

  double getDouble() const
    {return u.doubleValue;}

  void setDouble(double value)
    {u.doubleValue = value;}

  const String& getString() const
    {return *u.stringValue;}

  void setString(const String& str)
    {jassert(!u.stringValue); u.stringValue = new String(str);}

  const ObjectPtr& getObject() const
    {return *(const ObjectPtr* )this;}

  template<class O>
  const ReferenceCountedObjectPtr<O>& getObjectAndCast() const
  {
#ifdef JUCE_DEBUG
    if (u.objectValue && !dynamic_cast<O* >(u.objectValue))
    {
      jassert(false);
      static ReferenceCountedObjectPtr<O> empty;
      return empty;
    }
#endif
    return *(const ReferenceCountedObjectPtr<O>* )this;
  }

  template<class O>
  const ReferenceCountedObjectPtr<O>& getObjectAndCast(ExecutionContext& context) const
  {
#ifdef JUCE_DEBUG
    if (u.objectValue && !dynamic_cast<O* >(u.objectValue))
    {
      static ReferenceCountedObjectPtr<O> empty;
      context.errorCallback(T("Variable::getObjectAndCast"), T("Could not cast object from '") + getTypeName(typeid(*u.objectValue)) + T("' to '") + getTypeName(typeid(O)) + T("'"));
      return empty;
    }
#endif
    return *(const ReferenceCountedObjectPtr<O>* )this;
  }

  Object* getObjectPointer() const
    {return u.objectValue;}

  void setObject(Object* pointer)
    {u.objectValue = pointer; if (pointer && !pointer->hasStaticAllocationFlag()) pointer->incrementReferenceCounter();}
  
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
    juce::int64 intValue;
    double doubleValue;
    String* stringValue;
    Object* objectValue;
    char* rawDataValue;
  } u;
};

inline void VariableValue::clearBuiltin()
  {u.intValue = 0;}

inline void VariableValue::clearObject()
{
  if (u.objectValue)
  {
    if (!u.objectValue->hasStaticAllocationFlag()) 
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

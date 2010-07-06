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
| Filename: TupleType.h                    | Tuple types                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_TUPLE_H_
# define LBCPP_OBJECT_TYPE_TUPLE_H_

# include <lbcpp/Object/Variable.h>
# include "BuiltinType.h"

namespace lbcpp
{

class TupleType : public RawDataBuiltinType
{
public:
  TupleType(const String& name, size_t size)
    : RawDataBuiltinType(name), size(size) {}

  static char* allocateMemory(size_t size)
  {
    char* res = new char[sizeof (Variable) * size];
    memset(res, 0, sizeof (Variable) * size);
    return res;
  }
  
  virtual void destroy(VariableValue& value) const
  {
    Variable* data = (Variable* )value.getRawData();
    if (data)
    {
      for (size_t i = 0; i < size; ++i)
        data[i] = Variable();
      RawDataBuiltinType::destroy(value);
    }
  }

  virtual String toString(const VariableValue& value) const
  {
    const Variable* data = (const Variable* )value.getRawData();
    String res;
    for (size_t i = 0; i < size; ++i)
    {
      if (i == 0)
        res = T("(");
      else
        res += T(", ");
      res += data[i].toString();
    }
    return res + T(")");
  }

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    const Variable* sourceData = (const Variable* )source.getRawData();
    Variable* destData = (Variable* )allocateMemory(size);
    for (size_t i = 0; i < size; ++i)
      destData[i] = sourceData[i];
    dest.setRawData((char* )destData);
  }
  
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
  {
    const Variable* data1 = (const Variable* )value1.getRawData();
    const Variable* data2 = (const Variable* )value2.getRawData();
    for (size_t i = 0; i < size; ++i)
    {
      int res = data1[i].compare(data2[i]);
      if (res != 0)
        return res;
    }
    return 0;
  }
  
  virtual size_t getNumStaticVariables() const
    {return size;}

  virtual TypePtr getStaticVariableType(size_t index) const
    {return TypePtr();}

  virtual String getStaticVariableName(size_t index) const
    {return T("[") + String((int)index) + T("]");}

  virtual Variable getSubVariable(const VariableValue& value, size_t index) const
  {
    jassert(index < size);
    return ((const Variable* )value.getRawData())[index];
  }

  virtual void setSubVariable(const VariableValue& value, size_t index, const Variable& subValue) const
  {
    jassert(index < size);
    Variable* data = (Variable* )value.getRawData();
    data[index] = subValue;
  }

protected:
  size_t size;
};

class PairType : public TupleType
{
public:
  PairType() : TupleType(T("Pair"), 2) {}

  static VariableValue allocate(const Variable& variable1, const Variable& variable2)
  {
    char* data = allocateMemory(2);
    ((Variable* )data)[0] = variable1;
    ((Variable* )data)[1] = variable2;
    return data;
  }

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    Variable* sourceData = (Variable* )source.getRawData();
    Variable* destData = (Variable* )allocateMemory(size);
    destData[0] = sourceData[0];
    destData[1] = sourceData[1];
    dest.setRawData((char* )destData);
  }
  
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
  {
    Variable* data1 = (Variable* )value1.getRawData();
    Variable* data2 = (Variable* )value2.getRawData();
    int res = data1[0].compare(data2[0]);
    return res ? res : data1[1].compare(data2[1]);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_TUPLE_H_

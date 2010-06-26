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
| Filename: TupleClass.h                   | Tuple classes                   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CLASS_TUPLE_H_
# define LBCPP_OBJECT_CLASS_TUPLE_H_

# include <lbcpp/Object/Variable.h>
# include "BuiltinTypeClass.h"

namespace lbcpp
{

class TupleClass : public RawDataBuiltinTypeClass
{
public:
  TupleClass(const String& name, size_t size)
    : RawDataBuiltinTypeClass(name), size(size) {}
  
  virtual size_t getNumSubVariables(const VariableValue& value) const
    {return size;}

  static char* allocateMemory(size_t size)
  {
    char* res = new char[sizeof (Variable) * size];
    memset(res, 0, sizeof (Variable) * size);
    return res;
  }

  //virtual Variable getSubVariable(const VariableValue& value, size_t index) const;
  virtual String getSubVariableName(const VariableValue& value, size_t index) const
    {jassert(false); return String::empty;}
  virtual void setSubVariable(const VariableValue& value, size_t index, const VariableValue& subValue) const
    {jassert(false);}
  
protected:
  size_t size;
};

class PairClass : public TupleClass
{
public:
  PairClass() : TupleClass(T("Pair"), 2) {}

  static VariableValue allocate(const Variable& variable1, const Variable& variable2)
  {
    char* data = allocateMemory(2);
    ((Variable* )data)[0] = variable1;
    ((Variable* )data)[1] = variable2;
    return data;
  }

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    jassert(false);
    //dest = allocate(source[0], source[1]);
  }

};

class VariableVectorClass : public RawDataBuiltinTypeClass
{
public:
  VariableVectorClass(ClassPtr elementsClass)
    : RawDataBuiltinTypeClass(T("VariableVector<") + elementsClass->getName() + T(">")), elementsClass(elementsClass) {}

private:
  ClassPtr elementsClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_TUPLE_H_

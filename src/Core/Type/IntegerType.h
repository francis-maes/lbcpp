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
| Filename: IntegerType.h                  | Integer type                    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_INTEGER_H_
# define LBCPP_OBJECT_TYPE_INTEGER_H_

# include "BuiltinType.h"

namespace lbcpp
{

class IntegerType : public BuiltinType
{
public:
  IntegerType(const String& className, TypePtr baseType)
    : BuiltinType(className, baseType) {}
  IntegerType() : BuiltinType(T("Integer")) {}

  virtual Variable create(ExecutionContext& context) const
    {return Variable(0, refCountedPointerFromThis(this));}

  virtual Variable createFromString(ExecutionContext& context, const String& value) const
  {
    if (!value.trim().containsOnly(T("-+e0123456789")))
    {
      context.errorCallback(T("IntegerType::createFromString"), value.quoted() + T(" is not a valid integer"));
      return Variable::missingValue(refCountedPointerFromThis(this));
    }
    return Variable(value.getIntValue(), refCountedPointerFromThis(this));
  }

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setInteger(source.getInteger());}

  virtual String toString(const VariableValue& value) const
    {return String(value.getInteger());}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return (int)(value1.getInteger() - value2.getInteger());}

  lbcpp_UseDebuggingNewOperator
};

class PositiveIntegerType : public IntegerType
{
public:
  PositiveIntegerType(const String& typeName, TypePtr baseType)
    : IntegerType(typeName, baseType) {}
  PositiveIntegerType() : IntegerType(T("PositiveInteger"), integerType) {}

  virtual Variable createFromString(ExecutionContext& context, const String& value) const
  {
    if (!value.trim().containsOnly(T("+e0123456789")))
    {
      context.errorCallback(T("IntegerType::createFromString"), value.quoted() + T(" is not a valid integer"));
      return Variable::missingValue(refCountedPointerFromThis(this));
    }
    int intValue = value.getIntValue();
    if (intValue < 0)
    {
      context.errorCallback(T("A positive integer cannot be negative"));
      return Variable::missingValue(refCountedPointerFromThis(this));
    }
    return Variable(intValue, refCountedPointerFromThis(this));
  }

  virtual VariableValue getMissingValue() const
  {
    return VariableValue((size_t)-1);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_INTEGER_H_

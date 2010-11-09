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
| Filename: BooleanType.h                  | Boolean type                    |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_BOOLEAN_H_
# define LBCPP_OBJECT_TYPE_BOOLEAN_H_

# include "BuiltinType.h"

namespace lbcpp
{

class BooleanType : public BuiltinType
{
public:
  BooleanType(const String& name, TypePtr baseType)
    : BuiltinType(name, baseType) {}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const
  {
    String v = value.trim().toLowerCase();
    if (v == T("true"))
      return VariableValue(true);
    else if (v == T("false"))
      return VariableValue(false);
    else
    {
      callback.errorMessage(T("BooleanType::createFromString"), T("Could not read boolean value ") + value.quoted());
      return getMissingValue();
    }
  }

  virtual VariableValue create() const
    {return VariableValue(false);}

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setBoolean(source.getBoolean());}

  virtual String toString(const VariableValue& value) const
    {jassert(!isMissingValue(value)); return value.getBoolean() ? T("True") : T("False");}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return (value1.getBoolean() ? 1 : 0) - (value2.getBoolean() ? 1 : 0);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_BOOLEAN_H_

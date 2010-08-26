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
| Filename: StringType.h                   | String type                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_STRING_H_
# define LBCPP_OBJECT_TYPE_STRING_H_

# include "BuiltinType.h"

namespace lbcpp
{

class StringType : public BuiltinType
{
public:
  StringType(const String& name, TypePtr baseType)
    : BuiltinType(name, baseType) {}
  StringType() : BuiltinType(T("String")) {}

  virtual VariableValue getMissingValue() const
    {return VariableValue();}

  virtual VariableValue create() const
    {return VariableValue(String::empty);}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const
  {
    String v = value.trim();
    return VariableValue(v.startsWithChar('"') ? v.unquoted() : v);
  }

  virtual void destroy(VariableValue& value) const
    {value.clearString();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setString(source.getString());}

  virtual String toString(const VariableValue& value) const
  {
    jassert(!isMissingValue(value));
    String str = value.getString();
    return str.containsAnyOf(T(" \t\r\n")) ? str.quoted() : str;
  }

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return value1.getString().compare(value2.getString());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_STRING_H_

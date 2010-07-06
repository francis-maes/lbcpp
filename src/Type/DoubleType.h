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
| Filename: DoubleType.h                   | Double type                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_DOUBLE_H_
# define LBCPP_OBJECT_TYPE_DOUBLE_H_

# include <lbcpp/Object/Variable.h>

namespace lbcpp
{

class DoubleType : public BuiltinType
{
public:
  DoubleType() : BuiltinType(T("Double")) {}

  virtual void destroy(VariableValue& value) const
    {}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setDouble(source.getDouble());}

  virtual String toString(const VariableValue& value) const
    {return String(value.getDouble());}

  virtual bool equals(const VariableValue& value1, const VariableValue& value2) const
    {return value1.getDouble() == value2.getDouble();}

  virtual size_t getNumSubVariables(const VariableValue& value) const
    {return 0;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_DOUBLE_H_

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
| Filename: BuiltinTypeClass.h             | BuiltinType classes             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CLASS_BUILTIN_H_
# define LBCPP_OBJECT_CLASS_BUILTIN_H_

# include <lbcpp/Object/Variable.h>

namespace lbcpp
{

class RawDataBuiltinTypeClass : public BuiltinTypeClass
{
public:
  RawDataBuiltinTypeClass(const String& name)
    : BuiltinTypeClass(name) {}

  virtual void destroy(VariableValue& value) const
    {value.clearRawData();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    jassert(false);
  }
  virtual String toString(const VariableValue& value) const
  {
    jassert(false); 
    return T("");
  }

  virtual bool equals(const VariableValue& value1, const VariableValue& value2) const
    {jassert(false); return false;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_BUILTIN_H_

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
| Filename: NilType.h                      | Nil Type                        |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 17:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_TYPE_NIL_H_
# define LBCPP_DATA_TYPE_NIL_H_

# include <lbcpp/Core/Variable.h>

namespace lbcpp
{

class NilType : public Type
{
public:
  NilType(const String& name, TypePtr baseType)
    : Type(name, baseType) {}
  virtual ~NilType() {}

  virtual VariableValue create(ExecutionContext& context) const
    {return VariableValue();}

  virtual VariableValue createFromXml(XmlImporter& importer) const
    {return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest = VariableValue();}

  virtual String toString(const VariableValue& value) const
    {return T("Nil");}

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
    {}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return 0;}

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TYPE_NIL_H_

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
| Filename: BuiltinType.h                  | BuiltinType classes             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_BUILTIN_H_
# define LBCPP_OBJECT_TYPE_BUILTIN_H_

# include <lbcpp/Data/Variable.h>
# include <lbcpp/Data/XmlSerialisation.h>

namespace lbcpp
{

class BuiltinType : public Type
{
public:
  BuiltinType(const String& name, TypePtr baseType = topLevelType)
    : Type(name, baseType) {}
  BuiltinType(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType)
    : Type(templateType, templateArguments, baseType) {}

  virtual VariableValue createFromXml(XmlImporter& importer) const
    {return createFromString(importer.getAllSubText(), importer.getCallback());}

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
    {exporter.addTextElement(toString(value));}

  virtual size_t getNumElements(const VariableValue& value) const
    {return 0;}

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_BUILTIN_H_

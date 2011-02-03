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
| Filename: TopLevelType.h                 | Top Level Type                  |
| Author  : Francis Maes                   |                                 |
| Started : 07/07/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_TYPE_TOP_LEVEL_H_
# define LBCPP_DATA_TYPE_TOP_LEVEL_H_

# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class TopLevelType : public Type
{
public:
  TopLevelType(const String& name)
    : Type(name, TypePtr()) {}

  virtual VariableValue create(ExecutionContext& context) const
    {context.errorCallback(T("Type::create"), getName() + T(" has no default constructor")); return VariableValue();}

  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const
    {context.errorCallback(T("Type::createFromString"), T("Not implemented")); return VariableValue();}

  virtual VariableValue createFromXml(XmlImporter& importer) const
    {importer.errorMessage(T("Type::createFromXml"), T("Not implemented")); return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {jassert(false);}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {jassert(false);}

  virtual String toString(const VariableValue& value) const
    {jassert(false); return String::empty;}

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
    {exporter.getContext().errorCallback(T("Type::saveToXml()"), T("Not implemented"));}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {jassert(false); return 0;}

  virtual size_t getNumMemberVariables() const
    {return 0;}

  virtual TypePtr getMemberVariableType(size_t index) const
    {jassert(false); return TypePtr();}

  virtual String getMemberVariableName(size_t index) const
    {jassert(false); return String::empty;}

  virtual String getMemberVariableShortName(size_t index) const
    {jassert(false); return String::empty;}

  virtual String getMemberVariableDescription(size_t index) const
    {jassert(false); return String::empty;}

  virtual int findMemberVariable(const String& name) const
    {return -1;}

  virtual Variable getMemberVariableValue(const VariableValue& value, size_t index) const
    {jassert(false); return Variable();}

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TYPE_TOP_LEVEL_H_

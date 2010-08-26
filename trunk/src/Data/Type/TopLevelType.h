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

# include <lbcpp/Data/Variable.h>

namespace lbcpp
{

class TopLevelType : public Type
{
public:
  TopLevelType(const String& name)
    : Type(name, TypePtr()) {}

  virtual VariableValue create() const
    {MessageCallback::error(T("Type::create"), getName() + T(" has no default constructor")); return VariableValue();}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const
    {callback.errorMessage(T("Type::createFromString"), T("Not implemented")); return VariableValue();}

  virtual VariableValue createFromXml(XmlElement* xml, MessageCallback& callback) const
    {callback.errorMessage(T("Type::createFromXml"), T("Not implemented")); return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {MessageCallback::error(T("Type::destroy()"), T("Not implemented"));}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {MessageCallback::error(T("Type::copy()"), T("Not implemented"));}

  virtual String toString(const VariableValue& value) const
    {MessageCallback::error(T("Type::toString()"), T("Not implemented")); return String::empty;}

  virtual void saveToXml(XmlElement* xml, const VariableValue& value) const
    {MessageCallback::error(T("Type::saveToXml()"), T("Not implemented"));}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {MessageCallback::error(T("Type::compare()"), T("Not implemented")); return 0;}

  virtual size_t getObjectNumVariables() const
    {return 0;}

  virtual TypePtr getObjectVariableType(size_t index) const
    {MessageCallback::error(T("Type::getObjectVariableType()"), T("Not implemented")); return TypePtr();}

  virtual String getObjectVariableName(size_t index) const
    {MessageCallback::error(T("Type::getObjectVariableName()"), T("Not implemented")); return String::empty;}

  virtual int findObjectVariable(const String& name) const
    {return -1;}

  virtual Variable getObjectVariable(const VariableValue& value, size_t index) const
    {MessageCallback::error(T("Type::getObjectVariable()"), T("Not implemented")); return Variable();}

  virtual size_t getNumElements(const VariableValue& value) const
    {return 0;}

  virtual Variable getElement(const VariableValue& value, size_t index) const
    {MessageCallback::error(T("Type::getElement()"), T("Not implemented")); return Variable();}

  virtual String getElementName(const VariableValue& value, size_t index) const
    {MessageCallback::error(T("Type::getElementName()"), T("Not implemented")); return String::empty;}

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TYPE_TOP_LEVEL_H_

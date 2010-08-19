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

#ifndef LBCPP_OBJECT_TYPE_INTEGER_H_
# define LBCPP_OBJECT_TYPE_INTEGER_H_

# include <lbcpp/Data/Variable.h>

namespace lbcpp
{

class TopLevelType : public Type
{
public:
  TopLevelType()
    : Type(T("Variable"), TypePtr()) {}

  virtual VariableValue create() const
    {jassert(false); return VariableValue();}

  virtual VariableValue createFromString(const String& value, ErrorHandler& callback) const
    {callback.errorMessage(T("Type::createFromString"), T("Not implemented")); return VariableValue();}

  virtual VariableValue createFromXml(XmlElement* xml, ErrorHandler& callback) const
    {callback.errorMessage(T("Type::createFromXml"), T("Not implemented")); return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {jassert(false);}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {jassert(false);}

  virtual String toString(const VariableValue& value) const
  {
    ErrorHandler::error(T("Type::toString(value)"), T("Not implemented"));
    jassert(false);
    return String::empty;
  }

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {jassert(false); return false;}
};

class NilType : public Type
{
public:
  NilType() : Type(T("Nil"), topLevelType()) {}

  virtual VariableValue create() const
    {return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest = VariableValue();}

  virtual String toString(const VariableValue& value) const
    {return T("Nil");}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return 0;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_INTEGER_H_

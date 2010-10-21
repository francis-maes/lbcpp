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

# include "BuiltinType.h"

namespace lbcpp
{

class DoubleType : public BuiltinType
{
public:
  DoubleType() : BuiltinType(T("Double")) {}
  DoubleType(const String& name, TypePtr baseType)
    : BuiltinType(name, baseType) {}

  virtual VariableValue create() const
    {return VariableValue(0.0);}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const
  {
    String v = value.trim().toLowerCase();
    if (!v.containsOnly(T("0123456789e.-")))
    {
      callback.errorMessage(T("DoubleType::createFromString"), T("Could not read double value ") + value.quoted());
      return getMissingValue();
    }
    return VariableValue(v.getDoubleValue());
  }

  virtual VariableValue createFromXml(XmlImporter& importer) const
    {return VariableValue(importer.getAllSubText().getDoubleValue());}

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
    {exporter.addTextElement(String(value.getDouble(), 8));}

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setDouble(source.getDouble());}

  virtual String toString(const VariableValue& value) const
    {jassert(!isMissingValue(value)); return String(value.getDouble());}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
  {
    double v1 = value1.getDouble();
    double v2 = value2.getDouble();
    return v1 < v2 ? -1 : (v1 > v2 ? 1 : 0);
  }
};

class ProbabilityType : public DoubleType
{
public:
  ProbabilityType(const String& name, TypePtr baseType)
    : DoubleType(name, baseType) {}

  virtual String toString(const VariableValue& value) const
    {return String(value.getDouble() * 100, 2) + T("%");}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const
  {
    String str = value.trim();
    if (str.endsWithChar('%'))
      str.dropLastCharacters(1);
    return str.getDoubleValue() / 100.0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_DOUBLE_H_

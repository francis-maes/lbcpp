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

  virtual Variable create(ExecutionContext& context) const
    {return Variable(0.0, refCountedPointerFromThis(this));}

  virtual Variable createFromString(ExecutionContext& context, const String& value) const
  {
    String v = value.trim().toLowerCase();
    if (!v.containsOnly(T("0123456789e.-")))
    {
      context.errorCallback(T("DoubleType::createFromString"), T("Could not read double value ") + value.quoted());
      return Variable::missingValue(refCountedPointerFromThis(this));
    }
    return Variable(v.getDoubleValue(), refCountedPointerFromThis(this));
  }

  virtual Variable createFromXml(XmlImporter& importer) const
    {return Variable(importer.getAllSubText().getDoubleValue(), refCountedPointerFromThis(this));}

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
    {exporter.addTextElement(String(value.getDouble(), 8));}

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setDouble(source.getDouble());}

  virtual String toString(const VariableValue& value) const
    {jassert(!isMissingValue(value)); return String(value.getDouble(), 4);}

  static String positiveNumberToShortString(double d)
  {
    int l1 = (int)log10(d);
    int l3 = (l1 / 3) * 3;
    
    double Z = pow(10.0, (double)l3);
    String res = String(d / Z, 3 - abs(l1 - l3));
    int numZeros = 0;
    int pos = res.length() - 1;
    while (pos > 0 && res[pos] == '0')
      --pos, ++numZeros;
    if (pos > 0 && res[pos] == '.')
      ++numZeros;
    if (numZeros)
      res = res.dropLastCharacters(numZeros);
    if (l3 != 0)
    {
      res += T("e");
      res += (l3 > 0 ? T("+") : T("-"));
      res += String((int)abs(l3));
    }
    return res;
  }

  virtual String toShortString(const VariableValue& value) const
  {
    jassert(!isMissingValue(value));
    double d = value.getDouble();
    if (!d)
      return T("0");
    String res = positiveNumberToShortString(fabs(d));
    return d < 0.0 ? T("-") + res : res;
  }

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

  virtual Variable createFromString(ExecutionContext& context, const String& value) const
  {
    String str = value.trim();
    if (str.endsWithChar('%'))
      str.dropLastCharacters(1);
    return Variable(str.getDoubleValue() / 100.0, refCountedPointerFromThis(this));
  }
};

class TimeType : public DoubleType
{
public:
  TimeType(const String& name, TypePtr baseType)
    : DoubleType(name, baseType) {}

  virtual String toShortString(const VariableValue& value) const
  {
    jassert(!isMissingValue(value));
    double timeInSeconds = value.getDouble();
    if (timeInSeconds == 0.0)
      return T("0 s");

    String sign;
    if (timeInSeconds > 0)
      sign = String::empty;
    else
    {
      timeInSeconds = -timeInSeconds;
      sign = T("-");
    }

    if (timeInSeconds < 1e-5)
      return sign + String((int)(timeInSeconds / 1e-9)) + T(" nanos");
    if (timeInSeconds < 1e-2)
      return sign + String((int)(timeInSeconds / 1e-6)) + T(" micros");

    int numSeconds = (int)timeInSeconds;
    if (timeInSeconds < 10)
      return sign + (numSeconds ? String(numSeconds) + T(" s ") : String::empty) + String((int)(timeInSeconds * 1000) % 1000) + T(" ms");

    String res = sign;
    if (numSeconds > 3600)
    {
      int numHours = numSeconds / 3600;
      if (numHours > 24)
      {
        int numDays = numHours / 24;
        res += numDays == 1 ? T("1 day") : String(numDays) + T(" days");
      }
      if (res.isNotEmpty())
        res += T(" ");
      res += String(numHours % 24) + T(" hours");
    }
    if (numSeconds >= 60)
    {
      if (res.isNotEmpty())
        res += T(" ");
      res += String((numSeconds / 60) % 60) + T(" min");
    }
    if (res.isNotEmpty())
      res += T(" ");
    res += String(numSeconds % 60) + T(" s");
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_DOUBLE_H_

/*-----------------------------------------.---------------------------------.
| Filename: Double.cpp                     | Double                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Double.h>
#include <lbcpp/Core/DefaultClass.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** Double
*/
DoublePtr Double::create(ClassPtr type, double value)
{
  if (type == probabilityClass)
    return new Probability(type, value);
  else if (type == timeClass)
    return new Time(type, value);
  else
    return new Double(type, value);
}

static String positiveNumberToShortString(double d)
{
  int l1 = (int)log10(d);
  int l3 = (int)(floor(l1 / 3.0) * 3.0);
    
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

String Double::toShortString() const
{
  if (!value)
    return T("0");
  String res = positiveNumberToShortString(fabs(value));
  return value < 0.0 ? T("-") + res : res;
}

String Double::toString() const
  {return String(value);}
  
double Double::toDouble() const
  {return value;}

int Double::compare(const ObjectPtr& otherObject) const
{
  const DoublePtr& other = otherObject.staticCast<Double>();
  double delta = value - other->get();
  return delta < 0.0 ? -1 : (delta > 0.0 ? 1 : 0);
}

void Double::clone(ExecutionContext& context, const ObjectPtr& target) const
  {target.staticCast<Double>()->value = value;}
  
bool Double::loadFromString(ExecutionContext& context, const String& str)
{
  String v = str.trim().toLowerCase();
  if (!v.containsOnly(T("0123456789e.-")))
  {
    context.errorCallback(T("Double::loadFromString"), T("Could not read double value ") + str.quoted());
    return false;
  }
  value = v.getDoubleValue();
  return true;
}

bool Double::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void Double::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

/*
** Probability
*/
String Probability::toShortString() const
  {return String(value * 100, 1) + T("%");}

bool Probability::toBoolean() const
  {return value > 0.5;}

/*
** Time
*/
String Time::toShortString() const
{
  double timeInSeconds = value;
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

/*-----------------------------------------.---------------------------------.
| Filename: Double.cpp                     | Double                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/11/2012 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core/Double.h>
#include <oil/Core/DefaultClass.h>
#include <oil/Core/Vector.h>
#include <oil/Core/XmlSerialisation.h>
#include <oil/Execution/ExecutionContext.h>
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

static string positiveNumberToShortString(double d)
{
  int l1 = (int)log10(d);
  int l3 = (int)(floor(l1 / 3.0) * 3.0);
    
  double Z = pow(10.0, (double)l3);
  string res = string(d / Z, 3 - abs(l1 - l3));
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
    res += JUCE_T("e");
    res += (l3 > 0 ? JUCE_T("+") : JUCE_T("-"));
    res += string((int)abs(l3));
  }
  return res;
}

string Double::toShortString() const
{
  if (!value)
    return JUCE_T("0");
  string res = positiveNumberToShortString(fabs(value));
  return value < 0.0 ? JUCE_T("-") + res : res;
}

string Double::toString() const
  {return string(value);}
  
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
  
bool Double::loadFromString(ExecutionContext& context, const string& str)
{
  string v = str.trim().toLowerCase();
  if (v == JUCE_T("nan"))
  {
    value = DVector::missingValue;
    return true;
  }
  if (!v.containsOnly(JUCE_T("0123456789e.-")))
  {
    context.errorCallback(JUCE_T("Double::loadFromString"), JUCE_T("Could not read double value ") + str.quoted());
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
string Probability::toShortString() const
  {return string(value * 100, 1) + JUCE_T("%");}

bool Probability::toBoolean() const
  {return value > 0.5;}

/*
** Time
*/
string Time::toShortString() const
{
  double timeInSeconds = value;
  if (timeInSeconds == 0.0)
    return JUCE_T("0 s");

  string sign;
  if (timeInSeconds > 0)
    sign = string::empty;
  else
  {
    timeInSeconds = -timeInSeconds;
    sign = JUCE_T("-");
  }

  if (timeInSeconds < 1e-5)
    return sign + string((int)(timeInSeconds / 1e-9)) + JUCE_T(" nanos");
  if (timeInSeconds < 1e-2)
    return sign + string((int)(timeInSeconds / 1e-6)) + JUCE_T(" micros");

  int numSeconds = (int)timeInSeconds;
  if (timeInSeconds < 10)
    return sign + (numSeconds ? string(numSeconds) + JUCE_T(" s ") : string::empty) + string((int)(timeInSeconds * 1000) % 1000) + JUCE_T(" ms");

  string res = sign;
  if (numSeconds > 3600)
  {
    int numHours = numSeconds / 3600;
    if (numHours > 24)
    {
      int numDays = numHours / 24;
      res += numDays == 1 ? JUCE_T("1 day") : string(numDays) + JUCE_T(" days");
    }
    if (res.isNotEmpty())
      res += JUCE_T(" ");
    res += string(numHours % 24) + JUCE_T(" hours");
  }
  if (numSeconds >= 60)
  {
    if (res.isNotEmpty())
      res += JUCE_T(" ");
    res += string((numSeconds / 60) % 60) + JUCE_T(" min");
  }
  if (res.isNotEmpty())
    res += JUCE_T(" ");
  res += string(numSeconds % 60) + JUCE_T(" s");
  return res;
}

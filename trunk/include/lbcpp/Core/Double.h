/*-----------------------------------------.---------------------------------.
| Filename: Double.h                       | Double                          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_DOUBLE_H_
# define LBCPP_CORE_DOUBLE_H_

# include "Object.h"
# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class NewDouble : public Object
{
public:
  NewDouble(ClassPtr thisClass, double value = 0.0)
    : Object(thisClass), value(value) {}
  NewDouble(double value = 0.0)
    : value(value) {}

  static NewDoublePtr create(TypePtr type, double value);
  static double get(ObjectPtr object)
    {return object.staticCast<NewDouble>()->get();}

  void set(double value)
    {this->value = value;}

  double get() const
    {return value;}
  
  virtual String toShortString() const
  {
    if (!value)
      return T("0");
    String res = positiveNumberToShortString(fabs(value));
    return value < 0.0 ? T("-") + res : res;
  }

  virtual String toString() const
    {return String(value);}
  
  virtual double toDouble() const
    {return value;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const NewDoublePtr& other = otherObject.staticCast<NewDouble>();
    double delta = value - other->get();
    return delta < 0.0 ? -1 : (delta > 0.0 ? 1 : 0);
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new NewDouble(value);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<NewDouble>()->value = value;}
  
  virtual bool loadFromString(ExecutionContext& context, const String& str)
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

protected:
  double value;

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
};

extern ClassPtr newDoubleClass;

class NewProbability : public NewDouble
{
public:
  NewProbability(ClassPtr thisClass, double value = 0.0)
    : NewDouble(thisClass, value) {}
  NewProbability(double value = 0.0)
    : NewDouble(value) {}

  virtual String toShortString() const
    {return String(value * 100, 1) + T("%");}
};

extern ClassPtr newProbabilityClass;

class NewTime : public NewDouble
{
public:
  NewTime(ClassPtr thisClass, double value = 0.0)
    : NewDouble(thisClass, value) {}
  NewTime(double value = 0.0)
    : NewDouble(value) {}

  virtual String toShortString() const
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
};

extern ClassPtr newTimeClass;


inline NewDoublePtr NewDouble::create(TypePtr type, double value)
{
  if (type == newProbabilityClass)
    return new NewProbability(type, value);
  else if (type == newTimeClass)
    return new NewTime(type, value);
  else
    return new NewDouble(type, value);
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

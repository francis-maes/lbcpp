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

  void set(double value)
    {this->value = value;}

  double get() const
    {return value;}
  
  static double get(ObjectPtr object)
    {return object.staticCast<NewDouble>()->get();}

  virtual String toShortString() const
  {
    if (!value)
      return T("0");
    String res = positiveNumberToShortString(fabs(value));
    return value < 0.0 ? T("-") + res : res;
  }

  virtual String toString() const
    {return String(value);}

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
      context.errorCallback(T("Double::createFromString"), T("Could not read double value ") + str.quoted());
      return false;
    }
    value = v.getDoubleValue();
    return true;
  }

private:
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

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

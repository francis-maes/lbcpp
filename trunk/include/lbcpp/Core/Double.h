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

namespace lbcpp
{

class Double : public Object
{
public:
  Double(ClassPtr thisClass, double value = 0.0)
    : Object(thisClass), value(value) {}
  Double(double value = 0.0)
    : value(value) {}

  static DoublePtr create(ClassPtr type, double value);
  static double get(ObjectPtr object)
    {return object.staticCast<Double>()->get();}

  void set(double value)
    {this->value = value;}

  double get() const
    {return value;}
  
  virtual string toShortString() const;

  virtual double toDouble() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  virtual bool loadFromString(ExecutionContext& context, const string& str);
  virtual string toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  double value;
};

extern ClassPtr doubleClass;

class Probability : public Double
{
public:
  Probability(ClassPtr thisClass, double value = 0.0)
    : Double(thisClass, value) {}
  Probability(double value = 0.0)
    : Double(value) {}

  virtual string toShortString() const;
  virtual bool toBoolean() const;
};

extern ClassPtr probabilityClass;

class Time : public Double
{
public:
  Time(ClassPtr thisClass, double value = 0.0)
    : Double(thisClass, value) {}
  Time(double value = 0.0)
    : Double(value) {}

  virtual string toShortString() const;

  static double getHighResolutionCounter()
    {return juce::Time::getMillisecondCounterHiRes() / 1000.0;}
};

extern ClassPtr timeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

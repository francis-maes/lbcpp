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

class NewDouble : public Object
{
public:
  NewDouble(ClassPtr thisClass, double value = 0.0)
    : Object(thisClass), value(value) {}
  NewDouble(double value = 0.0)
    : value(value) {}

  static NewDoublePtr create(ClassPtr type, double value);
  static double get(ObjectPtr object)
    {return object.staticCast<NewDouble>()->get();}

  void set(double value)
    {this->value = value;}

  double get() const
    {return value;}
  
  virtual String toShortString() const;

  virtual double toDouble() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  virtual String toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  double value;
};

extern ClassPtr newDoubleClass;

class NewProbability : public NewDouble
{
public:
  NewProbability(ClassPtr thisClass, double value = 0.0)
    : NewDouble(thisClass, value) {}
  NewProbability(double value = 0.0)
    : NewDouble(value) {}

  virtual String toShortString() const;
  virtual bool toBoolean() const;
};

extern ClassPtr newProbabilityClass;

class NewTime : public NewDouble
{
public:
  NewTime(ClassPtr thisClass, double value = 0.0)
    : NewDouble(thisClass, value) {}
  NewTime(double value = 0.0)
    : NewDouble(value) {}

  virtual String toShortString() const;
};

extern ClassPtr newTimeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

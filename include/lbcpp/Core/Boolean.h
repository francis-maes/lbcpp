/*-----------------------------------------.---------------------------------.
| Filename: Boolean.h                      | Boolean                         |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_BOOLEAN_H_
# define LBCPP_CORE_BOOLEAN_H_

# include "Object.h"

namespace lbcpp
{

class NewBoolean : public Object
{
public:
  NewBoolean(ClassPtr thisClass, bool value = false)
    : Object(thisClass), value(value) {}
  NewBoolean(bool value = false)
    : value(value) {}

  void set(bool value)
    {this->value = value;}

  bool get() const
    {return value;}

  static bool get(ObjectPtr object)
    {return object.staticCast<NewBoolean>()->get();}

  virtual String toShortString() const;
  
  virtual double toDouble() const;
  virtual bool toBoolean() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual bool loadFromString(ExecutionContext& context, const String& str);
  virtual String toString() const;
  
  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  bool value;
};

extern ClassPtr newBooleanClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_BOOLEAN_H_

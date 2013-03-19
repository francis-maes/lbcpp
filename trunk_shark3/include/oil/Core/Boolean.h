/*-----------------------------------------.---------------------------------.
| Filename: Boolean.h                      | Boolean                         |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_CORE_BOOLEAN_H_
# define OIL_CORE_BOOLEAN_H_

# include "Object.h"

namespace lbcpp
{

class Boolean : public Object
{
public:
  Boolean(ClassPtr thisClass, bool value = false)
    : Object(thisClass), value(value) {}
  Boolean(bool value = false)
    : value(value) {}

  void set(bool value)
    {this->value = value;}

  bool get() const
    {return value;}

  static bool get(ObjectPtr object)
    {return object.staticCast<Boolean>()->get();}

  virtual string toShortString() const;
  
  virtual double toDouble() const;
  virtual bool toBoolean() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual bool loadFromString(ExecutionContext& context, const string& str);
  virtual string toString() const;
  
  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  bool value;
};

extern ClassPtr booleanClass;

}; /* namespace lbcpp */

#endif // !OIL_CORE_BOOLEAN_H_

/*-----------------------------------------.---------------------------------.
| Filename: Integer.h                      | Integer                         |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_INTEGER_H_
# define LBCPP_CORE_INTEGER_H_

# include "Object.h"
# include "Enumeration.h"

namespace lbcpp
{

class NewInteger : public Object
{
public:
  NewInteger(ClassPtr thisClass, juce::int64 value = 0)
    : Object(thisClass), value(value) {}
  NewInteger(juce::int64 value = 0)
    : value(value) {}

  static NewIntegerPtr create(ClassPtr type, juce::int64 value);
  
  static juce::int64 get(ObjectPtr object)
    {return object.staticCast<NewInteger>()->get();}

  void set(juce::int64 value)
    {this->value = value;}

  juce::int64 get() const
    {return value;}
  
  virtual String toShortString() const;

  virtual double toDouble() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual bool loadFromString(ExecutionContext& context, const String& value);
  virtual String toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  juce::int64 value;
};

extern ClassPtr newIntegerClass;

class NewPositiveInteger : public NewInteger
{
public:
  NewPositiveInteger(ClassPtr thisClass, size_t value = 0)
    : NewInteger(thisClass, value) {}
  NewPositiveInteger(size_t value = 0)
    : NewInteger(value) {}

  static size_t get(ObjectPtr object)
    {return object.staticCast<NewPositiveInteger>()->get();}

  void set(size_t value)
    {this->value = (juce::int64)value;}

  size_t get() const
    {return (size_t)value;}
};

extern ClassPtr newPositiveIntegerClass;

class NewEnumValue : public NewPositiveInteger
{
public:
  NewEnumValue(EnumerationPtr enumeration, size_t value = 0)
   : NewPositiveInteger(enumeration, value) {}
  NewEnumValue() {}

  EnumerationPtr getEnumeration() const
    {return getClass().staticCast<Enumeration>();}

  EnumerationElementPtr getEnumerationElement() const
    {return getEnumeration()->getElement((int)value);}

  virtual String toShortString() const;

  virtual bool loadFromString(ExecutionContext& context, const String& value);
  virtual String toString() const;
};

extern ClassPtr newEnumValueClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

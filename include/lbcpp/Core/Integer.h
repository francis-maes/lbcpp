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

class Integer : public Object
{
public:
  Integer(ClassPtr thisClass, juce::int64 value = 0)
    : Object(thisClass), value(value) {}
  Integer(juce::int64 value = 0)
    : value(value) {}

  static IntegerPtr create(ClassPtr type, juce::int64 value);
  
  static juce::int64 get(ObjectPtr object)
    {return object.staticCast<Integer>()->get();}

  void set(juce::int64 value)
    {this->value = value;}

  juce::int64 get() const
    {return value;}
  
  virtual string toShortString() const;

  virtual double toDouble() const;
  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual bool loadFromString(ExecutionContext& context, const string& value);
  virtual string toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  juce::int64 value;
};

extern ClassPtr integerClass;

class PositiveInteger : public Integer
{
public:
  PositiveInteger(ClassPtr thisClass, size_t value = 0)
    : Integer(thisClass, value) {}
  PositiveInteger(size_t value = 0)
    : Integer(value) {}

  static size_t get(ObjectPtr object)
    {return object.staticCast<PositiveInteger>()->get();}

  void set(size_t value)
    {this->value = (juce::int64)value;}

  size_t get() const
    {return (size_t)value;}
};

extern ClassPtr positiveIntegerClass;
extern ClassPtr memorySizeClass;

class MemorySize : public PositiveInteger
{
public:
  MemorySize(ClassPtr thisClass, size_t value = 0)
    : PositiveInteger(thisClass, value) {}
  MemorySize(size_t value = 0)
    : PositiveInteger(memorySizeClass, value) {}

  virtual string toShortString() const;
};

class EnumValue : public PositiveInteger
{
public:
  EnumValue(EnumerationPtr enumeration, size_t value = 0)
   : PositiveInteger(enumeration, value) {}
  EnumValue() {}

  EnumerationPtr getEnumeration() const
    {return getClass().staticCast<Enumeration>();}

  EnumerationElementPtr getEnumerationElement() const
    {return getEnumeration()->getElement((int)value);}

  virtual string toShortString() const;

  virtual bool loadFromString(ExecutionContext& context, const string& value);
  virtual string toString() const;
};

extern ClassPtr enumValueClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

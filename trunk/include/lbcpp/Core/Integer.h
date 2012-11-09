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

  void set(juce::int64 value)
    {this->value = value;}

  juce::int64 get() const
    {return value;}
  
  static juce::int64 get(ObjectPtr object)
    {return object.staticCast<NewInteger>()->get();}

  virtual String toShortString() const
    {return String(value);}

  virtual String toString() const
    {return String(value);}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const NewIntegerPtr& other = otherObject.staticCast<NewInteger>();
    return (int)(value - other->get());
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new NewInteger(value);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<NewInteger>()->value = value;}

protected:
  juce::int64 value;
};

extern ClassPtr newIntegerClass;

class NewEnumValue : public NewInteger
{
public:
  NewEnumValue(EnumerationPtr enumeration, juce::int64 value = 0)
   : NewInteger(enumeration, value) {}
  NewEnumValue() {}

  EnumerationPtr getEnumeration() const
    {return getClass().staticCast<Enumeration>();}

  EnumerationElementPtr getEnumerationElement() const
    {return getEnumeration()->getElement((int)value);}

  virtual String toShortString() const
  {
    EnumerationElementPtr element = getEnumerationElement();
    return element->getShortName().isNotEmpty() ? element->getShortName() : element->getName();
  }

  virtual String toString() const
    {return getEnumerationElement()->getName();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

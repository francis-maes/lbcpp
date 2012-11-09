/*-----------------------------------------.---------------------------------.
| Filename: String.h                       | String                          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 17:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_STRING_H_
# define LBCPP_CORE_STRING_H_

# include "Object.h"

namespace lbcpp
{

class NewString : public Object
{
public:
  NewString(const juce::String& value = juce::String::empty)
    : value(value) {}

  void set(const juce::String& value)
    {this->value = value;}

  const juce::String& get() const
    {return value;}
  
  static juce::String get(ObjectPtr object)
    {return object.staticCast<NewString>()->get();}

  virtual juce::String toShortString() const
    {return value;}

  virtual String toString() const
    {return value.quoted();}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const NewStringPtr& other = otherObject.staticCast<NewString>();
    if (value < other->get())
      return -1;
    else if (value > other->get())
      return 1;
    else
      return 0;
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new NewString(value);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<NewString>()->value = value;}

private:
  juce::String value;
};

extern ClassPtr newStringClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

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
  NewBoolean(bool value = false)
    : value(value) {}

  void set(bool value)
    {this->value = value;}

  bool get() const
    {return value;}

  virtual String toShortString() const
    {return value ? "true" : "false";}

  virtual String toString() const
    {return value ? "true" : "false";}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const NewBooleanPtr& other = otherObject.staticCast<NewBoolean>();
    return (value ? 1 : 0) - (other->value ? 1 : 0);
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new NewBoolean(value);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<NewBoolean>()->value = value;}

private:
  bool value;
};

extern ClassPtr newBooleanClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_BOOLEAN_H_

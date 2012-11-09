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
  NewDouble(double value = 0.0)
    : value(value) {}

  void set(double value)
    {this->value = value;}

  double get() const
    {return value;}

  virtual String toShortString() const
    {return String(value, 2);}

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

private:
  double value;
};

extern ClassPtr newDoubleClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_

// OnlineFQI
#ifndef GLOB_XOR_PERCEPTION_H_
# define GLOB_XOR_PERCEPTION_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class XorExamplePerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairClass(doubleType, doubleType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("unit"), doubleType);
    addOutputVariable(T("x1"), doubleType);
    addOutputVariable(T("x2"), doubleType);
    addOutputVariable(T("x1.x2"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    double x1 = input[0].getDouble();
    double x2 = input[1].getDouble();
    callback->sense(0, 1.0);
    callback->sense(1, x1);
    callback->sense(2, x2);
    callback->sense(3, x1 * x2);
  }
};

}; /* namespace lbcpp */

#endif // !GLOB_XOR_PERCEPTION_H_

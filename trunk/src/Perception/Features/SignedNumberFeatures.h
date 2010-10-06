/*------------------------------------------.---------------------------------.
 | Filename: SignedNumberFeatures.h         | A Feature Generator for         |
 | Author  : Francis Maes                   | signed numbers                  |
 | Started : 04/10/2010 12:33               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class SignedNumberFeatures : public Perception
{
public:
  SignedNumberFeatures(PerceptionPtr positiveNumberPerception)
    : positiveNumberPerception(positiveNumberPerception)
    {computeOutputType();}

  SignedNumberFeatures() {}

  virtual TypePtr getInputType() const
    {return positiveNumberPerception->getInputType();}

  virtual bool isSparse() const
    {return true;}

  virtual void computeOutputType()
  {
    reserveOutputVariables(3);
    addOutputVariable(T("zero"), doubleType());
    addOutputVariable(T("positive"), positiveNumberPerception);
    addOutputVariable(T("negative"), positiveNumberPerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input);
    double value = input.isDouble() ? input.getDouble() : (double)input.getInteger();
    const double epsilon = 1e-15;
    if (fabs(value) < epsilon)
      callback->sense(0, 1.0);
    else if (value > 0)
      callback->sense(1, positiveNumberPerception, input);
    else
    {
      Variable opposite(input.isInteger() ? Variable(-(int)value, input.getType()) : Variable(-value, input.getType()));
      callback->sense(2, positiveNumberPerception, opposite);
    }
  }

protected:
  friend class SignedNumberFeaturesClass;

  PerceptionPtr positiveNumberPerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_

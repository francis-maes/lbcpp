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
  {
  }

  SignedNumberFeatures() {}

  virtual TypePtr getInputType() const
    {return positiveNumberPerception->getInputType();}

  virtual bool isSparse() const
    {return true;}

  virtual size_t getNumOutputVariables() const
    {return 3;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return index == 0 ? doubleType() : positiveNumberPerception->getOutputType();}

  virtual String getOutputVariableName(size_t index) const
    {return index == 0 ? T("zero") : (index == 1 ? T("positive") : T("negative"));}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return index == 0 ? PerceptionPtr() : positiveNumberPerception;}

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

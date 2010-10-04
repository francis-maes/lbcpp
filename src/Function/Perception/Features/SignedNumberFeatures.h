/*------------------------------------------.---------------------------------.
 | Filename: SignedNumberFeatures.h         | A Feature Generator for         |
 | Author  : Francis Maes                   | signed numbers                  |
 | Started : 04/10/2010 12:33               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
  
class SignedNumberFeatures : public CompositePerception
{
public:
  SignedNumberFeatures(PerceptionPtr positiveNumberPerception)
  {
    addPerception(T("negative"), positiveNumberPerception);
    addPerception(T("positive"), positiveNumberPerception);
  }

  SignedNumberFeatures() {}
  
  virtual TypePtr getInputType() const
    {return getPerception(0)->getInputType();}

  virtual size_t getNumOutputVariables() const
    {return 1 + CompositePerception::getNumOutputVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return index == 0 ? doubleType() : CompositePerception::getOutputVariableType(index - 1);}

  virtual String getOutputVariableName(size_t index) const
    {return index == 0 ? T("zero") : CompositePerception::getOutputVariableName(index - 1);}

  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return index == 0 ? PerceptionPtr() : CompositePerception::getOutputVariableGenerator(index - 1);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input);
    double value = input.isDouble() ? input.getDouble() : (double)input.getInteger();
    const double epsilon = 1e-15;
    if (fabs(value) < epsilon)
      callback->sense(0, 1.0);
    else if (value < 0)
    {
      Variable opposite(input.isInteger() ? Variable(-(int)value, input.getType()) : Variable(-value, input.getType()));
      callback->sense(1, getPerception(0), opposite);
    }
    else
      callback->sense(2, getPerception(1), input);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_SIGNED_NUMBER_H_

/*-----------------------------------------.---------------------------------.
| Filename: BooleanFeatures.h              | A Feature Generator that        |
| Author  : Francis Maes                   | converts a boolean to a feature |
| Started : 04/10/2010 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_BOOLEAN_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_BOOLEAN_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class BooleanFeatures : public Perception
{
public:
  virtual String toString() const
    {return T("boolean as feature");}

  virtual TypePtr getInputType() const
    {return booleanType();}

  virtual size_t getNumOutputVariables() const
    {return 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("value");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isBoolean());
    if (input && input.getBoolean())
      callback->sense(0, 1.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_BOOLEAN_H_

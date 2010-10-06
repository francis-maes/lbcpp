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
  BooleanFeatures()
    {computeOutputType();}

  virtual String toString() const
    {return T("boolean as feature");}

  virtual TypePtr getInputType() const
    {return booleanType();}

  virtual void computeOutputType()
  {
    addOutputVariable(T("value"), doubleType());
    Perception::computeOutputType();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isBoolean());
    if (input && input.getBoolean())
      callback->sense(0, 1.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_BOOLEAN_H_

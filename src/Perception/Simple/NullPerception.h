/*-----------------------------------------.---------------------------------.
| Filename: NullPerception.h               | A Perception that returns Nil   |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_NULL_H_
# define LBCPP_FUNCTION_PERCEPTION_NULL_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class NullPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return anyType();}

  virtual TypePtr getOutputType() const
    {return nilType();}

  virtual void computeOutputVariables()
    {}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {}

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_NULL_H_

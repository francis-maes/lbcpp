/*-----------------------------------------.---------------------------------.
| Filename: NullPerception.h               | A Perception that returns Nil   |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_NULL_H_
# define LBCPP_FUNCTION_PERCEPTION_NULL_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class NullPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return anyType();}

  virtual TypePtr getOutputType() const
    {return nilType();}

  virtual size_t getNumOutputVariables() const
    {return 0;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {jassert(false); return TypePtr();}

  virtual String getOutputVariableName(size_t index) const
    {jassert(false); return String::empty;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {}

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_NULL_H_

/*-----------------------------------------.---------------------------------.
| Filename: FunctionBasedPerception.h      | Transforms a Function into      |
| Author  : Francis Maes                   |  a single-output Perception     |
| Started : 14/07/2010 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FUNCTION_BASED_H_
# define LBCPP_FUNCTION_PERCEPTION_FUNCTION_BASED_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class FunctionBasedPerception : public Perception
{
public:
  FunctionBasedPerception(FunctionPtr function = FunctionPtr())
    : function(function) {computeOutputVariables();}

  virtual String toString() const
    {return classNameToOutputClassName(function->getClassName());}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual void computeOutputVariables()
    {addOutputVariable(T("value"), function->getOutputType(getInputType()));}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, function->compute(input));}

  juce_UseDebuggingNewOperator

protected:
  friend class FunctionBasedPerceptionClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FUNCTION_BASED_H_

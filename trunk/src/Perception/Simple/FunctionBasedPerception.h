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
  FunctionBasedPerception(FunctionPtr function) : function(function)
    {computeOutputType();}
  FunctionBasedPerception() {}

  virtual String toString() const
    {return classNameToOutputClassName(function->getClassName());}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual void computeOutputType()
  {
    addOutputVariable(T("value"), function->getOutputType(getInputType()));
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, function->computeFunction(context, input));}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionBasedPerceptionClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FUNCTION_BASED_H_

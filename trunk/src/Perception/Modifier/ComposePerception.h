/*-----------------------------------------.---------------------------------.
| Filename: ComposePerception.h            | Compose a function with a       |
| Author  : Francis Maes                   |  Perception                     |
| Started : 06/10/2010 16:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_COMPOSE_H_
# define LBCPP_DATA_PERCEPTION_COMPOSE_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class ComposePerception : public Perception
{
public:
  ComposePerception(FunctionPtr function, PerceptionPtr perception)
    : function(function), perception(perception) {computeOutputType();}
  ComposePerception() {}

  virtual String toString() const
    {return perception->toString() + T("(") + function->toString() + T(")");}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual TypePtr getOutputType() const
    {return perception->getOutputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return perception->getOutputType(inputType);}

  virtual bool isSparse() const
    {return perception->isSparse();}

  virtual void computeOutputType()
  {
    outputVariables = perception->getOutputVariables();
    perceptionInputType = perception->getInputType();
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (context.checkInheritance(function->getOutputType(input.getType()), perceptionInputType))
    {
      Variable intermediate = function->compute(context, input);
      perception->computePerception(context, intermediate, callback);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ComposePerceptionClass;

  FunctionPtr function;
  PerceptionPtr perception;
  TypePtr perceptionInputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_COMPOSE_H_

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
    Perception::computeOutputType();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (checkInheritance(function->getOutputType(input.getType()), perception->getInputType()))
    {
      Variable intermediate = function->compute(input);
      perception->computePerception(intermediate, callback);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ComposePerceptionClass;

  FunctionPtr function;
  PerceptionPtr perception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_COMPOSE_H_

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
    : function(function), perception(perception) {}
  ComposePerception() {}

  virtual String toString() const
    {return function->toString() + T(" => ") + perception->toString();}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual bool isSparse() const
    {return perception->isSparse();}
 
  virtual size_t getNumOutputVariables() const
    {return perception->getNumOutputVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return perception->getOutputVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return perception->getOutputVariableName(index);}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return perception->getOutputVariableSubPerception(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (checkInheritance(function->getOutputType(input.getType()), perception->getInputType()))
    {
      Variable intermediate = function->compute(input);
      perception->computePerception(intermediate, callback);
    }
  }

  juce_UseDebuggingNewOperator

protected:
  friend class ComposePerceptionClass;

  FunctionPtr function;
  PerceptionPtr perception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_COMPOSE_H_

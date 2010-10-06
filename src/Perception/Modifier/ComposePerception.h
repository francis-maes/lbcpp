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

class ComposePerception : public DecoratorPerception
{
public:
  ComposePerception(FunctionPtr function, PerceptionPtr perception)
    : DecoratorPerception(perception), function(function) {}
  ComposePerception() {}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (checkInheritance(function->getOutputType(input.getType()), decorated->getInputType()))
    {
      Variable intermediate = function->compute(input);
      decorated->computePerception(intermediate, callback);
    }
  }

protected:
  friend class ComposePerceptionClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_COMPOSE_H_

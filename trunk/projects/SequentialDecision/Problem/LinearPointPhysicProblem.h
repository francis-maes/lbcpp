/*-----------------------------------------.---------------------------------.
| Filename: LinearPointPhysicSystem.h      | Simple 1D Physic System         |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 15:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
# define LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_

# include "../core/SequentialDecisionProblem.h"

namespace lbcpp
{

extern ClassPtr linearPointPhysicStateClass;

class LinearPointPhysicState : public Object
{
public:
  LinearPointPhysicState(double position = 0.0, double velocity = 0.0)
    : Object(linearPointPhysicStateClass), position(position), velocity(velocity) {}

  double getPosition() const
    {return position;}

  double getVelocity() const
    {return velocity;}

protected:
  friend class LinearPointPhysicStateClass;

  double position;
  double velocity;
};

typedef ReferenceCountedObjectPtr<LinearPointPhysicState> LinearPointPhysicStatePtr;

class LinearPointPhysicStateSampler : public SimpleUnaryFunction
{
public:
  LinearPointPhysicStateSampler()
    : SimpleUnaryFunction(randomGeneratorClass, linearPointPhysicStateClass) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    double position = random->sampleDouble(-1.0, 1.0);
    double velocity = random->sampleDouble(-2.0, 2.0);
    return new LinearPointPhysicState(position, velocity);
  }
};

// state: LinearPointPhysicState, action: boolean
class LinearPointPhysicTransitionFunction : public SimpleBinaryFunction
{
public:
  LinearPointPhysicTransitionFunction()
    : SimpleBinaryFunction(linearPointPhysicStateClass, booleanType, linearPointPhysicStateClass) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const LinearPointPhysicStatePtr& state = inputs[0].getObjectAndCast<LinearPointPhysicState>();
    bool action = inputs[1].getBoolean();

    static const double deltaT = 0.1;
    double newVelocity = state->getVelocity() + deltaT * (action ? 1.0 : -1.0);
    return new LinearPointPhysicState(state->getPosition() + deltaT * newVelocity, newVelocity);
  }
};

class LinearPointPhysicRewardFunction : public SimpleBinaryFunction
{
public:
  LinearPointPhysicRewardFunction()
    : SimpleBinaryFunction(linearPointPhysicStateClass, booleanType, doubleType) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const LinearPointPhysicStatePtr& state = inputs[0].getObjectAndCast<LinearPointPhysicState>();
    bool action = inputs[1].getBoolean();

    static const double deltaT = 0.1;
    double newVelocity = state->getVelocity() + deltaT * (action ? 1.0 : -1.0);
    double newPosition = state->getPosition() + deltaT * newVelocity;
    return juce::jmax(0.0, 1.0 - newPosition * newPosition);
  }
};

class LinearPointPhysicProblem : public SequentialDecisionProblem
{
public:
  LinearPointPhysicProblem() 
    : SequentialDecisionProblem(new LinearPointPhysicStateSampler(), 
                                new LinearPointPhysicTransitionFunction(),
                                new LinearPointPhysicRewardFunction()) {}
};

extern SequentialDecisionProblemPtr linearPointPhysicProblem();

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
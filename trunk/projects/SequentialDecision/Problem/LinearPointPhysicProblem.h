/*-----------------------------------------.---------------------------------.
| Filename: LinearPointPhysicSystem.h      | Simple 1D Physic System         |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 15:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
# define LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_

# include "../Core/DecisionProblem.h"
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

extern ClassPtr linearPointPhysicStateClass;

class LinearPointPhysicState : public DecisionProblemState
{
public:
  LinearPointPhysicState(double position = 0.0, double velocity = 0.0)
    : position(position), velocity(velocity) {}

  double getPosition() const
    {return position;}

  double getVelocity() const
    {return velocity;}

  virtual TypePtr getActionType() const
    {return booleanType;}

  virtual ContainerPtr getAvailableActions() const
  {
    VectorPtr res = new GenericVector(booleanType, 2);
    res->setElement(0, false);
    res->setElement(1, true);
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    static const double deltaT = 0.1;
    position += deltaT * velocity;
    velocity += deltaT * (action.getBoolean() ? 1.0 : -1.0);
    reward = juce::jmax(0.0, 1.0 - position * position);
  }

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
    //return new LinearPointPhysicState(-1.0, 0.0);
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    double position = random->sampleDouble(-1.0, 1.0);
    double velocity = random->sampleDouble(-2.0, 2.0);
    return new LinearPointPhysicState(position, velocity);
  }
};

class LinearPointPhysicProblem : public DecisionProblem
{
public:
  LinearPointPhysicProblem(double discount = 0.9) 
    : DecisionProblem(new LinearPointPhysicStateSampler(), discount) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual TypePtr getActionType() const
    {return booleanType;}

  virtual size_t getFixedNumberOfActions() const
    {return 2;}
};

extern DecisionProblemPtr linearPointPhysicProblem(double discount);

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
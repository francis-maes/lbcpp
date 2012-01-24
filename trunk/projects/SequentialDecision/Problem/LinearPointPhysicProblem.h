/*-----------------------------------------.---------------------------------.
| Filename: LinearPointPhysicSystem.h      | Simple 1D Physic System         |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 15:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
# define LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

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
    {return doubleType;}

  virtual ContainerPtr getAvailableActions() const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(2, 0.0);
    res->setValue(0, -1.0);
    res->setValue(1, 1.0);
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    static const double deltaT = 0.1;
    position += deltaT * velocity;
    velocity += deltaT * action.getDouble();
    reward = juce::jmax(0.0, 1.0 - position * position);
  }

protected:
  friend class LinearPointPhysicStateClass;

  double position;
  double velocity;
};

typedef ReferenceCountedObjectPtr<LinearPointPhysicState> LinearPointPhysicStatePtr;

class LinearPointPhysicProblem : public DecisionProblem
{
public:
  LinearPointPhysicProblem() 
    : DecisionProblem(FunctionPtr(), 0.9, 50) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual ClassPtr getStateClass() const
    {return linearPointPhysicStateClass;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual size_t getFixedNumberOfActions() const
    {return 2;}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
  {
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    double position = random->sampleDouble(-1.0, 1.0);
    double velocity = random->sampleDouble(-2.0, 2.0);
    return new LinearPointPhysicState(position, velocity);
  }

  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
    {return sampleInitialState(context);}

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
  {
    ObjectVectorPtr res = new ObjectVector(linearPointPhysicStateClass, 11 * 11);
    size_t index = 0;
    for (size_t i = 0; i < 11; ++i)
      for (size_t j = 0; j < 11; ++j)
        res->set(index++, new LinearPointPhysicState(-1.0 + i * 0.2, -2.0 + j * 0.4));
    numTrajectoriesToValidate = 1; // system is deterministic
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_

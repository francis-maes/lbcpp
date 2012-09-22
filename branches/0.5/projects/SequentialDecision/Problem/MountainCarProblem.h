/*-----------------------------------------.---------------------------------.
| Filename: MountainCarProblem.h           | The classical mountain car      |
| Author  : Francis Maes                   |                                 |
| Started : 23/01/2012 17:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_MOUNTAIN_CAR_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_MOUNTAIN_CAR_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

extern ClassPtr mountainCarStateClass;

class MountainCarState : public DecisionProblemState
{
public:
  MountainCarState(double position = 0.0, double velocity = 0.0)
    : position(position), velocity(velocity) {}

  double getPosition() const
    {return position;}

  double getVelocity() const
    {return velocity;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual ContainerPtr getAvailableActions() const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(3, 0.0);
    res->setValue(0, -1.0);
    res->setValue(1, 0.0);
    res->setValue(2, 1.0);
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    double a = action.getDouble();
    velocity = juce::jlimit(-0.07, 0.07, velocity + a * 0.001 - 0.0025 * cos(3 * position));
    position = juce::jlimit(-1.2, 0.6, position + velocity);
    if (position == -1.2 && velocity < 0.0)
      velocity = 0.0;
    reward = -1;
  }

  virtual bool isFinalState() const
    {return position >= 0.5;}

protected:
  friend class MountainCarStateClass;

  double position;
  double velocity;
};

typedef ReferenceCountedObjectPtr<MountainCarState> MountainCarStatePtr;

class MountainCarStateSampler : public SimpleUnaryFunction
{
public:
  MountainCarStateSampler()
    : SimpleUnaryFunction(randomGeneratorClass, mountainCarStateClass) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    double position = random->sampleDouble(-1.2, 0.6);
    double velocity = random->sampleDouble(-0.07, 0.07);
    return new MountainCarState(position, velocity);
  }
};

class MountainCarProblem : public DecisionProblem
{
public:
  MountainCarProblem(double discount = 1.0) 
    : DecisionProblem(new MountainCarStateSampler(), discount) {}

  virtual double getMaxReward() const
    {return 0.0;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual size_t getFixedNumberOfActions() const
    {return 3;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_MOUNTAIN_CAR_H_

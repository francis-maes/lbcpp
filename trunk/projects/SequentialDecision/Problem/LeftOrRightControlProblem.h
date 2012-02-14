/*-----------------------------------------.---------------------------------.
| Filename: LeftOrRightControlProblem.h    | Left-or-Right Control Problem   |
| Author  : Francis Maes                   | From FQI Paper                  |
| Started : 24/01/2012 13:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_LEFT_OR_RIGHT_CONTROL_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_LEFT_OR_RIGHT_CONTROL_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

extern ClassPtr leftOrRightControlStateClass;

class LeftOrRightControlState : public DecisionProblemState
{
public:
  LeftOrRightControlState(double position = 0.0) : position(position) {}

  double getPosition() const
    {return position;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual ContainerPtr getAvailableActions() const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(2, 0.0);
    res->setValue(0, -2.0);
    res->setValue(1, 2.0);
    return res;
  }

  virtual bool isFinalState() const
    {return position < 0.0 || position > 10.0;}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    position += action.getDouble() + context.getRandomGenerator()->sampleDoubleFromGaussian();
    if (position < 0.0)
      reward = 50.0;
    else if (position > 10.0)
      reward = 100.0;
    else
      reward = 0.0;
  }

protected:
  friend class LeftOrRightControlStateClass;

  double position;
};

typedef ReferenceCountedObjectPtr<LeftOrRightControlState> LeftOrRightControlStatePtr;

class LeftOrRightControlStateSampler : public SimpleUnaryFunction
{
public:
  LeftOrRightControlStateSampler()
    : SimpleUnaryFunction(randomGeneratorClass, leftOrRightControlStateClass) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    double position = random->sampleDouble(0.0, 10.0);
    return new LeftOrRightControlState(position);
  }
};

class LeftOrRightControlProblem : public DecisionProblem
{
public:
  LeftOrRightControlProblem() 
    : DecisionProblem(new LeftOrRightControlStateSampler(), 0.75, 20) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual double getMaxCumulativeReward() const
    {return 100.0;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual size_t getFixedNumberOfActions() const
    {return 2;}

  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
    {return sampleInitialState(context);}

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
  {
    numTrajectoriesToValidate = 100000; // should be 100,000
    ObjectVectorPtr res = new ObjectVector(leftOrRightControlStateClass, 11);
    for (size_t i = 0; i <= 10; ++i)
      res->set(i, new LeftOrRightControlState((double)i));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_LEFT_OR_RIGHT_CONTROL_H_

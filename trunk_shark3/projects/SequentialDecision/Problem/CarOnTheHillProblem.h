/*-----------------------------------------.---------------------------------.
| Filename: CarOnTheHillProblem.h          | "Car on the hill" problem       |
| Author  : Francis Maes                   |                                 |
| Started : 24/01/2012 18:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_CAR_ON_THE_HILL_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_CAR_ON_THE_HILL_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

extern ClassPtr carOnTheHillStateClass;

class CarOnTheHillState : public DecisionProblemState
{
public:
  CarOnTheHillState(double position = 0.0, double velocity = 0.0)
    : position(position), velocity(velocity) {}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual ContainerPtr getAvailableActions() const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(2, 0.0);
    res->setValue(0, -4.0);
    res->setValue(1, 4.0);
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
  {
    static const double deltaT = 0.001;
    static const double Mcar = 1.0;
    static const double g = 9.81;

    for (size_t i = 0; i < 100; ++i)
    {
      double action = ac.getDouble();
      double acceleration = action /(Mcar * (1 + pow(dH(position),2.)))
                - g*dH(position)/(1+pow(dH(position),2))
                -(pow(velocity,2.)*ddH(position)*dH(position))/(1.+pow(dH(position),2.));
      
      position += deltaT * velocity;
      velocity += deltaT * acceleration;
    }

    reward = isFinalState() ? (((position < -1.0 || fabs(velocity) > 3.0)) ? -1 : 1) : 0.0;
  }

  virtual bool isFinalState() const
    {return fabs(position) > 1.0 || fabs(velocity) > 3.0;}

protected:
  friend class CarOnTheHillStateClass;

  double position;
  double velocity;

  static double dH(double x)
  {
    if (x < 0.0)
      return (2*x+1);
    else 
      return (1./(pow(1.+5.*pow(x,2.),1.5)));
  }

  static double ddH(double x)
  {
    if (x < 0.0)
      return 2.0;
    else
      return (-(1.5*5.*2.*x)/pow(1.+5.*pow(x,2.),2.5));
  }
};

typedef ReferenceCountedObjectPtr<CarOnTheHillState> CarOnTheHillStatePtr;

class CarOnTheHillProblem : public DecisionProblem
{
public:
  CarOnTheHillProblem() 
    : DecisionProblem(FunctionPtr(), 0.95, 1000) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual ClassPtr getStateClass() const
    {return carOnTheHillStateClass;}
  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual size_t getFixedNumberOfActions() const
    {return 2;}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
  {
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    double position = random->sampleDouble(-1.1, 1.1);
    double velocity = random->sampleDouble(-3.1, 3.1);
    return new CarOnTheHillState(position, velocity);
  }

  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
    {return sampleInitialState(context);}

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
  {
    ObjectVectorPtr res = new ObjectVector(carOnTheHillStateClass, 17 * 17);
    size_t index = 0;
    for (size_t i = 0; i < 17; ++i)
      for (size_t j = 0; j < 17; ++j)
        res->set(index++, new CarOnTheHillState(-1.0 + i * 0.125, -3.0 + j * 0.375));
    numTrajectoriesToValidate = 1; // system is deterministic
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_CAR_ON_THE_HILL_H_

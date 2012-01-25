/*-----------------------------------------.---------------------------------.
| Filename: AcrobotProblem.h               | Acrobot problem                 |
| Author  : Francis Maes                   |                                 |
| Started : 24/01/2012 19:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_ACROBOT_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_ACROBOT_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

extern ClassPtr acrobotStateClass;

class AcrobotState : public DecisionProblemState
{
public:
  AcrobotState(double angle1 = 0.0, double velocity1 = 0.0, double angle2 = 0.0, double velocity2 = 0.0)
    : angle1(angle1), velocity1(velocity1), angle2(angle2), velocity2(velocity2) {}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual ContainerPtr getAvailableActions() const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(2, 0.0);
    res->setValue(0, -5.0);
    res->setValue(1, 5.0);
    return res;
  }

  std::vector<double> computeGradient(double u) const
  {
    static const double g = 9.81;
    static const double M1 = 1.0;
    static const double M2 = 1.0;
    static const double L1 = 1.0;
    static const double L2 = 1.0;
    static const double mu1 = 0.01;
    static const double mu2 = 0.01;

    double d11 = M1*pow(L1,2.)+M2*(pow(L1,2.)+pow(L2,2.)+2*L1*L2*cos(angle2));
    double d22 = M2*pow(L2,2.);
    double d12 = M2*(pow(L2,2.)+L1*L2*cos(angle2));
    double c1 = -M2*L1*L2*velocity2*(2*velocity1+velocity2)*sin(angle2);
    double c2 = M2*L1*L2*pow(velocity1,2.)*sin(angle2);
    double phi1 = (M1*L1+M2*L1)*g*sin(angle1)+M2*L2*g*sin(angle1+angle2);
    double phi2 = M2*L2*g*sin(angle1+angle2);
    
    std::vector<double> res(4);
    res[0] = velocity1;
    res[1] = (-mu1*velocity1-phi1-c1-(d12/d22)*(u-mu2*velocity2-c2-phi2))/(d11-pow(d12,2.)/d22);
    res[2] = velocity2;
    res[3] = (u-mu2*velocity2-phi2-c2-(d12/d11)*(-mu1*velocity1-c1-phi1))/(d22-pow(d12,2.)/d11);
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
  {
    static const double deltaT = 0.001;

    for (size_t i = 0; i < 100; ++i)
    {
      std::vector<double> gradient = computeGradient(ac.getDouble());
      angle1 += deltaT * gradient[0];
      velocity1 += deltaT * gradient[1];
      angle2 += deltaT * gradient[2];
      velocity2 += deltaT * gradient[3];
    }
    normalizeAngle(angle1);
    normalizeAngle(angle2);

    double d = computeDistanceFromEquilibrium();
    reward = d < 1.0 ? 1.0 - d : 0.0;
  }

  virtual bool isFinalState() const
    {return computeDistanceFromEquilibrium() < 1.0;}

protected:
  friend class AcrobotStateClass;

  double angle1;
  double velocity1;
  double angle2;
  double velocity2;

  static inline double sqr(double x) {return x * x;}

  double computeDistanceFromEquilibrium() const
  {
    static const double pi = acos(-1.);
    double x0 = pow(pow(angle1,2.),0.5);
    double x1 = pow(pow(velocity1,2.),0.5);
    double x2 = pow(pow(angle2,2.),0.5);
    double x3 = pow(pow(velocity2,2.),0.5);
    return sqrt(sqr(x0 - pi) + sqr(x1) + sqr(x2) + sqr(x3));
  }

  void normalizeAngle(double& angle)
  {
    static const double pi = acos(-1.0);

    while (true)
    {
      if (angle > pi)
	      angle -= 2*pi;
      else if (angle < -pi)
	      angle += 2*pi;
      else
        break;
    }
  }
};

typedef ReferenceCountedObjectPtr<AcrobotState> AcrobotStatePtr;

class AcrobotProblem : public DecisionProblem
{
public:
  AcrobotProblem() 
    : DecisionProblem(FunctionPtr(), 0.95, 100) {}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual double getMaxCumulativeReward() const
    {return 1.0;}

  virtual ClassPtr getStateClass() const
    {return acrobotStateClass;}

  virtual TypePtr getActionType() const
    {return doubleType;}

  virtual size_t getFixedNumberOfActions() const
    {return 2;}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
    {return new AcrobotState(context.getRandomGenerator()->sampleDouble(-2.0, 2.0), 0.0, 0.0, 0.0);}

  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
  {
    static const double pi = acos(-1.0);

    const RandomGeneratorPtr& random = context.getRandomGenerator();
    return new AcrobotState(random->sampleDouble(-pi, pi), random->sampleDouble(-10, 10),
                            random->sampleDouble(-pi, pi), random->sampleDouble(-10, 10));
  }

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
  {
    ObjectVectorPtr res = new ObjectVector(acrobotStateClass, 21);
    for (size_t i = 0; i < 21; ++i)
      res->set(i, new AcrobotState(-2.0 + i * 0.1, 0.0, 0.0, 0.0));
    numTrajectoriesToValidate = 1; // system is deterministic
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_ACROBOT_H_

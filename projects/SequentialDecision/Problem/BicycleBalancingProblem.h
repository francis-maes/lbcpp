/*-----------------------------------------.---------------------------------.
| Filename: BicyleBalancingProblem.h       | Bicyle Balancing problem        |
| Author  : Francis Maes                   |                                 |
| Started : 25/01/2012 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_BICYCLE_BALANCING_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_BICYCLE_BALANCING_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

class BicyleBalancingState;
typedef ReferenceCountedObjectPtr<BicyleBalancingState> BicyleBalancingStatePtr;
extern ClassPtr bicyleBalancingStateClass;

class BicyleBalancingState : public DecisionProblemState
{
public:
  BicyleBalancingState(double omega, double omega_dot, double theta, double theta_dot, double xb, double yb, double psi, bool useRidingReward)
    : omega(omega), omega_dot(omega_dot), theta(theta), theta_dot(theta_dot), xb(xb), yb(yb), psi(psi), useRidingReward(useRidingReward) {}
  BicyleBalancingState() : omega(0.0), omega_dot(0.0), theta(0.0), theta_dot(0.0), xb(0.0), yb(0.0), psi(0.0), useRidingReward(false) {}

  virtual TypePtr getActionType() const
    {return pairClass(doubleType, doubleType);} // d, T

  virtual ContainerPtr getAvailableActions() const
  {
    ClassPtr cl = (ClassPtr)getActionType();
    ObjectVectorPtr res = new ObjectVector(cl, 9);
    res->set(0, new Pair(cl, -0.02, -2.0));
    res->set(1, new Pair(cl, -0.02, 0.0));
    res->set(2, new Pair(cl, -0.02, 2.0));
    res->set(3, new Pair(cl, 0.0, -2.0));
    res->set(4, new Pair(cl, 0.0, 0.0));
    res->set(5, new Pair(cl, 0.0, 2.0));
    res->set(6, new Pair(cl, 0.02, -2.0));
    res->set(7, new Pair(cl, 0.02, 0.0));
    res->set(8, new Pair(cl, 0.02, 2.0));
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& ac, double& reward, Variable* stateBackup = NULL)
  {
    static const double l = 1.11;     /* distance between the point where the front and back tyre touch the ground */
	  static const double c = 0.66;
    static const double h = 0.94;

    static const double Mc = 15.0;
    static const double Md = 1.7;
	  static const double Mp = 60.0;
	  static const double M = Mc + Mp;

    static const double R = 0.34;          /* tyre radius */
    static const double I_dc = Md * R * R;
    static const double v = 10.0 / 3.6;  /* 10 km/t in m/s */
    static const double sigma_dot = v / R;

    static const double dCM = 0.3;
    static const double I_bike = ((13.0/3)*Mc*h*h + Mp*(h+dCM)*(h+dCM));
    static const double I_dv = ((3.0/2)*Md*R*R);  
	  static const double I_dl = ((1.0/2)*Md*R*R);  

    static const double g = 9.81;
    static const double dt = 0.01;

    const RandomGeneratorPtr& random = context.getRandomGenerator();
    const PairPtr& action = ac.getObjectAndCast<Pair>();
    double d = action->getFirst().getDouble();
    double T = action->getSecond().getDouble();

//	  double previousXfValue = xb+l*cos(psi);
//	  double previousXbValue = xb;
	  double previousPsi = psi;
  
    double rf;
	  double rb;
	  double rCM;
    if (theta == 0)
    {
	      rb = 9999999.; 
	      rf = 9999999.; 
	      rCM = 9999999.;
	  }
    else
    {
	      rf = l / fabs(sin(theta));
	      rb = l / abs(tan(theta));
	      rCM = sqrt(pow(l-c,2) + l*l/(pow(tan(theta),2)));
	  } 

    //
    /* Main physics eq. in the bicycle model coming here: */
    double noise = random->sampleDouble(-0.02,0.02);
	  double phi = omega + atan((d+noise)/h);
	  double omega_d_dot;
	  if (theta >= 0)
	    omega_d_dot=( h*M*g*sin(phi)  - cos(phi)*(I_dc*sigma_dot*theta_dot + v*v*(Md*R*(1.0/rf + 1.0/rb) +  M*h/rCM))) / I_bike;
	  else
	    omega_d_dot=( h*M*g*sin(phi)  - cos(phi)*(I_dc*sigma_dot*theta_dot - v*v*(Md*R*(1.0/rf + 1.0/rb) +  M*h/rCM))) / I_bike;

  //	omega_d_dot=0.;
	  double theta_d_dot = (T - I_dv*omega_dot*sigma_dot) /  I_dl;
  /*--- Eulers method ---*/
	  double omega_dot_old=omega_dot;
	  double theta_dot_old=theta_dot;
	  double theta_old=theta;
	  omega_dot = omega_dot+omega_d_dot * dt;
	  omega = omega+omega_dot_old * dt;
	  theta_dot = theta_dot+theta_d_dot * dt;
	  theta = theta+theta_dot_old * dt;
	  if (fabs(theta) > 80./180.*pi)
    { 
      // handlebars cannot turn more than 80 degrees
      if (theta >= 0.)
	      theta =  80./180.*pi;
      else
	      theta =  -80./180.*pi;
	      
      // If the handlebars is hitting the limits that the speed is also set equal to 0 (from Damien)
	    theta_dot = 0.;
	  }
 	  xb += v * dt * cos(psi);
 	  yb += v * dt * sin(psi);
	  if (theta_old  > 0.)
      psi = psi+ v*dt/rb;
	  else
      psi = psi - v*dt/rb;
	  

    if (isFinalState())
      reward = -1.0;
    else if (useRidingReward)
    { 
      static const double coeff = 0.1;
    	reward = coeff * (fabs(normalizeAngle(previousPsi)) - fabs(normalizeAngle(psi)));
    }
    else
      reward = 0.0;
  }

  virtual bool isFinalState() const
    {return fabs(omega) > pi / 15.0;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const BicyleBalancingStatePtr& target = t.staticCast<BicyleBalancingState>();
    DecisionProblemState::clone(context, target);
    target->xb = xb;
    target->yb = yb;
    target->useRidingReward = useRidingReward;
  }

protected:
  friend class BicyleBalancingStateClass;

  double omega;
  double omega_dot;
  double theta;
  double theta_dot;
  double xb;
  double yb;
  double psi;

  bool useRidingReward;

  static const double pi;

  double normalizeAngle(double angle)
  {
    while (true)
    {
      if (angle > pi)
	      angle -= 2*pi;
      else if (angle < -pi)
	      angle += 2*pi;
      else
        break;
    }
    return angle;
  }
};

class BicyleBalancingProblem : public DecisionProblem
{
public:
  BicyleBalancingProblem() 
    : DecisionProblem(FunctionPtr(), 0.98, 50000) {}

  virtual double getMaxReward() const
    {return 0.0;}

  virtual double getMaxCumulativeReward() const
    {return 0.0;}

  virtual ClassPtr getStateClass() const
    {return bicyleBalancingStateClass;}

  virtual TypePtr getActionType() const
    {return pairClass(doubleType, doubleType);}

  virtual size_t getFixedNumberOfActions() const
    {return 9;}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const
    {return new BicyleBalancingState(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, context.getRandomGenerator()->sampleDouble(-pi, pi), useRidingReward);}

  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
  {
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    return new BicyleBalancingState(
          random->sampleDouble(-pi / 15.0, pi / 15.0), random->sampleDouble(-10, 10),
          random->sampleDouble(-4.0 * pi / 7.0, 4.0 * pi / 7.0), random->sampleDouble(-10, 10),
          random->sampleDouble(-1000.0, 1000.0), // xb
          random->sampleDouble(-200.0, 200.0), // yb
          random->sampleDouble(-pi, pi),
          useRidingReward);
  }

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
  {
    ObjectVectorPtr res = new ObjectVector(bicyleBalancingStateClass, 9);
    for (size_t i = 0; i < 9; ++i)
      res->set(i, new BicyleBalancingState(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, pi * (-1.0 + i / 8.0), useRidingReward));
    numTrajectoriesToValidate = 10;
    return res;
  }

protected:
  friend class BicyleBalancingProblemClass;

  bool useRidingReward;

  static const double pi;
};

const double BicyleBalancingState::pi = acos(-1.0);
const double BicyleBalancingProblem::pi = acos(-1.0);


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_BICYCLE_BALANCING_H_

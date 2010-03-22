/*-----------------------------------------.---------------------------------.
| Filename: SimulationStateValueFunction.h | Simulation State Value Function |
| Author  : Francis Maes                   |                                 |
| Started : 22/06/2009 20:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_SIMULATION_STATE_H_
# define LBCPP_CORE_VALUE_FUNCTION_SIMULATION_STATE_H_

# include <lbcpp/ChooseFunction.h>

namespace lbcpp
{

class SimulationStateValueFunction : public StateValueFunction
{
public:
  SimulationStateValueFunction(PolicyPtr policy, double discount = 1, size_t horizon = 0)
    : policy(policy), discount(discount), horizon(horizon) {}
  SimulationStateValueFunction() : discount(0), horizon(0) {}  
  
  virtual std::string toString() const
    {return "simulationStateValues(" + policy->toString() + ", " +
      lbcpp::toString(discount) + ", " + lbcpp::toString(horizon) + ")";}
  
  virtual void setChoose(ChoosePtr choose)
    {this->choose = choose;}
  
  virtual double compute() const
  {
    ChoosePtr choose = this->choose->cloneAndCast<Choose>();
    
    double res = 0.0;
    for (size_t step = 0; true; ++step)
    {
      VariablePtr choice = policy->policyChoose(choose);
      double reward;
      choose = choose->getCRAlgorithm()->runUntilNextChoose(choice, &reward);
      policy->policyReward(reward);
      res *= discount;
      res += reward;
      if (!choose || (horizon > 0 && step == horizon))
        break;
    }
    return res;
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, policy) && read(istr, discount) && read(istr, horizon);}

  virtual void save(std::ostream& ostr) const
    {write(ostr, policy); write(ostr, discount); write(ostr, horizon);}
  
private:
  PolicyPtr policy;
  double discount;
  size_t horizon;

  ChoosePtr choose;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_SIMULATION_STATE_H_

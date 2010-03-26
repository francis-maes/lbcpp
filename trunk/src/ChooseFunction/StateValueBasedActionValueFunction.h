/*-----------------------------------------.---------------------------------.
| Filename: StateValueBasedActionValueF...h| Action Values based on          |
| Author  : Francis Maes                   |     State Values                |
| Started : 22/06/2009 20:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_STATE_VALUE_BASED_H_
# define LBCPP_CORE_VALUE_FUNCTION_STATE_VALUE_BASED_H_

# include <lbcpp/ChooseFunction.h>

namespace lbcpp
{

class StateValueBasedActionValueFunction : public ActionValueFunction
{
public:
  StateValueBasedActionValueFunction(StateValueFunctionPtr stateValues, double discount)
    : stateValues(stateValues), discount(discount) {}
  StateValueBasedActionValueFunction() : discount(0.0) {}
  
  virtual String toString() const
    {return "stateValueBasedActionValues(" + stateValues->toString() + ", " + lbcpp::toString(discount) + ")";}
  
  virtual void setChoose(ChoosePtr choose)
    {this->choose = choose;}
  
  virtual double compute(VariablePtr choice) const
  {
    ChoosePtr choose = this->choose->cloneAndCast<Choose>();
    double reward;
    choose = choose->getCRAlgorithm()->runUntilNextChoose(choice, &reward);
    stateValues->setChoose(choose);
    return reward + discount * stateValues->compute();
  }
  
  virtual bool load(InputStream& istr)
    {return read(istr, stateValues) && read(istr, discount);}
    
  virtual void save(OutputStream& ostr) const
    {write(ostr, stateValues); write(ostr, discount);}
  
private:
  StateValueFunctionPtr stateValues;
  double discount;
  
  ChoosePtr choose;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_STATE_VALUE_BASED_H_

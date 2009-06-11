/*-----------------------------------------.---------------------------------.
| Filename: GreedyPolicy.h                 | Greedy Policy                   |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_GREEDY_H_
# define LBCPP_POLICY_GREEDY_H_

# include <lbcpp/Policy.h>

namespace lbcpp
{

class GreedyPolicy : public Policy
{
public:
  GreedyPolicy(ActionValueFunctionPtr actionValue = ActionValueFunctionPtr())
    : actionValue(actionValue) {}
    
  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return choose->sampleBestChoice(actionValue);}
  
  virtual std::string toString() const
    {return "greedyPolicy(" + actionValue->toString() + ")";}
    
  virtual void save(std::ostream& ostr) const
    {write(ostr, actionValue);}

  virtual bool load(std::istream& istr)
    {return read(istr, actionValue);}
    
private:
  ActionValueFunctionPtr actionValue;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_GREEDY_H_

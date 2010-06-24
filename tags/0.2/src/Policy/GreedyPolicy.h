/*-----------------------------------------.---------------------------------.
| Filename: GreedyPolicy.h                 | Greedy Policy                   |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_GREEDY_H_
# define LBCPP_POLICY_GREEDY_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

class GreedyPolicy : public Policy
{
public:
  GreedyPolicy(ActionValueFunctionPtr actionValue = ActionValueFunctionPtr())
    : actionValue(actionValue) {}
    
  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return choose->sampleBestChoice(actionValue);}
  
  virtual String toString() const
    {return "greedyPolicy(" + actionValue->toString() + ")";}
    
  virtual void save(OutputStream& ostr) const
    {write(ostr, actionValue);}

  virtual bool load(InputStream& istr)
    {return read(istr, actionValue);}
    
private:
  ActionValueFunctionPtr actionValue;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_GREEDY_H_

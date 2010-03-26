/*-----------------------------------------.---------------------------------.
| Filename: GibbsGreedyPolicy.h            | Gibbs greedy Policy             |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_GIBBS_GREEDY_H_
# define LBCPP_POLICY_GIBBS_GREEDY_H_

# include <lbcpp/Policy.h>

namespace lbcpp
{

class GibbsGreedyPolicy : public Policy
{
public:
  GibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature)
    : actionValue(actionValue), temperature(temperature) {}
  GibbsGreedyPolicy() {}

  // P[a | s] = exp(Q(s,a) / T)
  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    double T = temperature->compute(numChooses);
    jassert(T);
    ++numChooses;

    std::vector<double> actionValues;
    choose->computeActionValues(actionValues, actionValue);
    double sum = 0;
    for (size_t i = 0; i < actionValues.size(); ++i)
    {
      double p = exp(actionValues[i] / T);
      sum += p;
      actionValues[i] = p;
    }
    return choose->sampleChoiceWithProbabilities(actionValues, sum);
  }
  
  virtual String toString() const
    {return "gibbsGreedyPolicy(" + actionValue->toString() + ", " + temperature->toString() + ")";}
  
  virtual void save(std::ostream& ostr) const
    {write(ostr, actionValue);}

  virtual bool load(std::istream& istr)
    {return read(istr, actionValue);}
    
private:
  size_t numChooses;
  ActionValueFunctionPtr actionValue;
  IterationFunctionPtr temperature;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_GIBBS_GREEDY_H_

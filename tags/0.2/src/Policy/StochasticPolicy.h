/*-----------------------------------------.---------------------------------.
| Filename: StochasticPolicy.h             | Stochastic Policy defined by    |
| Author  : Francis Maes                   |   a conditional distribution    |
| Started : 11/06/2009 21:41               |     P[a|s]                      |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_STOCHASTIC_H_
# define LBCPP_POLICY_STOCHASTIC_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

class StochasticPolicy : public Policy
{
public:
  StochasticPolicy(ActionValueFunctionPtr actionProbabilities = ActionValueFunctionPtr())
    : actionProbabilities(actionProbabilities) {}

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    std::vector<double> probabilities;
    choose->computeActionValues(probabilities, actionProbabilities);
#ifdef NDEBUG
    double sum = 0.0;
    for (size_t i = 0; i < probabilities.size(); ++i)
      sum += probabilities[i];
    jassert(fabs(sum - 1.0) < 0.000001);
#endif
    return choose->sampleChoiceWithProbabilities(probabilities, 1.0);
  }
  
  virtual String toString() const
    {return "stochasticPolicy(" + actionProbabilities->toString() + ")";}
    
  virtual void save(OutputStream& ostr) const
    {write(ostr, actionProbabilities);}

  virtual bool load(InputStream& istr)
    {return read(istr, actionProbabilities);}

private:
  ActionValueFunctionPtr actionProbabilities;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_STOCHASTIC_H_

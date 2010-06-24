/*-----------------------------------------.---------------------------------.
| Filename: MixturePolicy.h                | Mixture Policy                  |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_MIXTURE_H_
# define LBCPP_POLICY_MIXTURE_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

class MixturePolicy : public Policy
{
public:
  MixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient)
    : policy1(policy1), policy2(policy2), mixtureCoefficient(mixtureCoefficient) {}
  MixturePolicy() : mixtureCoefficient(0.5) {}
    
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    policy1->policyEnter(crAlgorithm);
    policy2->policyEnter(crAlgorithm);
  }
  
  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr choice1 = policy1->policyChoose(choose);
    VariablePtr choice2 = policy2->policyChoose(choose);
    return RandomGenerator::getInstance().sampleBool(mixtureCoefficient) ? choice2 : choice1;
  }
  
  virtual void policyReward(double reward)
  {
    policy1->policyReward(reward);
    policy2->policyReward(reward);
  }
  
  virtual void policyLeave()
  {
    policy1->policyLeave();
    policy2->policyLeave();
  }
  
  virtual String toString() const
    {return "mixturePolicy(" + policy1->toString() + ", " + policy2->toString() + ", " + lbcpp::toString(mixtureCoefficient) + ")";}
  
  virtual bool load(InputStream& istr)
    {return read(istr, policy1) && read(istr, policy2) && read(istr, mixtureCoefficient);}
  
  virtual void save(OutputStream& ostr) const
    {write(ostr, policy1); write(ostr, policy2); write(ostr, mixtureCoefficient);}
  
private:
  PolicyPtr policy1;
  PolicyPtr policy2;
  double mixtureCoefficient;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_MIXTURE_H_

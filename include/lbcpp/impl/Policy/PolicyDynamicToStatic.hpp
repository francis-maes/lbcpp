/*-----------------------------------------.---------------------------------.
| Filename: PolicyDynamicToStatic.hpp      | Policy dynamic -> static bridge |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 13:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_DYNAMIC_TO_STATIC_H_
# define LBCPP_CORE_IMPL_POLICY_DYNAMIC_TO_STATIC_H_

# include "PolicyStatic.hpp"

namespace lbcpp {
namespace impl {

struct DynamicToStaticPolicy : public Policy<DynamicToStaticPolicy>
{
  DynamicToStaticPolicy(PolicyPtr policy) : policy(policy) {}
  
  PolicyPtr policy;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
    {policy->policyEnter(crAlgorithm);}
    
  VariablePtr policyChoose(ChoosePtr choose)
    {return policy->policyChoose(choose);}
    
  void policyReward(double reward)
    {policy->policyReward(reward);}
    
  void policyLeave()
    {policy->policyLeave();}

  size_t getNumResults() const
    {return policy->getNumResults();}
    
  ObjectPtr getResult(size_t i) const
    {return policy->getResult(i);}
};

inline DynamicToStaticPolicy dynamicToStatic(PolicyPtr policy)
  {return DynamicToStaticPolicy(policy);}

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_DYNAMIC_TO_STATIC_H_

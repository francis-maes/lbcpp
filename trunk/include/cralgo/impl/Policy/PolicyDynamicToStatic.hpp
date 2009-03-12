/*-----------------------------------------.---------------------------------.
| Filename: PolicyDynamicToStatic.hpp      | Policy dynamic -> static bridge |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 13:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_DYNAMIC_TO_STATIC_H_
# define CRALGO_IMPL_POLICY_DYNAMIC_TO_STATIC_H_

# include "PolicyStatic.hpp"

namespace cralgo {
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
};

inline DynamicToStaticPolicy dynamicToStatic(PolicyPtr policy)
  {return DynamicToStaticPolicy(policy);}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_DYNAMIC_TO_STATIC_H_

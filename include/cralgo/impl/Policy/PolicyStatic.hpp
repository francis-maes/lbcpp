/*-----------------------------------------.---------------------------------.
| Filename: PolicyStatic.hpp               | Static Policy Interface         |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_STATIC_H_
# define CRALGO_IMPL_POLICY_STATIC_H_

# include "../../CRAlgorithm.h"

namespace cralgo {
namespace impl {

template<class ExactType>
struct Object
{
  std::string toString() const {return "";}
  
  void save(std::ostream& ostr) const {}
  bool load(std::istream& istr) {return true;}
};

template<class ExactType>
struct Policy : public Object<ExactType>
{
  void policyEnter(CRAlgorithmPtr crAlgorithm) {}
  const void* policyChoose(ChoosePtr choose) {assert(false); return NULL;}
  void policyReward(double reward) {}
  void policyLeave() {}  
};

template<class ExactType, class DecoratedType>
struct DecoratorPolicy : public Policy<ExactType>
{
  DecoratorPolicy(const DecoratedType& decorated)
    : decorated(decorated) {}
  
  DecoratedType decorated;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated.policyEnter(crAlgorithm);}
    
  const void* policyChoose(ChoosePtr choose)
    {return decorated.policyChoose(choose);}
    
  void policyReward(double reward)
    {decorated.policyReward(reward);}
    
  void policyLeave()
    {decorated.policyLeave();}
};

struct DynamicToStaticPolicy : public Policy<DynamicToStaticPolicy>
{
  DynamicToStaticPolicy(PolicyPtr policy) : policy(policy) {}
  
  PolicyPtr policy;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
    {policy->policyEnter(crAlgorithm);}
    
  const void* policyChoose(ChoosePtr choose)
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

#endif // !CRALGO_IMPL_POLICY_STATIC_H_

/*-----------------------------------------.---------------------------------.
| Filename: PolicyStaticToDynamic.hpp      | Policy Static to Dynamic        |
| Author  : Francis Maes                   |    Bridge                       |
| Started : 11/03/2009 20:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_STATIC_TO_DYNAMIC_H_
# define CRALGO_IMPL_POLICY_STATIC_TO_DYNAMIC_H_

# include "PolicyStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace cralgo {
namespace impl {

template<class ImplementationType>
class StaticToDynamicPolicy
  : public StaticToDynamicObject<ImplementationType, cralgo::Policy>
{
public:
  typedef StaticToDynamicObject<ImplementationType, cralgo::Policy> BaseClass;
  
  StaticToDynamicPolicy(const ImplementationType& impl)
    : BaseClass(impl) {}
    
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {BaseClass::impl.policyEnter(crAlgorithm);}
    
  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return BaseClass::impl.policyChoose(choose);}
    
  virtual void policyReward(double reward)
    {BaseClass::impl.policyReward(reward);}
    
  virtual void policyLeave()
    {BaseClass::impl.policyLeave();}

  virtual size_t getNumResults() const
    {return BaseClass::impl.getNumResults();}
    
  virtual ObjectPtr getResult(size_t i) const
    {return BaseClass::impl.getResult(i);}
};

template<class ExactType>
inline PolicyPtr staticToDynamic(const Policy<ExactType>& staticPolicy)
  {return PolicyPtr(new StaticToDynamicPolicy<ExactType>(static_cast<const ExactType& >(staticPolicy)));}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_STATIC_TO_DYNAMIC_H_

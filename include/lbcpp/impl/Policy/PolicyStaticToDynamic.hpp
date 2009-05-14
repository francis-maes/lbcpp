/*-----------------------------------------.---------------------------------.
| Filename: PolicyStaticToDynamic.hpp      | Policy Static to Dynamic        |
| Author  : Francis Maes                   |    Bridge                       |
| Started : 11/03/2009 20:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_STATIC_TO_DYNAMIC_H_
# define LBCPP_CORE_IMPL_POLICY_STATIC_TO_DYNAMIC_H_

# include "PolicyStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace lbcpp {
namespace impl {

STATIC_TO_DYNAMIC_CLASS(Policy, Object)
    
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
    
STATIC_TO_DYNAMIC_ENDCLASS(Policy);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_STATIC_TO_DYNAMIC_H_

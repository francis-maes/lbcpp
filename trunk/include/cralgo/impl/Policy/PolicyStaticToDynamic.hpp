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

namespace cralgo {
namespace impl {

// todo: move
template<class ImplementationType, class BaseClass>
class StaticToDynamicObject : public BaseClass
{
public:
  StaticToDynamicObject(const ImplementationType& impl)
    : impl(impl) {}

  virtual bool load(std::istream& istr)
    {return impl.load(istr);}
    
  virtual void save(std::ostream& ostr) const
    {impl.save(ostr);}

  virtual std::string toString() const
    {return impl.toString();}
    
protected:
  ImplementationType impl;
};

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
    
  virtual const void* policyChoose(ChoosePtr choose)
    {return BaseClass::impl.policyChoose(choose);}
    
  virtual void policyReward(double reward)
    {BaseClass::impl.policyReward(reward);}
    
  virtual void policyLeave()
    {BaseClass::impl.policyLeave();}
};

template<class ExactType>
inline PolicyPtr staticToDynamic(const Policy<ExactType>& staticPolicy)
  {return PolicyPtr(new StaticToDynamicPolicy<ExactType>(static_cast<const ExactType& >(staticPolicy)));}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_STATIC_TO_DYNAMIC_H_

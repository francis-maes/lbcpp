/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithm.hpp                | Static to Dynamic CR-algorithm  |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 04/02/2009 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_CRALGORITHM_HPP_
# define CRALGO_STATIC_CRALGORITHM_HPP_

# include "../../CRAlgorithm.h"
# include "CRAlgorithmScope.hpp"
# include "Choose.hpp"
# include "Callback.hpp"

namespace cralgo
{


template<class T_impl>
class StaticToDynamicCRAlgorithm
  : public StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithm, StaticToDynamicCRAlgorithm<T_impl> >
{
public:
  typedef StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithm, StaticToDynamicCRAlgorithm<T_impl> > BaseClassType;

  StaticToDynamicCRAlgorithm(T_impl* newImpl) : BaseClassType(newImpl) {}

  virtual std::string getName() const
    {return BaseClassType::getImplementation().getName();}

  virtual bool hasReturn() const
    {return BaseClassType::getImplementation().hasReturn();}

  virtual const void* getReturn() const
    {return BaseClassType::getImplementation().getReturn();}

  virtual void run(PolicyPtr policy, const void* choice)
  {
    T_impl& impl = BaseClassType::getImplementation();
    assert(impl.__state__ >= 0); // in order to run a policy from the initial state, use run(policy)
    PolicyToStaticCallback staticCallback(policy, CRAlgorithmPtr(this), choice);
    while (!stepImpl(staticCallback, staticCallback.getLastChoice()));      
  }
    
  virtual void run(PolicyPtr policy)
  {
    // todo: inlined version
    assert(BaseClassType::getImplementation().__state__ == -1); // in order to run a policy from a non-initial state, use run(policy, choice)
    PolicyToStaticCallback staticCallback(policy, CRAlgorithmPtr(this));
    while (!stepImpl(staticCallback, staticCallback.getLastChoice()));      
  }

  virtual bool step(Callback& callback, const void* choice)
  {
    DynamicToStaticCallback<T_impl> staticCallback(callback, CRAlgorithmPtr(this));
    return stepImpl(staticCallback, choice);
  }

  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL)
  {
    T_impl& impl = BaseClassType::getImplementation();
    assert(impl.__state__ == -1);
    return runUntilNextChoose(NULL, reward);
  }
  
  virtual ChoosePtr runUntilNextChoose(const void* choice, double* reward = NULL)
  {
    ConstructChooseStaticCallback<T_impl> callback(CRAlgorithmPtr(this));
    stepImpl(callback, choice);
    if (reward)
      *reward = callback.getCurrentReward();
    return callback.getCurrentChoose();
  }
  
private:
  template<class CallbackType>
  bool stepImpl(CallbackType& callback, const void* choice) // returns true if the cr-algorithm is finished
  {
    T_impl& impl = BaseClassType::getImplementation();
    int res = T_impl::step(impl, impl, callback, choice);
    return res == stateFinish || res == stateReturn;
  }
};

template<class CRAlgorithmType>
inline CRAlgorithmType& dynamicToStaticCRAlgorithm(CRAlgorithmPtr crAlgorithm)
{
  typedef StaticToDynamicCRAlgorithm<CRAlgorithmType> CRAlgorithmWrapperType;
  
  CRAlgorithmWrapperType* cralgo = dynamic_cast<CRAlgorithmWrapperType* >(crAlgorithm.get());
  assert(cralgo); // todo: error message
  return cralgo->getImplementation();
}

template<class CRAlgorithmType>
inline CRAlgorithmPtr staticToDynamicCRAlgorithm(CRAlgorithmType* newStaticCRAlgorithm)
{
  return CRAlgorithmPtr(new StaticToDynamicCRAlgorithm<CRAlgorithmType>(newStaticCRAlgorithm));
}

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_CRALGORITHM_HPP_

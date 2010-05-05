/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScope.hpp           | Static to Dynamic CR-algorithm  |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 04/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_CRALGORITHM_SCOPE_HPP_
# define LBCPP_STATIC_CRALGORITHM_SCOPE_HPP_

# include "../../CRAlgorithm/CRAlgorithmScope.h"

namespace lbcpp
{

enum
{
  stateBreak = 0,
  stateContinue,
  stateChoose,
  stateReturn,
  stateFinish,
};

template<class T_impl, class T_baseclass, class T_exact>
class StaticToDynamicCRAlgorithmScope_ : public T_baseclass
{
public:
  /*
  ** Constructor / Destructor
  */
  StaticToDynamicCRAlgorithmScope_(T_impl* newImpl)
    : impl(newImpl) {}
    
  virtual ~StaticToDynamicCRAlgorithmScope_()
    {delete impl;}
    
  /*
  ** Introspection
  */
  virtual size_t getNumVariables() const
    {return impl->getNumVariables();}
  
  virtual VariablePtr getVariable(size_t num) const
    {return Variable::createFromPointer(const_cast<void* >(impl->getVariablePointer(num)),
        impl->getVariableType(num), impl->getVariableName(num));}

  virtual VariablePtr getVariable(const String& name) const
  {
    size_t n = impl->getNumVariables();
    for (size_t i = 0; i < n; ++i)
      if (impl->getVariableName(i) == name)
        return getVariable(i);
    return VariablePtr();
  }
  
  virtual String getVariableType(size_t num) const
    {return impl->getVariableType(num);}
    
  virtual String getVariableName(size_t num) const
    {return impl->getVariableName(num);}

  virtual String getVariableValue(size_t num) const
    {return impl->getVariableValue(num);}

  virtual int getState() const
    {return impl->__state__;}

  virtual CRAlgorithmScopePtr getCurrentInnerScope()
    {return impl->getCurrentInnerScope();}

  /*
  ** Clone / assignment
  */
  virtual ObjectPtr clone() const
    {return new T_exact(new T_impl(*impl));}
    
  virtual void setScope(const CRAlgorithmScope& otherScope)
  {
    const T_exact* other = cast(otherScope);
    if (other)
    {
      delete impl;
      impl = new T_impl(*other->impl);
    }
  }

  virtual void swapScope(CRAlgorithmScope& otherScope)
  {
    T_exact* other = cast(otherScope);
    if (other)
    {
      T_impl* tmp = other->impl;
      other->impl = impl;
      impl = tmp;
    }
  }

  const T_impl& getImplementation() const
    {return *impl;}
  T_impl& getImplementation()
    {return *impl;}

private:
  T_impl* impl;
  
  static const T_exact* cast(const CRAlgorithmScope& c)
  {
    const T_exact* res = dynamic_cast<const T_exact* >(&c);
    jassert(res); // todo: error message
    return res;
  }

  static T_exact* cast(CRAlgorithmScope& c)
  {
    T_exact* res = dynamic_cast<T_exact* >(&c);
    jassert(res); // todo: error message
    return res;
  }
};

template<class T_impl>
class StaticToDynamicCRAlgorithmScope
  : public StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithmScope, StaticToDynamicCRAlgorithmScope<T_impl> >
{
public:
  typedef StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithmScope, StaticToDynamicCRAlgorithmScope<T_impl> > BaseClassType;
  
  StaticToDynamicCRAlgorithmScope(T_impl* newImpl)
    : BaseClassType(newImpl) {}
};

template<class T_impl>
inline CRAlgorithmScopePtr staticToDynamicCRAlgorithmScope(T_impl* impl)
  {jassert(impl); return CRAlgorithmScopePtr(new StaticToDynamicCRAlgorithmScope<T_impl>(new T_impl(*impl)));}

}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_CRALGORITHM_SCOPE_HPP_

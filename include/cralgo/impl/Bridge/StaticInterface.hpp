/*-----------------------------------------.---------------------------------.
| Filename: StaticInterface.h              | Specification of the static     |
| Author  : Francis Maes                   |   interface                     |
| Started : 01/03/2009 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_BRIDGE_STATIC_INTERFACE_HPP_
# define CRALGO_BRIDGE_STATIC_INTERFACE_HPP_

namespace cralgo
{

// todo: ranger
struct StaticCallback
{
  template<class ScopeType>
  void enterLocalScope(size_t scopeNumber, ScopeType& scope) {}
  
  void leaveLocalScope() {}

  template<class ContainerType, class ChooseType>
  void choose(const ContainerType& choices, const ChooseType& dummy) {}
  
  void reward(double r) {}
};

}; /* namespace cralgo */

#endif // !CRALGO_BRIDGE_STATIC_INTERFACE_HPP_


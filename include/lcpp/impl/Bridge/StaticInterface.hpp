/*-----------------------------------------.---------------------------------.
| Filename: StaticInterface.h              | Specification of the static     |
| Author  : Francis Maes                   |   interface                     |
| Started : 01/03/2009 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_BRIDGE_STATIC_INTERFACE_HPP_
# define LCPP_BRIDGE_STATIC_INTERFACE_HPP_

namespace lcpp
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

}; /* namespace lcpp */

#endif // !LCPP_BRIDGE_STATIC_INTERFACE_HPP_


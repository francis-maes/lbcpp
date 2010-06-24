/*-----------------------------------------.---------------------------------.
| Filename: TransferArchitecture.hpp       | Apply a transfer function       |
| Author  : Francis Maes                   |   on each input                 |
| Started : 11/03/2009 19:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_
# define LBCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lbcpp {
namespace impl {

// -> transform into VectorFunction ?
template<class DerivableFunction>
struct TransferArchitecture : public VectorArchitecture< TransferArchitecture<DerivableFunction> >
{
  TransferArchitecture(const DerivableFunction& function)
    : function(function) {}
  
  // todo: compute
  
  DerivableFunction function;
};

}; /* namespace impl */
}; /* namespace lbcpp */


#endif // !LBCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_

/*-----------------------------------------.---------------------------------.
| Filename: TransferArchitecture.hpp       | Apply a transfer function       |
| Author  : Francis Maes                   |   on each input                 |
| Started : 11/03/2009 19:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_
# define LCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lcpp {
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
}; /* namespace lcpp */


#endif // !LCPP_CORE_IMPL_FUNCTION_TRANSFER_ARCHITECTURE_H_

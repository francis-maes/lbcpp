/*-----------------------------------------.---------------------------------.
| Filename: VectorDerivableArchitecture.hpp| Parameterized functions         |
| Author  : Francis Maes                   |   f_theta : Phi -> R^o          |
| Started : 07/03/2009 15:48               |      theta = DenseVector        |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_
# define CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// todo: MultiLinearArchitecture

template<class DerivableFunction>
struct TransferArchitecture : public VectorArchitecture< TransferArchitecture<DerivableFunction> >
{
  TransferArchitecture(const DerivableFunction& function)
    : function(function) {}
  
  // todo: compute
  
  DerivableFunction function;
};

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_

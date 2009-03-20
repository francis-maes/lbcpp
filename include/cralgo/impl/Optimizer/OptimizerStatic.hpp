/*-----------------------------------------.---------------------------------.
| Filename: OptimizerStatic.hpp            | Optimizers Static Interface     |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_OPTIMIZER_STATIC_H_
# define CRALGO_IMPL_OPTIMIZER_STATIC_H_

# include "../../Optimizer.h"
# include "../Object.hpp"

namespace cralgo {
namespace impl {

template<class ExactType>
struct ScalarOptimizer : public Object<ExactType>
{
  typedef Object<ExactType> BaseClass;

  double optimizerStart(ScalarFunctionPtr function, double parameter)
    {return parameter;}

  double optimizerStep(ScalarFunctionPtr function, double parameter, double derivative, double value)
    {assert(false); return parameter;}
};

template<class ExactType>
struct VectorOptimizer : public Object<ExactType>
{
  typedef Object<ExactType> BaseClass;

  DenseVectorPtr optimizerStart(ScalarVectorFunctionPtr function, DenseVectorPtr parameters)
    {return parameters;}
    
  DenseVectorPtr optimizerStep(ScalarVectorFunctionPtr function, DenseVectorPtr parameters, const DenseVectorPtr gradient, double value)
    {assert(false); return parameters;}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_OPTIMIZER_STATIC_H_

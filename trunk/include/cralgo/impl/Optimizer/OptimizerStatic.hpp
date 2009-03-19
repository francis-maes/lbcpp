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
};

template<class ExactType>
struct VectorOptimizer : public Object<ExactType>
{
  typedef Object<ExactType> BaseClass;

  DenseVectorPtr optimizerStart(ScalarVectorFunctionPtr function, DenseVectorPtr parameters)
    {assert(false); return DenseVectorPtr();}
    
  DenseVectorPtr optimizerStep(ScalarVectorFunctionPtr function, DenseVectorPtr parameters, const DenseVectorPtr gradient, double value)
    {assert(false); return DenseVectorPtr();}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_OPTIMIZER_STATIC_H_

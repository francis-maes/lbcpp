/*-----------------------------------------.---------------------------------.
| Filename: OptimizerStaticToDynamic.hpp   | Optimizer Static to Dynamic     |
| Author  : Francis Maes                   |    Bridge                       |
| Started : 19/03/2009 21:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_OPTIMIZER_STATIC_TO_DYNAMIC_H_
# define CRALGO_IMPL_OPTIMIZER_STATIC_TO_DYNAMIC_H_

# include "OptimizerStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace cralgo {
namespace impl {

STATIC_TO_DYNAMIC_CLASS(ScalarOptimizer, Object)
    
  virtual bool optimize(ScalarFunctionPtr function, double& value, OptimizerTerminationTestPtr termination, ProgressCallback& callback)
  {
    return true;
  }

STATIC_TO_DYNAMIC_ENDCLASS(ScalarOptimizer);

STATIC_TO_DYNAMIC_CLASS(VectorOptimizer, Object)

  virtual bool optimize(ScalarVectorFunctionPtr function, DenseVectorPtr& parameters, OptimizerTerminationTestPtr termination, ProgressCallback& callback)
  {
    parameters = BaseClass::impl.optimizerStart(function, parameters);
    if (!parameters)
      return false;
    
    for (size_t i = 0; true; ++i)
    {
      double value;
      LazyVectorPtr gradient = new LazyVector(parameters->getDictionary());
      function->compute(parameters, &value, FeatureGeneratorPtr(), gradient);
      if (!callback.progressStep("Optimizing, f = " + cralgo::toString(value) + " norm = " + cralgo::toString(parameters->l2norm()), (double)i))
        return false;
      DenseVectorPtr denseGradient = gradient->toDenseVector();
      if (termination->isTerminated(value, parameters, denseGradient))
        break;
      parameters = BaseClass::impl.optimizerStep(function, parameters, denseGradient, value);
      if (!parameters)
        return false;
    }
    return true;
  }

STATIC_TO_DYNAMIC_ENDCLASS(VectorOptimizer);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_OPTIMIZER_STATIC_TO_DYNAMIC_H_

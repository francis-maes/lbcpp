/*-----------------------------------------.---------------------------------.
| Filename: OWLQNOptimizer.h               | Orthant-Wise Limited memory     |
| Author  : Francis Maes                   | Quasi Newton optimizer          |
| Started : 29/03/2009 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_OPTIMIZER_OWLQN_H_
# define CRALGO_OPTIMIZER_OWLQN_H_

/*
** from:
** "Scalable Training of L1-regularized Log-linear Models", Galen Andrew, ICML 2007
** www.machinelearning.org/proceedings/icml2007/papers/449.pdf
*/

# include <cralgo/Optimizer.h>
# include "QuasiNewtonMemory.h"
# include "BackTrackingLineSearch.h"

namespace cralgo
{

class LBFGSOptimizer : public VectorOptimizer
{
public:
  enum {quasiNewtonMemory = 10};

  LBFGSOptimizer(BackTrackingLineSearch* lineSearch = NULL)
    : memory(quasiNewtonMemory), lineSearch(lineSearch)
  {
    if (!lineSearch)
      lineSearch = new BackTrackingLineSearch();
  }
  
  virtual ~LBFGSOptimizer()
    {delete lineSearch;}
  
  virtual bool initialize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr parameters)
  {
    iteration = 0;
    return true;
  }
  
  virtual OptimizerState step(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, double value, FeatureGeneratorPtr gradient)
  {
    DenseVectorPtr direction = FeatureGenerator::multiplyByScalar(gradient, -1.0)->toDenseVector();
    memory.mapDirectionByInverseHessian(direction);
    FeatureGeneratorPtr newParameters, newGradient;
    if (!lineSearch->search(function, parameters, gradient, direction, value, iteration == 0, newParameters, newGradient))
      return optimizerError;
    // FIXME: le gradient est calculé deux fois
    memory.shift(parameters, gradient, newParameters, newGradient);
    ++iteration;
    return optimizerContinue;
  }

protected:
  size_t iteration;
  QuasiNewtonMemory memory;
  BackTrackingLineSearch* lineSearch;
};


}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_OWLQN_H_

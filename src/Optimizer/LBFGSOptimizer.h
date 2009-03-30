/*-----------------------------------------.---------------------------------.
| Filename: LBFGSOptimizer.h               | Limited Memory                  |
| Author  : Francis Maes                   | Quasi Newton optimizer          |
| Started : 29/03/2009 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_OPTIMIZER_LBFGS_H_
# define CRALGO_OPTIMIZER_LBFGS_H_

/*
** "On the limited memory BFGS method for large scale optimization",
** by D. Liu and J. Nocedal, Mathematical Programming B 45 (1989) 503-528
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
      this->lineSearch = new BackTrackingLineSearch();
  }
  
  virtual ~LBFGSOptimizer()
    {delete lineSearch;}
  
  virtual OptimizerState step()
  {
    DenseVectorPtr direction = new DenseVector(parameters->getDictionary());
    direction->addWeighted(gradient, -1.0);
    memory.mapDirectionByInverseHessian(direction);
    FeatureGeneratorPtr newParameters, newGradient;
    if (!lineSearch->search(function, parameters, gradient, direction, value, iteration == 0, newParameters, newGradient))
      return optimizerError;
    memory.shift(parameters, gradient, newParameters, newGradient);
    setParametersGradientAndValue(parameters, gradient, value);
    return optimizerContinue;
  }

protected:
  QuasiNewtonMemory memory;
  BackTrackingLineSearch* lineSearch;
};


}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_LBFGS_H_

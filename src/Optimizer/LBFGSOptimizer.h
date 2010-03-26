/*-----------------------------------------.---------------------------------.
| Filename: LBFGSOptimizer.h               | Limited Memory                  |
| Author  : Francis Maes                   | Quasi Newton optimizer          |
| Started : 29/03/2009 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_LBFGS_H_
# define LBCPP_OPTIMIZER_LBFGS_H_

/*
** "On the limited memory BFGS method for large scale optimization",
** by D. Liu and J. Nocedal, Mathematical Programming B 45 (1989) 503-528
*/
# include <lbcpp/Optimizer.h>
# include "QuasiNewtonMemory.h"
# include "BackTrackingLineSearch.h"

namespace lbcpp
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
  
  virtual String toString() const
    {return "LBFGSOptimizer";}

  virtual ~LBFGSOptimizer()
    {delete lineSearch;}
  
  virtual bool initialize()
  {
    memory.reset();
    return true;
  }

  virtual OptimizerState step()
  {
    jassert(isNumberValid(gradient->l2norm()));
    DenseVectorPtr direction = new DenseVector(parameters->getDictionary());
    direction->addWeighted(gradient, -1.0);
    jassert(isNumberValid(direction->l2norm()));
    memory.mapDirectionByInverseHessian(direction);
    jassert(isNumberValid(direction->l2norm()));
    FeatureGeneratorPtr newParameters, newGradient;
    if (!lineSearch->search(function, parameters, gradient, direction, value, iteration == 0, newParameters, newGradient))
      return optimizerError;
    memory.shift(parameters, gradient, newParameters, newGradient);
    setParametersGradientAndValue(parameters, gradient, value);
    return optimizerContinue;
  }

  virtual void save(OutputStream& ostr) const
    {}

  virtual bool load(InputStream& istr)
    {return true;}

protected:
  QuasiNewtonMemory memory;
  BackTrackingLineSearch* lineSearch;
};


}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_LBFGS_H_

/*-----------------------------------------.---------------------------------.
| Filename: BacktrackingLineSearchOpt...hpp| Backtracking line search        |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_
# define CRALGO_IMPL_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_

# include "OptimizerStatic.hpp"
# include "../../IterationFunction.h"

namespace cralgo {
namespace impl {

struct BackTrackingLineSearchOptimizer : public ScalarOptimizer<BackTrackingLineSearchOptimizer>
{
  double optimizerStart(ScalarFunctionPtr function, double parameter)
  {
    iteration = 0;
    return parameter;
  }

  double optimizerStep(ScalarFunctionPtr function, double parameter, double derivative, double value)
  {
    static const double c1 = 1e-4;
    
    if (iteration == 0)
    {
      initialValue = value;
      initialDerivative = derivative;
      if (initialDerivative >= 0)
      {
        Object::error("BackTrackingLineSearchOptimizer::optimizerStep",
          "Non-descent direction in backtracking line search: check your gradient");
        // FIXME: stop search with error state
        //checkDerivativeWrtDirection(parameters, gradient, direction);
        //return false;
      }
      alpha = 1.0;
      backoff = 0.5;
    }
    

    if (value <= initialValue + c1 * initialDerivative * alpha)
      ; //  FIXME: stop search
    
    parameter *= backoff;
    
    ++iteration;
    if (iteration >= 50)
    {
      Object::warning("BackTrackingLineSearchOptimizer::optimizerStep", "Max iterations in back tracking line search");
      std::cerr << "Origin Derivative = " << originDerivative << std::endl;
//      std::cerr << "Parameters = " << (const char* )parameters->toString() << std::endl;
//      std::cerr << "Gradient = " << (const char* )gradient->toString() << std::endl;
//      std::cerr << "Direction = " << (const char* )direction->toString() << std::endl;
    }
    
    return parameter;
  }
  
private:
  double initialDerivative;
  double initialValue;
  double alpha, backoff;
  size_t iteration;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_

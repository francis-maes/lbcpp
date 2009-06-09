/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Optimizers code                 |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Optimizer.h>
#include "Optimizer/RPropOptimizer.h"
#include "Optimizer/GradientDescentOptimizer.h"
#include "Optimizer/LBFGSOptimizer.h"
using namespace lbcpp;

bool VectorOptimizer::optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& params, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress)
{
  assert(params);
  this->function = function;
  setParameters(params);
  if (!initialize())
    return false;
  
  stoppingCriterion->reset();
  for (iteration = 0; true; ++iteration)
  {
    if (progress && !progress->progressStep("Optimizing, f = " + lbcpp::toString(value) + " norm = " + lbcpp::toString(parameters->l2norm()), (double)iteration))
      return false;
    if (stoppingCriterion->isTerminated(value, parameters, gradient))
      break;
    OptimizerState state = step();
    if (state == optimizerError)
      return false;
    else if (state == optimizerDone)
      break;
    if (!parameters)
    {
      error("VectorOptimizer::optimize", "null parameters");
      return false;
    }
  }
  params = parameters;
  return true;
}

VectorOptimizerPtr lbcpp::rpropOptimizer()
  {return new RPropOptimizer();}

VectorOptimizerPtr lbcpp::gradientDescentOptimizer(IterationFunctionPtr stepSize)
  {return new GradientDescentOptimizer(stepSize);}

VectorOptimizerPtr lbcpp::lbfgsOptimizer()
  {return new LBFGSOptimizer();}

/*
** Serializable classes declaration
*/
void declareOptimizers()
{
  LBCPP_DECLARE_CLASS(RPropOptimizer);
  LBCPP_DECLARE_CLASS(GradientDescentOptimizer);
  LBCPP_DECLARE_CLASS(LBFGSOptimizer);
}

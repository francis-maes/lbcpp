/*-----------------------------------------.---------------------------------.
| Filename: MaxIterationsStoppingCriterion.h| Max Iterations Stopping        |
| Author  : Francis Maes                   |  Criterion                      |
| Started : 25/08/2010 22:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_H_
# define LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_H_

# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class MaxIterationsStoppingCriterion : public StoppingCriterion
{
public:
  MaxIterationsStoppingCriterion(size_t maxIterations = 0)
    : iterations(0), maxIterations(maxIterations) {}

  virtual String toString() const
    {return "MaxIterations(" + String((int)maxIterations) + ")";}

  virtual void reset()
    {iterations = 0;}

  virtual bool shouldStop(double)
    {jassert(maxIterations); ++iterations; return iterations >= maxIterations;}

private:
  friend class MaxIterationsStoppingCriterionClass;

  size_t iterations, maxIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_H_

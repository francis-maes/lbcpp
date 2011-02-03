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

class IsAboveValueStoppingCriterion : public StoppingCriterion
{
public:
  IsAboveValueStoppingCriterion(double referenceValue = 0.0)
    : referenceValue(referenceValue) {}

  virtual String toString() const
    {return T("IsAboveValue(") + String(referenceValue) + T(")");}

  virtual void reset()
    {}

  virtual bool shouldStop(double value)
    {return value >= referenceValue;}

private:
  friend class IsAboveValueStoppingCriterionClass;

  double referenceValue;
};

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

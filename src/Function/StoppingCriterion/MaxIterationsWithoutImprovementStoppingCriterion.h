/*-----------------------------------------.---------------------------------.
| Filename: MaxIterationsWihoutImprovme...h| Max Iterations without          |
| Author  : Francis Maes                   |  improvements Stopping Criterion|
| Started : 25/08/2010 22:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_WITHOUT_IMPROVMENT_H_
# define LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_WITHOUT_IMPROVMENT_H_

# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class MaxIterationsWithoutImprovementStoppingCriterion : public StoppingCriterion
{
public:
  MaxIterationsWithoutImprovementStoppingCriterion(size_t maxIterationsWithoutImprovement = 5)
    : maxIterationsWithoutImprovement(maxIterationsWithoutImprovement), numIterationsWithoutImprovement(0), bestValue(-DBL_MAX)
    {reset();}

  virtual String toString() const
    {return "MaxIterationsWithoutImprovement(" + String((int)maxIterationsWithoutImprovement) + ")";}

  virtual void reset()
    {numIterationsWithoutImprovement = 0; bestValue = -DBL_MAX;}

  virtual bool shouldStop(double value)
  {
    static const double epsilon = 1e-10;
    if (value > bestValue + epsilon) // reject too small improvements
    {
      bestValue = value;
      numIterationsWithoutImprovement = 0;
    }
    else
    {
      ++numIterationsWithoutImprovement;
      if (numIterationsWithoutImprovement > maxIterationsWithoutImprovement)
        return true;
    }
    return false;
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    ReferenceCountedObjectPtr<MaxIterationsWithoutImprovementStoppingCriterion> res 
      = new MaxIterationsWithoutImprovementStoppingCriterion(maxIterationsWithoutImprovement);
    res->numIterationsWithoutImprovement = numIterationsWithoutImprovement;
    res->bestValue = bestValue;
    return res;
  }

private:
  friend class MaxIterationsWithoutImprovementStoppingCriterionClass;

  size_t maxIterationsWithoutImprovement;
  size_t numIterationsWithoutImprovement;
  double bestValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_STOPPING_CRITERION_MAX_ITERATIONS_WITHOUT_IMPROVMENT_H_

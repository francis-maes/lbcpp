/*-----------------------------------------.---------------------------------.
| Filename: AverageImprovmentStoppingCr..h | Average Improvement Stopping    |
| Author  : Francis Maes                   |  Criterion                      |
| Started : 25/08/2010 22:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_STOPPING_CRITERION_AVERAGE_IMPROVEMENT_H_
# define LBCPP_FUNCTION_STOPPING_CRITERION_AVERAGE_IMPROVEMENT_H_

# include <lbcpp/Function/StoppingCriterion.h>
# include <deque>
# include <cfloat>

namespace lbcpp
{

class AverageImprovementStoppingCriterion : public StoppingCriterion
{
public:
  AverageImprovementStoppingCriterion(double tolerance = 0.001, bool relativeImprovement = false)
    : tolerance(tolerance), relativeImprovement(relativeImprovement) {}

  virtual void reset()
    {prevs.clear();}

  virtual bool shouldStop(double value)
  {
    if (prevs.size())
    {
      double prevVal = prevs.front();
      /*if (energy > prevVal)
        prevs.clear();
      else */if (prevs.size() >= 5)
      {
        double averageImprovement = (prevVal - value) / prevs.size();
        if (relativeImprovement)
        {
          double relAvgImpr = value ? averageImprovement / fabs(value) : 0;
  //        std::cout << "Av-Improvment: " << averageImprovement << " RealImprovment: " << relAvgImpr << " tol = " << tolerance << std::endl;
          if ((averageImprovement >= 0 && averageImprovement < DBL_EPSILON) || (relAvgImpr >= 0 && relAvgImpr <= tolerance))
            return true;
        }
        else if (averageImprovement >= 0 && averageImprovement <= tolerance)
          return true;
        prevs.pop_front();
      }
    }
    prevs.push_back(value);
    return false;
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    ReferenceCountedObjectPtr<AverageImprovementStoppingCriterion> res 
      = new AverageImprovementStoppingCriterion(tolerance, relativeImprovement);
    res->prevs = prevs;
    return res;
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class AverageImprovementStoppingCriterionClass;

  double tolerance;
  bool relativeImprovement;
  std::deque<double> prevs;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_STOPPING_CRITERION_AVERAGE_IMPROVEMENT_H_

/*----------------------------------------.-----------------------------.
 | Filename: GreedySelectionCriterion.h   | Greedy Selection Criterion  |
 | Author  : Denny Verbeeck               |                             |
 | Started : 27/02/2013 15:00             |                             |
 `----------------------------------------/                             |
                        |                                               |
                        `----------------------------------------------*/

#ifndef ML_GREEDY_SELECTION_CRITERION_H_
# define ML_GREEDY_SELECTION_CRITERION_H_

# include <ml/SelectionCriterion.h>
# include <ml/RandomVariable.h>
# include <ml/Problem.h>

namespace lbcpp
{

/** Class for greedy selection.
 *  This class's evaluate() function just returns the model prediction.
 */

class GreedySelectionCriterion : public SelectionCriterion
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const
    {return originalProblem->getObjective(0)->getObjectiveRange(worst, best);}
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    if (object.isInstanceOf<Double>() || object.isInstanceOf<ScalarVariableMeanAndVariance>())
      return object->toDouble();
    else
    {
      jassertfalse; // unsupported prediction type
      return 0.0;
    }
  }
};

}; /* namespace lbcpp */

#endif // !ML_GREEDY_SELECTION_CRITERION_H_

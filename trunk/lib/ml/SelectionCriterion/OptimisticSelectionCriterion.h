/*----------------------------------------.-----------------------------.
 | Filename: OptimisticSelectionCriterion.h   | Optimistic Selection Criterion  |
 | Author  : Denny Verbeeck               |                             |
 | Started : 27/02/2013 15:00             |                             |
 `----------------------------------------/                             |
 |                                               |
 `----------------------------------------------*/

#ifndef ML_OPTIMISTIC_SELECTION_CRITERION_H_
# define ML_OPTIMISTIC_SELECTION_CRITERION_H_

# include <ml/SelectionCriterion.h>
# include <ml/RandomVariable.h>
# include <ml/Problem.h>

namespace lbcpp
{

/** Class for optimistic selection.
 *  Optimistic selection relies not on the model prediction \f$\hat{y}\f$,
 *  but on the optimistic prediction \f$\hat{y} + k \cdot s^2\f$, where \f$k\f$
 *  is the level of optimism, and \f$s^2\f$ is the standard deviation on the
 *  prediction of the model. As a result, this selection criterion can only be
 *  used with models that provide a prediction as well as a standard deviation
 *  on that prediction.
 */

class OptimisticSelectionCriterion : public SelectionCriterion
{
public:
  /** Constructor
   *  \param optimism The level of optimism. This should be a positive double value.
   *                  The class checks for minimization or maximization internally.
   */
  OptimisticSelectionCriterion(double optimism = 1.0) : optimism(optimism)
    {jassert(optimism >= 0);}
  
  virtual void getObjectiveRange(double& worst, double& best) const
    {return originalProblem->getObjective(0)->getObjectiveRange(worst, best);}
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    ScalarVariableMeanAndVariancePtr pred = object.staticCast<ScalarVariableMeanAndVariance>();
    return pred->getMean() + (isMinimization() ? -1.0 : 1.0) * optimism * pred->getStandardDeviation();
  }
  
protected:
  friend class OptimisticSelectionCriterionClass;
  
  double optimism;       /*< The level of optimism. */
};
  
}; /* namespace lbcpp */

#endif // !ML_OPTIMISTIC_SELECTION_CRITERION_H_

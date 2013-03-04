/*-----------------------------------------.---------------------------------.
 | Filename: SelectionCriterion.h           | Selection Criterion             |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 27/02/2013 14:22               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SELECTION_CRITERION_H_
# define ML_SELECTION_CRITERION_H_

# include "predeclarations.h"
# include "VariableEncoder.h"
# include "Objective.h"

namespace lbcpp
{

/** Base class for selection criteria.
 *  Given a model prediction, concrete subclasses should be able to compute a
 *  heuristic for the given object. Optimizers can use this heuristic
 *  to select values given the model and type of heuristic.
 */
class SelectionCriterion : public Objective
{
public:
  virtual void initialize(ProblemPtr originalProblem)
  {
    this->originalProblem = originalProblem;
  }
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const = 0;
  
protected:
  friend class SelectionCriterionClass;
  
  ProblemPtr originalProblem;
  
};

extern SelectionCriterionPtr greedySelectionCriterion();
extern SelectionCriterionPtr optimisticSelectionCriterion(double optimism);
extern SelectionCriterionPtr probabilityOfImprovementSelectionCriterion(FitnessPtr& bestFitness);
extern SelectionCriterionPtr expectedImprovementSelectionCriterion(FitnessPtr& bestFitness);

}; /* namespace lbcpp */

#endif // !ML_SELECTION_CRITERION_H_

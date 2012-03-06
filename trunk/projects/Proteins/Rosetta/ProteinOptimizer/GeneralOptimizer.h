/*-----------------------------------------.---------------------------------.
| Filename: GeneralOptimizer.h             | GeneralOptimizer                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 1, 2012  9:20:54 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_

#include "precompiled.h"

namespace lbcpp
{

/*
 * Predeclarations
 */
class OptimizationProblemState;
typedef ReferenceCountedObjectPtr<OptimizationProblemState> OptimizationProblemStatePtr;

class GeneralOptimizerStoppingCriterion;
typedef ReferenceCountedObjectPtr<GeneralOptimizerStoppingCriterion> GeneralOptimizerStoppingCriterionPtr;

/*
 * Declarations
 */
class OptimizationProblemState : public Object
{
public:
  virtual double getObjective(ExecutionContext& context) const = 0;
  virtual Variable getInternalState() const = 0;

  virtual double isBetterThan(ExecutionContext& context, const OptimizationProblemStatePtr& candidate) const
    {return getObjective(context) < candidate->getObjective(context);}

protected:
  friend class OptimizationProblemStateClass;
};

class OptimizationProblemStateModifier : public Object
{
public:
  virtual OptimizationProblemStatePtr applyTo(ExecutionContext& context, const OptimizationProblemStatePtr& state) const = 0;

protected:
  friend class OptimizationProblemStateModifierClass;
};

typedef ReferenceCountedObjectPtr<OptimizationProblemStateModifier> OptimizationProblemStateModifierPtr;

class GeneralOptimizer : public Object
{
public:
  GeneralOptimizer(const OptimizationProblemStatePtr& initialState,
                   const OptimizationProblemStateModifierPtr& modifier,
                   const GeneralOptimizerStoppingCriterionPtr& stoppingCriterion)
    : iteration(0),
      initialState(initialState),
      bestState(initialState),
      modifier(modifier),
      stoppingCriterion(stoppingCriterion) {}

  /*
   * Solution
   */
  OptimizationProblemStatePtr getBestState()
    {return bestState;}

  /*
   * Optimization
   */
  virtual void optimize(ExecutionContext& context) = 0;

protected:
  friend class GeneralOptimizerClass;

  size_t iteration;
  OptimizationProblemStatePtr initialState;
  OptimizationProblemStatePtr bestState;
  OptimizationProblemStateModifierPtr modifier;
  GeneralOptimizerStoppingCriterionPtr stoppingCriterion;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizer> GeneralOptimizerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_


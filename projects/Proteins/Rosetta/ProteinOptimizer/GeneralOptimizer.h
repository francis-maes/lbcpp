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

class GeneralOptimizerParameters : public Object
{
public:
  virtual VariableVectorPtr getParameters(ExecutionContext& context, const Variable& input) const = 0;

protected:
  friend class GeneralOptimizerParametersClass;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizerParameters> GeneralOptimizerParametersPtr;

class GeneralOptimizerStoppingCriterion : public Object
{
public:
  virtual bool performNext(ExecutionContext& context, size_t iteration, const OptimizationProblemStatePtr& state, const OptimizationProblemStatePtr& bestState) const = 0;

protected:
  friend class GeneralOptimizerStoppingCriterionClass;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizerStoppingCriterion> GeneralOptimizerStoppingCriterionPtr;

class GeneralOptimizer : public Object
{
public:
  GeneralOptimizer() : iteration(0) {}
  GeneralOptimizer(const OptimizationProblemStatePtr& initialState,
                   const OptimizationProblemStateModifierPtr& modifier,
                   const GeneralOptimizerStoppingCriterionPtr& stoppingCriterion,
                   const GeneralOptimizerParametersPtr& parameters)
    : iteration(0),
      initialState(initialState),
      bestState(initialState),
      modifier(modifier),
      stoppingCriterion(stoppingCriterion),
      parameters(parameters) {}

  /*
   * Solution
   */
  OptimizationProblemStatePtr getBestState() const
    {return bestState;}

  /*
   * Optimization
   */
  virtual DenseDoubleVectorPtr optimize(ExecutionContext& context) = 0;

protected:
  friend class GeneralOptimizerClass;

  size_t iteration;
  OptimizationProblemStatePtr initialState;
  OptimizationProblemStatePtr bestState;
  OptimizationProblemStateModifierPtr modifier;
  GeneralOptimizerStoppingCriterionPtr stoppingCriterion;
  GeneralOptimizerParametersPtr parameters;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizer> GeneralOptimizerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_


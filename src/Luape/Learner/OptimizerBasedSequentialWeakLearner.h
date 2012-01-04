/*-----------------------------------------.---------------------------------.
| Filename: OptimizerBasedSequentialWeakLearner.h | Optimizer based          |
| Author  : Francis Maes                   | decision-process weak learner   |
| Started : 04/01/2012 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_WEAK_OPTIMIZER_BASED_SEQUENTIAL_H_
# define LBCPP_LUAPE_LEARNER_WEAK_OPTIMIZER_BASED_SEQUENTIAL_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include "../NodeBuilder/NodeBuilderDecisionProblem.h"

namespace lbcpp
{

class OptimizerBasedSequentialWeakLearner : public LuapeLearner
{
public:
  OptimizerBasedSequentialWeakLearner(OptimizerPtr optimizer, size_t complexity)
    : optimizer(optimizer), complexity(complexity) {}
  OptimizerBasedSequentialWeakLearner() : complexity(0) {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, complexity);
    OptimizationProblemPtr optimizationProblem = new OptimizationProblem(new Objective(refCountedPointerFromThis(this), problem, examples));
    optimizationProblem->setMaximisationProblem(true);
    optimizationProblem->setInitialState(new LuapeGraphBuilderState(problem, typeSearchSpace));

    OptimizerStatePtr state = optimizer->optimize(context, optimizationProblem);
    LuapeGraphBuilderStatePtr finalState = state->getBestSolution().getObjectAndCast<LuapeGraphBuilderState>();
    if (!finalState || finalState->getStackSize() != 1)
      return LuapeNodePtr();
    bestObjectiveValue = state->getBestScore();
    return finalState->getStackElement(0);
  }
 
protected:
  friend class OptimizerBasedSequentialWeakLearnerClass;

  OptimizerPtr optimizer; // example: new NestedMonteCarloOptimizer(level, iterations)
  size_t complexity;

  struct Objective : public SimpleUnaryFunction
  {
    Objective(LuapeLearnerPtr weakLearner,
              LuapeInferencePtr problem,
              const IndexSetPtr& examples)
       : SimpleUnaryFunction(luapeGraphBuilderStateClass, doubleType),
        weakLearner(weakLearner), problem(problem), examples(examples) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      const LuapeGraphBuilderStatePtr& builder = input.getObjectAndCast<LuapeGraphBuilderState>();
      if (builder->getStackSize() != 1)
        return 0.0;
      LuapeNodePtr node = builder->getStackElement(0);
      double res = weakLearner->getObjective()->computeObjectiveWithEventualStump(context, problem, node, examples);
      builder->setStackElement(0, node); // node may have been replaced by a stump of itself
      context.informationCallback(node->toShortString() + T(" ==> ") + String(res));
      return res;
    }

  private:
    LuapeLearnerPtr weakLearner;
    LuapeInferencePtr problem;
    IndexSetPtr examples;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_NESTED_MC_WEAK_H_

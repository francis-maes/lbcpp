/*-----------------------------------------.---------------------------------.
| Filename: NestedMCWeakLearner.h          | Nested Monte Carlo Weak Learner |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2011 09:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_NESTED_MC_WEAK_H_
# define LBCPP_LUAPE_LEARNER_NESTED_MC_WEAK_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include "../../../src/Luape/NodeBuilder/NodeBuilderDecisionProblem.h"
# include "../Core/NestedMonteCarloOptimizer.h"

namespace lbcpp
{

class DPOptimizerBasedWeakLearner : public WeakLearner
{
public:
  DPOptimizerBasedWeakLearner(OptimizerPtr optimizer, size_t maxDepth)
    : optimizer(optimizer), maxDepth(maxDepth) {}
  DPOptimizerBasedWeakLearner() : maxDepth(0) {}

  struct Objective : public SimpleUnaryFunction
  {
    Objective(WeakLearnerPtr weakLearner,
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
      double res = weakLearner->computeWeakObjectiveWithEventualStump(context, problem, node, examples);
      builder->setStackElement(0, node); // node may have been replaced by a stump of itself
      context.informationCallback(node->toShortString() + T(" ==> ") + String(res));
      return res;
    }

  private:
    WeakLearnerPtr weakLearner;
    LuapeInferencePtr problem;
    IndexSetPtr examples;
  };

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxDepth);
    typeSearchSpace->pruneStates(context);
    return true;
  }

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    OptimizationProblemPtr optimizationProblem = new OptimizationProblem(new Objective(refCountedPointerFromThis(this), problem, examples));
    optimizationProblem->setMaximisationProblem(true);
    optimizationProblem->setInitialState(new LuapeGraphBuilderState(problem, typeSearchSpace));

    OptimizerStatePtr state = optimizer->optimize(context, optimizationProblem);
    LuapeGraphBuilderStatePtr finalState = state->getBestSolution().getObjectAndCast<LuapeGraphBuilderState>();
    if (!finalState || finalState->getStackSize() != 1)
      return LuapeNodePtr();
    bestWeakObjectiveValue = state->getBestScore();
    return finalState->getStackElement(0);
  }
 
protected:
  friend class DPOptimizerBasedWeakLearnerClass;

  OptimizerPtr optimizer;
  size_t maxDepth;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
};

class NestedMCWeakLearner : public DPOptimizerBasedWeakLearner
{
public:
  NestedMCWeakLearner(size_t level, size_t iterations, size_t maxDepth)
    : DPOptimizerBasedWeakLearner(new NestedMonteCarloOptimizer(level, iterations), maxDepth) {}
  NestedMCWeakLearner() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_NESTED_MC_WEAK_H_

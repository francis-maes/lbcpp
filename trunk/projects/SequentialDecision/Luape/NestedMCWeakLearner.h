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
# include "LuapeGraphBuilder.h"
# include "../Core/NestedMonteCarloOptimizer.h"

namespace lbcpp
{

class DPOptimizerBasedWeakLearner : public BoostingWeakLearner
{
public:
  DPOptimizerBasedWeakLearner(OptimizerPtr optimizer, size_t maxDepth)
    : optimizer(optimizer), maxDepth(maxDepth) {}
  DPOptimizerBasedWeakLearner() : maxDepth(0) {}

  struct Objective : public SimpleUnaryFunction
  {
    Objective(BoostingWeakLearnerPtr weakLearner,
              BoostingLearnerPtr strongLearner,
              const std::vector<size_t>& examples)
       : SimpleUnaryFunction(luapeGraphBuilderStateClass, doubleType),
        weakLearner(weakLearner), strongLearner(strongLearner), examples(examples) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      const LuapeGraphBuilderStatePtr& builder = input.getObjectAndCast<LuapeGraphBuilderState>();
      if (builder->getStackSize() != 1)
        return 0.0;
      LuapeNodePtr node = builder->getStackElement(0);
      double res = weakLearner->computeWeakObjectiveWithEventualStump(context, strongLearner, node, examples);
      builder->setStackElement(0, node); // node may have been replaced by a stump of itself
      context.informationCallback(node->toShortString() + T(" ==> ") + String(res));
      return res;
    }

  private:
    BoostingWeakLearnerPtr weakLearner;
    BoostingLearnerPtr strongLearner;
    std::vector<size_t> examples;
  };

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxDepth);
    typeSearchSpace->pruneStates(context);
    return true;
  }

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& strongLearner, const std::vector<size_t>& examples, double& weakObjective) const
  {
    OptimizationProblemPtr problem = new OptimizationProblem(new Objective(refCountedPointerFromThis(this), strongLearner, examples));
    problem->setMaximisationProblem(true);
    problem->setInitialState(new LuapeGraphBuilderState(strongLearner->getFunction(), typeSearchSpace));

    OptimizerStatePtr state = optimizer->optimize(context, problem);
    LuapeGraphBuilderStatePtr finalState = state->getBestSolution().getObjectAndCast<LuapeGraphBuilderState>();
    if (!finalState || finalState->getStackSize() != 1)
      return LuapeNodePtr();
    LuapeNodePtr weakNode = finalState->getStackElement(0);
    weakObjective = state->getBestScore();
    return makeContribution(context, strongLearner, weakNode, examples);
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

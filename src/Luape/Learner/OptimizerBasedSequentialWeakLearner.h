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
  OptimizerBasedSequentialWeakLearner(OptimizerPtr optimizer, size_t complexity, bool useRandomSplit)
    : optimizer(optimizer), complexity(complexity), useRandomSplit(useRandomSplit), totalNumCalls(0), totalNumUniqueCalls(0) {}
  OptimizerBasedSequentialWeakLearner() : complexity(0), totalNumCalls(0), totalNumUniqueCalls(0) {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, complexity, verbose);
    ObjectivePtr objective = new Objective(refCountedPointerFromThis(this), problem, examples, useRandomSplit);
    OptimizationProblemPtr optimizationProblem = new OptimizationProblem(objective);
    optimizationProblem->setMaximisationProblem(true);
    optimizationProblem->setInitialState(new LuapeGraphBuilderState(problem, typeSearchSpace));

    OptimizerStatePtr state = optimizer->optimize(context, optimizationProblem);

    if (verbose)
    {
      context.resultCallback(T("numObjectiveCalls"), objective->getNumCalls());
      context.resultCallback(T("numObjectiveInvalidCalls"), objective->getNumInvalidCalls());
      context.resultCallback(T("numObjectiveUniqueCalls"), objective->getNumUniqueCalls());
    }
    totalNumCalls += objective->getNumCalls();
    totalNumUniqueCalls += objective->getNumUniqueCalls();

    LuapeGraphBuilderStatePtr finalState = state->getBestSolution().getObjectAndCast<LuapeGraphBuilderState>();
    if (!finalState || finalState->getStackSize() != 1)
      return LuapeNodePtr();

    bestObjectiveValue = state->getBestScore();
    return finalState->getStackElement(0);
  }
  
  size_t getTotalNumCalls() const
    {return totalNumCalls;}

  size_t getTotalNumUniqueCalls() const
    {return totalNumUniqueCalls;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    LuapeLearner::clone(context, target);
    target.staticCast<OptimizerBasedSequentialWeakLearner>()->optimizer = optimizer->cloneAndCast<Optimizer>();
  }

protected:
  friend class OptimizerBasedSequentialWeakLearnerClass;

  OptimizerPtr optimizer; // example: new NestedMonteCarloOptimizer(level, iterations)
  size_t complexity;
  bool useRandomSplit;
  size_t totalNumCalls;
  size_t totalNumUniqueCalls;

  struct Objective : public SimpleUnaryFunction
  {
    Objective(LuapeLearnerPtr weakLearner,
              LuapeInferencePtr problem,
              const IndexSetPtr& examples,
              bool useRandomSplit)
       : SimpleUnaryFunction(luapeGraphBuilderStateClass, doubleType),
        weakLearner(weakLearner), problem(problem), examples(examples), useRandomSplit(useRandomSplit),
        numCalls(0), numInvalidCalls(0), numUniqueCalls(0) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      Objective& pthis = *const_cast<Objective* >(this);

      ++pthis.numCalls;

      const LuapeGraphBuilderStatePtr& builder = input.getObjectAndCast<LuapeGraphBuilderState>();
      if (builder->getStackSize() != 1)
      {
        ++pthis.numInvalidCalls;
        return -DBL_MAX;
      }

      LuapeNodePtr node = builder->getStackElement(0);
      CacheMap::const_iterator it = cache.find(node);
      double res;
      if (it != cache.end())
      {
        node = it->second.first;
        res = it->second.second;
      }
      else
      {
        ++pthis.numUniqueCalls;
        if (node->getType() != booleanType && useRandomSplit)
          node = makeRandomSplit(context, node);
        res = weakLearner->getObjective()->computeObjectiveWithEventualStump(context, problem, node, examples);
        pthis.cache[builder->getStackElement(0)] = std::make_pair(node, res);
      }
      builder->setStackElement(0, node); // node may have been replaced by a stump of itself
      //context.informationCallback(node->toShortString() + T(" ==> ") + String(res));
      return res;
    }
    
    size_t getNumCalls() const
      {return numCalls;}

    size_t getNumInvalidCalls() const
      {return numInvalidCalls;}

    size_t getNumUniqueCalls() const
      {return numUniqueCalls;}

  private:
    LuapeLearnerPtr weakLearner;
    LuapeInferencePtr problem;
    IndexSetPtr examples;
    bool useRandomSplit;

    typedef std::map<LuapeNodePtr, std::pair<LuapeNodePtr, double> > CacheMap;
    CacheMap cache; // node -> (booleanized node, score)

    size_t numCalls;
    size_t numInvalidCalls;
    size_t numUniqueCalls;
    
    LuapeNodePtr makeRandomSplit(ExecutionContext& context, LuapeNodePtr node) const
    {
      LuapeSampleVectorPtr samples = problem->getTrainingCache()->getSamples(context, node, examples);
      double minimumValue = DBL_MAX;
      double maximumValue = -DBL_MAX;
      bool isInteger = samples->getElementsType()->inheritsFrom(integerType);
      for (LuapeSampleVector::const_iterator it = samples->begin(); it != samples->end(); ++it)
      {
        double value = isInteger ? (double)it.getRawInteger() : it.getRawDouble();
        if (value < minimumValue)
          minimumValue = value;
        if (value > maximumValue)
          maximumValue = value;
      }
      
      double threshold = context.getRandomGenerator()->sampleDouble(minimumValue, maximumValue);
      //context.informationCallback(T("min = ") + String(minimumValue) + T(" max = ") + String(maximumValue) + T(" threshold = ") + String(threshold));
      return new LuapeFunctionNode(stumpLuapeFunction(threshold), node);
    }
  };

  typedef ReferenceCountedObjectPtr<Objective> ObjectivePtr;
};

typedef ReferenceCountedObjectPtr<OptimizerBasedSequentialWeakLearner> OptimizerBasedSequentialWeakLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_NESTED_MC_WEAK_H_

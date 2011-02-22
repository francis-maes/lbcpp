/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionSandBox.h    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

# include "../Problem/LinearPointPhysicProblem.h"
# include "../Core/SearchSpace.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// SearchNode -> Scalar
class LearnableSearchHeuristic : public CompositeFunction
{
public:
  virtual FunctionPtr createPerceptionFunction() const = 0;
  virtual FunctionPtr createScoringFunction() const = 0;

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchSpaceNodeClass, T("node"));
    size_t perception = builder.addFunction(createPerceptionFunction(), node);
    size_t supervision = builder.addConstant(false);
    builder.addFunction(createScoringFunction(), perception, supervision);
  }

protected:
  FunctionPtr featuresFunction;  // SearchNode -> Features
  FunctionPtr scoreFunction;     // Features -> Score
};

class GenericClosedSearchSpaceNodeFeaturesFunction : public CompositeFunction
{
public:
  GenericClosedSearchSpaceNodeFeaturesFunction(size_t maxDepth = 1000, double maxReward = 1.0, double maxReturn = 10.0)
    : maxDepth(maxDepth), maxReward(maxReward), maxReturn(maxReturn) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchSpaceNodeClass, T("node"));
    size_t depth = builder.addFunction(getVariableFunction(T("depth")), node);
    size_t reward = builder.addFunction(getVariableFunction(T("reward")), node);
    size_t currentReturn = builder.addFunction(getVariableFunction(T("currentReturn")), node);
    //size_t action = builder.addFunction(getVariableFunction(T("previousAction")), node);

    builder.startSelection();

      // max depth = 1000, max reward = 100
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10((double)maxDepth), 20), depth);
      builder.addFunction(softDiscretizedNumberFeatureGenerator(0.0, maxReward, 20), reward);
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10(maxReturn), 20), currentReturn);

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

protected:
  size_t maxDepth;
  double maxReward;
  double maxReturn;
};

class LinearLearnableSearchHeuristic : public LearnableSearchHeuristic
{
public:
  virtual FunctionPtr createPerceptionFunction() const
    {return new GenericClosedSearchSpaceNodeFeaturesFunction();}

  virtual FunctionPtr createScoringFunction() const
    {return linearLearnableFunction();}
};

extern OnlineLearnerPtr lookAheadTreeSearchOnlineLearner();

// State -> Double
class LookAheadTreeSearchFunction : public SimpleUnaryFunction
{
public:
  LookAheadTreeSearchFunction(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, double discount, size_t maxSearchNodes)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), heuristic(heuristic), discount(discount), maxSearchNodes(maxSearchNodes)
  {
    const size_t maxIterations = 100;
    setBatchLearner(stochasticBatchLearner(maxIterations, true));
    setOnlineLearner(lookAheadTreeSearchOnlineLearner());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& initialState) const
  {
    SortedSearchSpacePtr searchSpace = new SortedSearchSpace(problem, heuristic, discount, initialState);
    searchSpace->reserveNodes(2 * maxSearchNodes);

    double highestReturn = 0.0;
    for (size_t j = 0; j < maxSearchNodes; ++j)
    {
      double value = searchSpace->exploreBestNode(context);
      if (value > highestReturn)
        highestReturn = value;
    }

    return searchSpace;
  }

  const FunctionPtr& getHeuristic() const
    {return heuristic;}

protected:
  SequentialDecisionProblemPtr problem;
  FunctionPtr heuristic;
  double discount;
  size_t maxSearchNodes;
};

typedef ReferenceCountedObjectPtr<LookAheadTreeSearchFunction> LookAheadTreeSearchFunctionPtr;


class LookAheadTreeSearchOnlineLearner : public OnlineLearner, public FunctionCallback
{
public:
  virtual void startLearning(ExecutionContext& context, const FunctionPtr& f, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    function = f.staticCast<LookAheadTreeSearchFunction>();
  }

  virtual void startLearningIteration(size_t iteration) {}

  virtual void startEpisode(const ObjectPtr& inputs)
  {
    //const FunctionPtr& heuristic = function->getHeuristic();
    //heuristic->addPostCallback(this);
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    const SortedSearchSpacePtr& searchSpace = output.getObjectAndCast<SortedSearchSpace>();

    size_t numOpenedNodes = searchSpace->getNumOpenedNodes();
    for (size_t i = 0; i < numOpenedNodes; ++i)
    {
      SearchSpaceNodePtr node = searchSpace->getOpenedNode(i);

    }
    //const FunctionPtr& heuristic = function->getHeuristic();
    //heuristic->removePostCallback(this);
    // output should be the result search space
  }

  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
  {
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize) {return false;} // returns true if learning is finished

  virtual void finishLearning() {}

private:
  LookAheadTreeSearchFunctionPtr function;
};

OnlineLearnerPtr lookAheadTreeSearchOnlineLearner()
  {return new LookAheadTreeSearchOnlineLearner();}

class SequentialDecisionSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    SequentialDecisionProblemPtr problem = linearPointPhysicProblem();
    if (!problem->initialize(context))
      return false;

    static const double discount = 0.9;
    static const size_t numSamples = 1000;
    
    FunctionPtr sampleInitialStatesFunction = createVectorFunction(problem->getInitialStateSampler(), false);
    if (!sampleInitialStatesFunction->initialize(context, positiveIntegerType, randomGeneratorClass))
      return false;
    
    RandomGeneratorPtr random = new RandomGenerator();
    ContainerPtr trainingStates = sampleInitialStatesFunction->compute(context, numSamples, random).getObjectAndCast<Container>();
    ContainerPtr testingStates = sampleInitialStatesFunction->compute(context, numSamples, random).getObjectAndCast<Container>();

    for (size_t depth = 2; depth <= 18; ++depth)
    {
      context.enterScope(T("Computing scores for depth = ") + String((int)depth));
      runAtDepth(context, depth, problem, trainingStates, testingStates, discount);
      context.leaveScope(true);
    }

    return true;
  }

  bool runAtDepth(ExecutionContext& context, size_t depth, SequentialDecisionProblemPtr problem, const ContainerPtr& trainingStates, const ContainerPtr& testingStates, double discount) const
  {
    context.resultCallback(T("depth"), depth);

    size_t maxSearchNodes = (size_t)pow(2.0, (double)(depth + 1)) - 1;
    context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
    
    FunctionPtr heuristic = new LinearLearnableSearchHeuristic();
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, discount, maxSearchNodes);
    if (!lookAHeadSearch->initialize(context, problem->getStateType()))
      return false;

    if (!lookAHeadSearch->train(context, trainingStates, testingStates, T("Training"), true))
      return false;

//    context.enterScope(T("Evaluation"));

    double learnedHeuristicScore = evaluateSearchHeuristic(context, problem, heuristic, maxSearchNodes, testingStates, discount);
    context.resultCallback(T("learned"), learnedHeuristicScore);

    double optimisticScore = evaluateSearchHeuristic(context, problem, optimisticPlanningSearchHeuristic(discount), maxSearchNodes, testingStates, discount);
    context.resultCallback(T("optimistic"), optimisticScore);

    double minDepthScore = evaluateSearchHeuristic(context, problem, minDepthSearchHeuristic(), maxSearchNodes, testingStates, discount);
    context.resultCallback(T("uniform"), minDepthScore);

    //context.leaveScope(learnedHeuristicScore);
    return true;
  }

  // todo: use new Evaluator here
  double evaluateSearchHeuristic(ExecutionContext& context, SequentialDecisionProblemPtr problem, FunctionPtr heuristic, size_t maxSearchNodes, ContainerPtr initialStates, double discount) const
  {
    RandomGeneratorPtr random = new RandomGenerator();
    
    //FunctionPtr heuristic = new LinearLearnableSearchHeuristic();
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, discount, maxSearchNodes);
    if (!lookAHeadSearch->initialize(context, problem->getStateType()))
      return 0.0;

    size_t n = initialStates->getNumElements();
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating heuristic ") + heuristic->toShortString(), n);
    std::vector<Variable> results(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable state = initialStates->getElement(i);
      workUnit->setWorkUnit(i, functionWorkUnit(lookAHeadSearch, std::vector<Variable>(1, state), String::empty, &results[i]));
    }
    workUnit->setProgressionUnit(T("Samples"));
    workUnit->setPushChildrenIntoStackFlag(false);
    context.run(workUnit);

    ScalarVariableStatistics stats;
    for (size_t i = 0; i < n; ++i)
      stats.push(results[i].getDouble());

    //context.informationCallback(stats.toString());
    return stats.getMean();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

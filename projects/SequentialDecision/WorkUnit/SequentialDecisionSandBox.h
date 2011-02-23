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
# include "../Core/SearchSpaceEvaluator.h"
# include "../Core/LookAheadTreeSearchFunction.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class HeuristicSearchSpaceNodeFeaturesFunction : public CompositeFunction
{
public:
  HeuristicSearchSpaceNodeFeaturesFunction(double discount = 0.9) : discount(discount) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchSpaceNodeClass, T("node"));

    builder.startSelection();

      builder.addFunction(greedySearchHeuristic(), node, T("maxReward"));
      builder.addFunction(greedySearchHeuristic(discount), node, T("maxDiscountedReward"));
      builder.addFunction(maxReturnSearchHeuristic(), node, T("maxReturn"));
      builder.addFunction(optimisticPlanningSearchHeuristic(discount), node, T("optimistic"));
      builder.addFunction(minDepthSearchHeuristic(), node, T("minDepth"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

protected:
  double discount;
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
    {return new HeuristicSearchSpaceNodeFeaturesFunction();}

  virtual FunctionPtr createScoringFunction() const
  {
    FunctionPtr res = linearLearnableFunction();
    return res;
  }
};

/////////////////////////////////////

class SequentialDecisionSandBox : public WorkUnit
{
public:
  SequentialDecisionSandBox() : numInitialStates(1000), minDepth(2), maxDepth(18), maxLearningIterations(100), discount(0.9) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }

    if (!problem->initialize(context))
      return false;

    FunctionPtr sampleInitialStatesFunction = createVectorFunction(problem->getInitialStateSampler(), false);
    if (!sampleInitialStatesFunction->initialize(context, positiveIntegerType, randomGeneratorClass))
      return false;
    
    RandomGeneratorPtr random = new RandomGenerator();
    ContainerPtr trainingStates = sampleInitialStatesFunction->compute(context, numInitialStates, random).getObjectAndCast<Container>();
    ContainerPtr testingStates = sampleInitialStatesFunction->compute(context, numInitialStates, random).getObjectAndCast<Container>();

    for (size_t depth = minDepth; depth <= maxDepth; ++depth)
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
    
    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(1.0), StoppingCriterionPtr(), maxLearningIterations);
    if (rankingLoss)
      parameters->setLossFunction(rankingLoss);
    parameters->setStoppingCriterion(averageImprovementStoppingCriterion(0.001));

    evaluate(context, T("maxReturn"), maxReturnSearchHeuristic(), maxSearchNodes, testingStates);
    evaluate(context, T("maxReward"), greedySearchHeuristic(), maxSearchNodes, testingStates);
    evaluate(context, T("maxDiscountedReward"), greedySearchHeuristic(discount), maxSearchNodes, testingStates);
    evaluate(context, T("optimistic"), optimisticPlanningSearchHeuristic(discount), maxSearchNodes, testingStates);
    evaluate(context, T("uniform"), minDepthSearchHeuristic(), maxSearchNodes, testingStates);

    trainAndEvaluate(context, T("baseline"), parameters, maxSearchNodes, trainingStates, testingStates);
    return true;
  }

protected:
  bool evaluate(ExecutionContext& context, const String& heuristicName, FunctionPtr heuristic, size_t maxSearchNodes, ContainerPtr initialStates) const
  {
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, LearnerParametersPtr(), discount, maxSearchNodes);
    if (!lookAHeadSearch->initialize(context, problem->getStateType()))
      return false;

    EvaluatorPtr evaluator = new SearchSpaceEvaluator();
    ScoreObjectPtr scores = lookAHeadSearch->evaluate(context, initialStates, evaluator, T("Evaluating heuristic ") + heuristicName);
    if (!scores)
      return false;

    context.resultCallback(heuristicName, -scores->getScoreToMinimize());
    return true;
  }

  FunctionPtr train(ExecutionContext& context, const String& name, StochasticGDParametersPtr parameters, size_t maxSearchNodes, const ContainerPtr& trainingStates, const ContainerPtr& testingStates) const
  {
    parameters->setEvaluator(new SearchSpaceEvaluator());
    FunctionPtr heuristic = new LinearLearnableSearchHeuristic();
    LookAheadTreeSearchFunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, parameters, discount, maxSearchNodes);
    if (!lookAHeadSearch->train(context, trainingStates, testingStates, T("Training ") + name, true))
      return FunctionPtr();
    return lookAHeadSearch->getHeuristic();
  }

  bool trainAndEvaluate(ExecutionContext& context, const String& name, StochasticGDParametersPtr parameters, size_t maxSearchNodes, const ContainerPtr& trainingStates, const ContainerPtr& testingStates) const
  {
    FunctionPtr heuristic = train(context, name, parameters, maxSearchNodes, trainingStates, testingStates);
    return heuristic && evaluate(context, name + T("-train"), heuristic, maxSearchNodes, trainingStates) 
        && evaluate(context, name + T("-test"), heuristic, maxSearchNodes, testingStates);
  }

private:
  friend class SequentialDecisionSandBoxClass;

  SequentialDecisionProblemPtr problem;
  RankingLossFunctionPtr rankingLoss;

  size_t numInitialStates;
  size_t minDepth;
  size_t maxDepth;
  size_t maxLearningIterations;
  double discount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

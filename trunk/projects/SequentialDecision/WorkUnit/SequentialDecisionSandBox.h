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

class SearchSpaceNodeIndexFeatureGenerator : public FeatureGenerator
{
public:
  SearchSpaceNodeIndexFeatureGenerator(size_t maxSearchNodes = 0)
    : maxSearchNodes(maxSearchNodes) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return searchSpaceNodeClass;}
  /*
  struct Enum : public Enumeration
  {
    Enum(size_t n)
      : Enumeration(T("SearchSpaceNodeIndex")), n(n) {}

    size_t n;

    virtual size_t getNumElements() const
      {return n;}

    virtual EnumerationElementPtr getElement(size_t index) const
    {
      String str((int)index);
      return new EnumerationElement(T("Node ") + str, String::empty, str);
    }
  };
*/
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return positiveIntegerEnumerationEnumeration;}// new Enum(maxSearchNodes);}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const SearchSpaceNodePtr& node = inputs[0].getObjectAndCast<SearchSpaceNode>();
    size_t index = node->getNodeIndex();
    //jassert(index < maxSearchNodes);
    callback.sense(index, 1.0);
  }

private:
  friend class SearchSpaceNodeIndexFeatureGeneratorClass;
        
  size_t maxSearchNodes;
};

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
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10((double)maxDepth), 7), depth);
      builder.addFunction(softDiscretizedNumberFeatureGenerator(0.0, maxReward, 7), reward);
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10(maxReturn), 7), currentReturn);

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
  LinearLearnableSearchHeuristic(size_t maxSearchNodes = 0)
    : maxSearchNodes(maxSearchNodes) {}

  size_t maxSearchNodes;

  virtual FunctionPtr createPerceptionFunction() const
    {return new SearchSpaceNodeIndexFeatureGenerator(maxSearchNodes);}

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
  SequentialDecisionSandBox() : numInitialStates(1000), maxSearchNodes(100000), beamSize(10000), maxLearningIterations(100), discount(0.9) {}

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

    /*
    for (size_t depth = minDepth; depth <= maxDepth; ++depth)
    {
      context.enterScope(T("Computing scores for depth = ") + String((int)depth));
      runAtDepth(context, depth, problem, trainingStates, testingStates, discount);
      context.leaveScope(true);
    }*/

    FunctionPtr href = optimisticPlanningSearchHeuristic(discount);
    FunctionPtr h1 = learnHeuristic(context, problem, href, trainingStates, testingStates, discount);

    return true;
  }

  FunctionPtr learnHeuristic(ExecutionContext& context, SequentialDecisionProblemPtr problem, FunctionPtr href, const ContainerPtr& trainingStates, const ContainerPtr& testingStates, double discount) const
  {
    evaluate(context, T("href-train"), href, trainingStates);
    evaluate(context, T("href-test"), href, testingStates);
    
    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(1.0), StoppingCriterionPtr(), maxLearningIterations);
    if (rankingLoss)
      parameters->setLossFunction(rankingLoss);
    parameters->setStoppingCriterion(averageImprovementStoppingCriterion(10e-6));
    trainAndEvaluate(context, T("pouet"), parameters, href, trainingStates, testingStates);
    return FunctionPtr();
  }

  /*
  bool runAtDepth(ExecutionContext& context, size_t depth, SequentialDecisionProblemPtr problem, const ContainerPtr& trainingStates, const ContainerPtr& testingStates, double discount) const
  {
    context.resultCallback(T("depth"), depth);

    size_t maxSearchNodes = (size_t)pow(2.0, (double)(depth + 1)) - 1;
    context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
    
    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(1.0), StoppingCriterionPtr(), maxLearningIterations);
    if (rankingLoss)
      parameters->setLossFunction(rankingLoss);
    parameters->setStoppingCriterion(averageImprovementStoppingCriterion(10e-6));

    evaluate(context, T("maxReturn"), maxReturnSearchHeuristic(), maxSearchNodes, testingStates);
    evaluate(context, T("maxReward"), greedySearchHeuristic(), maxSearchNodes, testingStates);
    evaluate(context, T("maxDiscountedReward"), greedySearchHeuristic(discount), maxSearchNodes, testingStates);
    evaluate(context, T("optimistic"), optimisticPlanningSearchHeuristic(discount), maxSearchNodes, testingStates);
    evaluate(context, T("uniform"), minDepthSearchHeuristic(), maxSearchNodes, testingStates);

    trainAndEvaluate(context, T("baseline"), parameters, maxSearchNodes, trainingStates, testingStates);
    return true;
  }*/

protected:
  bool evaluate(ExecutionContext& context, const String& heuristicName, FunctionPtr heuristic, ContainerPtr initialStates) const
  {
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, discount, maxSearchNodes, beamSize);
    if (!lookAHeadSearch->initialize(context, problem->getStateType()))
      return false;

    EvaluatorPtr evaluator = new SearchSpaceEvaluator();
    ScoreObjectPtr scores = lookAHeadSearch->evaluate(context, initialStates, evaluator, T("Evaluating heuristic ") + heuristicName);
    if (!scores)
      return false;

    context.resultCallback(heuristicName, -scores->getScoreToMinimize());
    return true;
  }

  FunctionPtr train(ExecutionContext& context, const String& name, StochasticGDParametersPtr parameters, FunctionPtr explorationHeuristic, const ContainerPtr& trainingStates, const ContainerPtr& testingStates) const
  {
    parameters->setEvaluator(new SearchSpaceEvaluator());
    LearnableSearchHeuristicPtr learnedHeuristic = new LinearLearnableSearchHeuristic(maxSearchNodes);
    learnedHeuristic->initialize(context, (TypePtr)searchSpaceNodeClass);
    LookAheadTreeSearchFunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, learnedHeuristic, parameters, explorationHeuristic, discount, maxSearchNodes, beamSize);
    if (!lookAHeadSearch->train(context, trainingStates, testingStates, T("Training ") + name, true))
      return FunctionPtr();
    return learnedHeuristic;
  }

  bool trainAndEvaluate(ExecutionContext& context, const String& name, StochasticGDParametersPtr parameters, FunctionPtr explorationHeuristic, const ContainerPtr& trainingStates, const ContainerPtr& testingStates) const
  {
    FunctionPtr heuristic = train(context, name, parameters, explorationHeuristic, trainingStates, testingStates);
    return heuristic && evaluate(context, name + T("-train"), heuristic, trainingStates) 
        && evaluate(context, name + T("-test"), heuristic, testingStates);
  }

private:
  friend class SequentialDecisionSandBoxClass;

  SequentialDecisionProblemPtr problem;
  RankingLossFunctionPtr rankingLoss;

  size_t numInitialStates;
  size_t maxSearchNodes;
  size_t beamSize;
  size_t minDepth;
  size_t maxDepth;
  size_t maxLearningIterations;
  double discount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

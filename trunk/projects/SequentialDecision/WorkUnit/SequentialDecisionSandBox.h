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
# include "../Core/SearchTree.h"
# include "../Core/SearchTreeEvaluator.h"
# include "../Core/SearchFunction.h"
# include "../Core/SearchPolicy.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class SearchTreeNodeIndexFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return searchTreeNodeClass();}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return positiveIntegerEnumerationEnumeration;}// new Enum(maxSearchNodes);}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const SearchTreeNodePtr& node = inputs[0].getObjectAndCast<SearchTreeNode>();
    size_t index = node->getNodeUid();
    callback.sense(index, 1.0);
  }
};

class HeuristicSearchTreeNodeFeaturesFunction : public CompositeFunction
{
public:
  HeuristicSearchTreeNodeFeaturesFunction(double discount = 0.9) : discount(discount) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass(), T("node"));

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

class GenericClosedSearchTreeNodeFeaturesFunction : public CompositeFunction
{
public:
  GenericClosedSearchTreeNodeFeaturesFunction(size_t maxDepth = 1000, double maxReward = 1.0, double maxReturn = 10.0)
    : maxDepth(maxDepth), maxReward(maxReward), maxReturn(maxReturn) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass(), T("node"));
    size_t depth = builder.addFunction(getVariableFunction(T("depth")), node);
    size_t reward = builder.addFunction(getVariableFunction(T("reward")), node);
    size_t currentReturn = builder.addFunction(getVariableFunction(T("currentReturn")), node);
    //size_t action = builder.addFunction(getVariableFunction(T("previousAction")), node);

    builder.startSelection();

      // max depth = 1000, max reward = 100
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10((double)maxDepth), 7), depth);
      builder.addFunction(softDiscretizedNumberFeatureGenerator(0.0, maxReward, 7), reward);
      builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10(maxReturn), 7), currentReturn);

    size_t allFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));

//    builder.addFunction(new CartesianProductFeatureGenerator(), allFeatures, allFeatures, T("conjunctions"));
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
    {return new HeuristicSearchTreeNodeFeaturesFunction();}

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
  // default values
  SequentialDecisionSandBox() : numInitialStates(1000), maxSearchNodes(100000), beamSize(10000), maxLearningIterations(100), numPasses(10)
  {
    problem = linearPointPhysicProblem(0.9);
    rankingLoss = allPairsRankingLossFunction(hingeDiscriminativeLossFunction());
  }

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }

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

    FunctionPtr href = optimisticPlanningSearchHeuristic(problem->getDiscount());
    PolicyPtr currentSearchPolicy = beamSearchPolicy(href, beamSize);

    evaluate(context, T("href-train"), currentSearchPolicy, trainingStates);
    evaluate(context, T("href-test"), currentSearchPolicy, testingStates);

    bazar(context, testingStates);
    return true;

    for (size_t i = 0; i < numPasses; ++i)
    {
      StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(0.01), StoppingCriterionPtr(), maxLearningIterations);
      if (rankingLoss)
        parameters->setLossFunction(rankingLoss);
      //parameters->setStoppingCriterion(averageImprovementStoppingCriterion(10e-6));

      PolicyPtr newSearchPolicy = train(context, T("iter-") + String((int)i), parameters, currentSearchPolicy, trainingStates, testingStates);

      context.enterScope(T("Evaluating candidate mixtures"));
      double bestMixtureScore = -DBL_MAX;
      double bestMixtureCoefficient;
      PolicyPtr bestMixturePolicy;

      for (double k = 0.0; k <= 1.005; k += 0.01)
      {
        PolicyPtr policy = mixturePolicy(currentSearchPolicy, newSearchPolicy, k);
        double score = evaluate(context, T("mixt(") + String(k) + T(")"), policy, testingStates, k);
        if (score > bestMixtureScore)
        {
          bestMixtureScore = score;
          bestMixturePolicy = policy;
          bestMixtureCoefficient = k;
        }
      }
      if (!bestMixturePolicy)
      {
        context.errorCallback(T("Could not find best mixture policy"));
        break;
      }
      context.leaveScope(new Pair(bestMixtureScore, bestMixtureCoefficient));

      currentSearchPolicy = bestMixturePolicy;
    }
    return true;
  }

  void bazar(ExecutionContext& context, ContainerPtr testingStates)
  {
    LearnableSearchHeuristicPtr learnedHeuristic = new LinearLearnableSearchHeuristic();
    learnedHeuristic->initialize(context, (TypePtr)searchTreeNodeClass());
    NumericalLearnableFunctionPtr linearFunction = learnedHeuristic->getScoringFunction().dynamicCast<NumericalLearnableFunction>();
    PolicyPtr learnedSearchPolicy = beamSearchPolicy(learnedHeuristic, beamSize);

    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(learnedHeuristic->getPerceptionFunction()->getOutputType());
    DenseDoubleVectorPtr parameters = linearFunction->createParameters();
    parameters->setValue(0, 1.0);
    linearFunction->setParameters(parameters);

    context.enterScope(T("Bazar"));
    for (size_t i = 0; i < maxLearningIterations; ++i)
    {
      size_t param = i % featuresEnumeration->getNumElements();
      context.enterScope(T("Iteration ") + String((int)i) + T(" Param ") + featuresEnumeration->getElementName(param));

      double previousValue = parameters->getValue(param);
      double bestValue = parameters->getValue(param);
      double bestScore = -DBL_MAX;
      double amp = (maxLearningIterations - i) / (double)maxLearningIterations;
      for (double k = previousValue - amp; k <= previousValue + amp; k += 2 * amp / 10.0)
      {
        if (fabs(k) < 10e-10)
          k = 0.0;
        context.enterScope(T("K = ") + String(k));
        context.resultCallback(T("K"), k);
        parameters->setValue(param, k);
        double score = evaluate(context, featuresEnumeration->getElementName(param), learnedSearchPolicy, testingStates);
        if (score > bestScore)
          bestScore = score, bestValue = k;
        context.leaveScope(score);
      }
      parameters->setValue(param, bestValue);
      double score = evaluate(context, featuresEnumeration->getElementName(param), learnedSearchPolicy, testingStates);

      context.leaveScope(bestScore);
    }
    context.leaveScope(true);

  }

protected:
  double evaluate(ExecutionContext& context, const String& heuristicName, PolicyPtr searchPolicy, ContainerPtr initialStates, const Variable& argument = Variable()) const
  {
    FunctionPtr searchFunction = new SearchFunction(problem, searchPolicy, maxSearchNodes);
    if (!searchFunction->initialize(context, problem->getStateClass()))
      return -1.0;

    String scopeName = T("Evaluating heuristic ") + heuristicName;

    context.enterScope(scopeName);
    EvaluatorPtr evaluator = new SearchTreeEvaluator();
    ScoreObjectPtr scores = searchFunction->evaluate(context, initialStates, evaluator);
    if (argument.exists())
      context.resultCallback(T("argument"), argument);
    context.leaveScope(scores);

    double bestReturn = scores ? -scores->getScoreToMinimize() : -1.0;
    context.resultCallback(heuristicName, bestReturn);
    return bestReturn;
  }

  PolicyPtr train(ExecutionContext& context, const String& name, StochasticGDParametersPtr parameters, PolicyPtr explorationPolicy, const ContainerPtr& trainingStates, const ContainerPtr& testingStates) const
  {
    LearnableSearchHeuristicPtr learnedHeuristic = new LinearLearnableSearchHeuristic();
    learnedHeuristic->initialize(context, (TypePtr)searchTreeNodeClass());
    PolicyPtr learnedSearchPolicy = beamSearchPolicy(learnedHeuristic, beamSize);

    SearchFunctionPtr lookAHeadSearch = new SearchFunction(problem, learnedSearchPolicy, parameters, explorationPolicy, maxSearchNodes);
    lookAHeadSearch->setEvaluator(new SearchTreeEvaluator());
    if (!lookAHeadSearch->train(context, trainingStates, testingStates, T("Training ") + name, true))
      return PolicyPtr();
    return learnedSearchPolicy;
  }

private:
  friend class SequentialDecisionSandBoxClass;

  DecisionProblemPtr problem;
  RankingLossFunctionPtr rankingLoss;

  size_t numInitialStates;
  size_t maxSearchNodes;
  size_t beamSize;
  size_t minDepth;
  size_t maxDepth;
  size_t maxLearningIterations;
  size_t numPasses;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

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
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

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

// SearchNode -> Scalar
class LearnableSearchHeuristic : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchSpaceNodeClass, T("node"));
    size_t perception = builder.addFunction(perceptionFunction = createPerceptionFunction(), node);
    size_t supervision = builder.addConstant(Variable());
    builder.addFunction(scoringFunction = createScoringFunction(), perception, supervision);
  }

  const FunctionPtr& getPerceptionFunction() const
    {return perceptionFunction;}

  const FunctionPtr& getScoringFunction() const
    {return scoringFunction;}

protected:
  FunctionPtr perceptionFunction;  // SearchNode -> Features
  FunctionPtr scoringFunction;     // Features -> Score

  virtual FunctionPtr createPerceptionFunction() const = 0;
  virtual FunctionPtr createScoringFunction() const = 0;
};

typedef ReferenceCountedObjectPtr<LearnableSearchHeuristic> LearnableSearchHeuristicPtr;

class LinearLearnableSearchHeuristic : public LearnableSearchHeuristic
{
public:
  virtual FunctionPtr createPerceptionFunction() const
    {return new GenericClosedSearchSpaceNodeFeaturesFunction();}

  virtual FunctionPtr createScoringFunction() const
  {
    FunctionPtr res = linearLearnableFunction();
    return res;
  }
};

extern OnlineLearnerPtr lookAheadTreeSearchOnlineLearner();

// State -> SearchSpace
class LookAheadTreeSearchFunction : public SimpleUnaryFunction
{
public:
  LookAheadTreeSearchFunction(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, StochasticGDParametersPtr learnerParameters, double discount, size_t maxSearchNodes)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), heuristic(heuristic), learnerParameters(learnerParameters), discount(discount), maxSearchNodes(maxSearchNodes)
  {
   
  }
  LookAheadTreeSearchFunction() : SimpleUnaryFunction(anyType, anyType) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!heuristic)
    {
      context.errorCallback(T("No heuristic"));
      return TypePtr();
    }
    if (!heuristic->initialize(context, (TypePtr)searchSpaceNodeClass))
      return TypePtr();

    TypePtr outputType = SimpleUnaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);

    const size_t maxIterations = 100;
    if (learnerParameters)
    {
      setBatchLearner(learnerParameters->createBatchLearner(context, inputVariables, outputType));

      std::vector<OnlineLearnerPtr> onlineLearners;
      onlineLearners.push_back(lookAheadTreeSearchOnlineLearner());
      if (learnerParameters->getStoppingCriterion())
        onlineLearners.push_back(stoppingCriterionOnlineLearner(learnerParameters->getStoppingCriterion()));
      if (learnerParameters->doRestoreBestParameters())
        onlineLearners.push_back(restoreBestParametersOnlineLearner());
      setOnlineLearner(compositeOnlineLearner(onlineLearners));

      LearnableSearchHeuristicPtr learnableHeuristic = heuristic.dynamicCast<LearnableSearchHeuristic>();
      if (learnableHeuristic)
      {
        OnlineLearnerPtr stochasticGDLearner = stochasticGDOnlineLearner(FunctionPtr(), learnerParameters->getLearningRate(), learnerParameters->doNormalizeLearningRate());
        learnableHeuristic->getScoringFunction()->setOnlineLearner(stochasticGDLearner);
      }
    }
    return outputType;
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
  friend class LookAheadTreeSearchFunctionClass;

  SequentialDecisionProblemPtr problem;
  FunctionPtr heuristic;
  StochasticGDParametersPtr learnerParameters;
  double discount;
  size_t maxSearchNodes;
};

typedef ReferenceCountedObjectPtr<LookAheadTreeSearchFunction> LookAheadTreeSearchFunctionPtr;

class LookAheadTreeSearchOnlineLearner : public OnlineLearner
{
public:
  LookAheadTreeSearchOnlineLearner(RankingLossFunctionPtr rankingLoss = allPairsRankingLossFunction(hingeDiscriminativeLossFunction()))
    : context(NULL), rankingLoss(rankingLoss) {}

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& f, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->context = &context;
    searchFunction = f.staticCast<LookAheadTreeSearchFunction>();

    LearnableSearchHeuristicPtr heuristic = searchFunction->getHeuristic().staticCast<LearnableSearchHeuristic>();
    featuresFunction = heuristic->getPerceptionFunction();
    scoringFunction = heuristic->getScoringFunction().staticCast<NumericalLearnableFunction>();
    jassert(scoringFunction);
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    const SortedSearchSpacePtr& searchSpace = output.getObjectAndCast<SortedSearchSpace>();

    size_t numOpenedNodes = searchSpace->getNumOpenedNodes();

    std::set<size_t> candidates;
    candidates.insert(0);
    std::vector<double> episodeGradient(searchSpace->getNumNodes(), 0.0);
    double selectedNodesCostSum = 0.0;
    for (size_t i = 0; i < numOpenedNodes; ++i)
    {
      size_t selectedNodeIndex = searchSpace->getOpenedNodeIndex(i);

      if (i > 0)
      {
        std::vector<double> scores(candidates.size());
        std::vector<double> costs(candidates.size());

        size_t c = 0;
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
        {
          SearchSpaceNodePtr node = searchSpace->getNode(*it);
          scores[c] = node->getHeuristicScore();
          double cost = (node == node->getParentNode()->getBestChildNode()) ? 0.0 : 1.0; // bipartite ranking for the moment
          costs[c] = cost;
          if (*it == selectedNodeIndex)
            selectedNodesCostSum += cost;
        }

        // compute ranking loss
        std::vector<double> rankingLossGradient(candidates.size(), 0.0);
        rankingLoss->computeRankingLoss(scores, costs, NULL, &rankingLossGradient);

        // update episode gradient
        c = 0;
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
          episodeGradient[*it] += rankingLossGradient[c];
      }

      // update candidates list
      candidates.erase(selectedNodeIndex);
      SearchSpaceNodePtr node = searchSpace->getNode(selectedNodeIndex);
      int begin = node->getChildBeginIndex();
      if (begin >= 0)
        for (int childIndex = begin; childIndex < node->getChildEndIndex(); ++childIndex)
          candidates.insert(childIndex);
    }

    // apply episode gradient
    double weight = 1.0 / (double)numOpenedNodes;
    DoubleVectorPtr parametersGradient;
    double gradientNorm = 0.0;
    for (size_t i = 0; i < episodeGradient.size(); ++i)
    {
      double g = episodeGradient[i];
      if (g)
      {
        DoubleVectorPtr nodeFeatures = featuresFunction->compute(*context, searchSpace->getNode(i)).getObjectAndCast<DoubleVector>();
        scoringFunction->addGradient(g, nodeFeatures, parametersGradient, weight);
        gradientNorm += g * g;
      }
    }
    scoringFunction->compute(*context, DoubleVectorPtr(), parametersGradient);

    this->selectedNodesCost.push(selectedNodesCostSum / (numOpenedNodes - 1));
    this->gradientNorm.push(sqrt(gradientNorm));
    this->bestReturn.push(searchSpace->getBestReturn());
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    DoubleVectorPtr parameters = scoringFunction->getParameters();

    context->resultCallback(T("Mean Selected Node Cost"), selectedNodesCost.getMean());
    context->resultCallback(T("Episode Gradient Norm"), gradientNorm.getMean());
    context->resultCallback(T("Best Return"), bestReturn.getMean());
    //context->resultCallback(T("Parameters Norm"), parameters->l2norm());
    //context->resultCallback(T("Num. Parameters"), parameters->l0norm());

    objectiveValueToMinimize = -bestReturn.getMean();
    selectedNodesCost.clear();
    gradientNorm.clear();
    bestReturn.clear();
    return false;
  }


private:
  friend class LookAheadTreeSearchOnlineLearnerClass;

  ExecutionContext* context;
  LookAheadTreeSearchFunctionPtr searchFunction;
  FunctionPtr featuresFunction;
  NumericalLearnableFunctionPtr scoringFunction;
  RankingLossFunctionPtr rankingLoss;

  ScalarVariableStatistics selectedNodesCost;
  ScalarVariableStatistics gradientNorm;
  ScalarVariableStatistics bestReturn;

};

OnlineLearnerPtr lookAheadTreeSearchOnlineLearner()
  {return new LookAheadTreeSearchOnlineLearner();}

class SequentialDecisionSandBox : public WorkUnit
{
public:
  SequentialDecisionSandBox() : numInitialStates(1000), minDepth(2), maxDepth(18) {}

  virtual Variable run(ExecutionContext& context)
  {
    SequentialDecisionProblemPtr problem = linearPointPhysicProblem();
    if (!problem->initialize(context))
      return false;

    static const double discount = 0.9;
    
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
    
    FunctionPtr heuristic = new LinearLearnableSearchHeuristic();
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, new StochasticGDParameters(), discount, maxSearchNodes);
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
    FunctionPtr lookAHeadSearch = new LookAheadTreeSearchFunction(problem, heuristic, LearnerParametersPtr(), discount, maxSearchNodes);
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
    {
      const SortedSearchSpacePtr& searchSpace = results[i].getObjectAndCast<SortedSearchSpace>();
      stats.push(searchSpace->getBestReturn());
    }

    //context.informationCallback(stats.toString());
    return stats.getMean();
  }

private:
  friend class SequentialDecisionSandBoxClass;

  size_t numInitialStates;
  size_t minDepth;
  size_t maxDepth;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

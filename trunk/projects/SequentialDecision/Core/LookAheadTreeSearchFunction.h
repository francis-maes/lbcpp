/*-----------------------------------------.---------------------------------.
| Filename: LookAheadTreeSearchFunction.h  | Look AHead Tree Search Function |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 12:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_FUNCTION_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_FUNCTION_H_

# include "SearchSpace.h"
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{
  
// SearchNode -> Scalar
class LearnableSearchHeuristic : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchSpaceNodeClass, T("node"));
    size_t perception = builder.addFunction(createPerceptionFunction(), node);
    size_t supervision = builder.addConstant(Variable());
    builder.addFunction(createScoringFunction(), perception, supervision);
  }

  const FunctionPtr& getPerceptionFunction() const
    {return functions[0];}

  const FunctionPtr& getScoringFunction() const
    {return functions[1];}
 
protected:
  virtual FunctionPtr createPerceptionFunction() const = 0; // SearchNode -> Features
  virtual FunctionPtr createScoringFunction() const = 0;    // Features -> Score
};

typedef ReferenceCountedObjectPtr<LearnableSearchHeuristic> LearnableSearchHeuristicPtr;


extern OnlineLearnerPtr lookAheadTreeSearchOnlineLearner(RankingLossFunctionPtr lossFunction);

// State -> SearchSpace
class LookAheadTreeSearchFunction : public SimpleUnaryFunction
{
public:
  LookAheadTreeSearchFunction(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, StochasticGDParametersPtr learnerParameters, FunctionPtr explorationHeuristic, double discount, size_t maxSearchNodes, size_t beamSize)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), heuristic(heuristic), learnerParameters(learnerParameters), explorationHeuristic(explorationHeuristic), discount(discount), maxSearchNodes(maxSearchNodes), beamSize(beamSize)
    {}

  LookAheadTreeSearchFunction(SequentialDecisionProblemPtr problem, FunctionPtr heuristic, double discount, size_t maxSearchNodes, size_t beamSize)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), heuristic(heuristic), discount(discount), maxSearchNodes(maxSearchNodes), beamSize(beamSize)
    {}

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

    if (learnerParameters)
    {
      setBatchLearner(learnerParameters->createBatchLearner(context, inputVariables, outputType));

      std::vector<OnlineLearnerPtr> onlineLearners;
      onlineLearners.push_back(lookAheadTreeSearchOnlineLearner(learnerParameters->getLossFunction().dynamicCast<RankingLossFunction>()));
      if (learnerParameters->getEvaluator())
        onlineLearners.push_back(evaluatorOnlineLearner(learnerParameters->getEvaluator()));
      if (learnerParameters->getStoppingCriterion())
        onlineLearners.push_back(stoppingCriterionOnlineLearner(learnerParameters->getStoppingCriterion()));
      if (learnerParameters->doRestoreBestParameters())
        onlineLearners.push_back(restoreBestParametersOnlineLearner());
      setOnlineLearner(compositeOnlineLearner(onlineLearners));

      LearnableSearchHeuristicPtr learnedHeuristic = heuristic.dynamicCast<LearnableSearchHeuristic>();
      if (learnedHeuristic)
      {
        OnlineLearnerPtr stochasticGDLearner = stochasticGDOnlineLearner(FunctionPtr(), learnerParameters->getLearningRate(), learnerParameters->doNormalizeLearningRate());
        learnedHeuristic->getScoringFunction()->setOnlineLearner(stochasticGDLearner);
      }
    }
    return outputType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& initialState) const
    {return search(context, isCurrentlyLearning() ? explorationHeuristic : heuristic, initialState);}

  SortedSearchSpacePtr search(ExecutionContext& context, const FunctionPtr& heuristic, const Variable& initialState) const
  {
    SortedSearchSpacePtr searchSpace = new SortedSearchSpace(problem, heuristic, discount, beamSize, initialState);
    searchSpace->reserveNodes(2 * maxSearchNodes);
    for (size_t j = 0; j < maxSearchNodes; ++j)
      searchSpace->exploreBestNode(context);
    return searchSpace;
  }

  void setHeuristic(const FunctionPtr& heuristic)
    {this->heuristic = heuristic;}

  const FunctionPtr& getHeuristic() const
    {return heuristic;}

  size_t getBeamSize() const
    {return beamSize;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    SimpleUnaryFunction::clone(context, target);
    if (heuristic)
      target.staticCast<LookAheadTreeSearchFunction>()->heuristic = heuristic->clone(context);
  }

protected:
  friend class LookAheadTreeSearchFunctionClass;

  SequentialDecisionProblemPtr problem;
  FunctionPtr heuristic;
  StochasticGDParametersPtr learnerParameters;
  FunctionPtr explorationHeuristic;
  double discount;
  size_t maxSearchNodes;
  size_t beamSize;
};

typedef ReferenceCountedObjectPtr<LookAheadTreeSearchFunction> LookAheadTreeSearchFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_FUNCTION_H_

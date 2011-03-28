/*-----------------------------------------.---------------------------------.
| Filename: SearchFunction.h               | Look AHead Tree Search Function |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 12:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_H_

# include "SearchTree.h"
# include "SearchHeuristic.h"
# include "Policy.h"
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{
  
extern OnlineLearnerPtr searchFunctionOnlineLearner(RankingLossFunctionPtr lossFunction);

// State -> SearchSpace
class SearchFunction : public SimpleUnaryFunction
{
public:
  SearchFunction(DecisionProblemPtr problem, PolicyPtr searchPolicy, StochasticGDParametersPtr learnerParameters, PolicyPtr explorationPolicy, size_t maxSearchNodes)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), searchPolicy(searchPolicy), learnerParameters(learnerParameters), explorationPolicy(explorationPolicy), maxSearchNodes(maxSearchNodes)
    {}

  SearchFunction(DecisionProblemPtr problem, PolicyPtr searchPolicy, size_t maxSearchNodes)
    : SimpleUnaryFunction(anyType, anyType), problem(problem), searchPolicy(searchPolicy), maxSearchNodes(maxSearchNodes)
    {}

  SearchFunction() : SimpleUnaryFunction(anyType, anyType) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!searchPolicy)
    {
      context.errorCallback(T("No search policy"));
      return TypePtr();
    }

    TypePtr outputType = SimpleUnaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);

    if (learnerParameters)
    {
      setBatchLearner(learnerParameters->createBatchLearner(context));

      std::vector<OnlineLearnerPtr> onlineLearners;
      onlineLearners.push_back(searchFunctionOnlineLearner(learnerParameters->getLossFunction().dynamicCast<RankingLossFunction>()));
      if (learnerParameters->doEvaluateAtEachIteration())
        onlineLearners.push_back(evaluatorOnlineLearner());
      if (learnerParameters->getStoppingCriterion())
        onlineLearners.push_back(stoppingCriterionOnlineLearner(learnerParameters->getStoppingCriterion()));
      if (learnerParameters->doRestoreBestParameters())
        onlineLearners.push_back(restoreBestParametersOnlineLearner());
      setOnlineLearner(compositeOnlineLearner(onlineLearners));

      LearnableSearchHeuristicPtr learnedHeuristic = searchPolicy->getVariable(0).dynamicCast<LearnableSearchHeuristic>();
      if (learnedHeuristic)
      {
        OnlineLearnerPtr stochasticGDLearner = stochasticGDOnlineLearner(FunctionPtr(), learnerParameters->getLearningRate(), learnerParameters->doNormalizeLearningRate());
        learnedHeuristic->getScoringFunction()->setOnlineLearner(stochasticGDLearner);
      }
    }
    return outputType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& initialState) const
  {
    PolicyPtr policy = isCurrentlyLearning() ? explorationPolicy : searchPolicy;
    SearchTreePtr searchTree = new SearchTree(problem, initialState.getObjectAndCast<DecisionProblemState>(), maxSearchNodes);
    searchTree->doSearchEpisode(context, policy, maxSearchNodes);
    return searchTree;
  }
   
  void setSearchPolicy(const PolicyPtr& searchPolicy)
    {this->searchPolicy = searchPolicy;}

  const PolicyPtr& getSearchPolicy() const
    {return searchPolicy;}

  size_t getMaxSearchNodes() const
    {return maxSearchNodes;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    SimpleUnaryFunction::clone(context, target);
    if (searchPolicy)
      target.staticCast<SearchFunction>()->searchPolicy = searchPolicy->clone(context);
  }

protected:
  friend class SearchFunctionClass;

  DecisionProblemPtr problem;
  PolicyPtr searchPolicy;
  StochasticGDParametersPtr learnerParameters;
  PolicyPtr explorationPolicy;
  size_t maxSearchNodes;
};

typedef ReferenceCountedObjectPtr<SearchFunction> SearchFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_H_

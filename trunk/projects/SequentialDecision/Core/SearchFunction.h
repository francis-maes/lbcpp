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
# include "Policy.h"
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
    size_t node = builder.addInput(searchTreeNodeClass, T("node"));
    size_t perception = builder.addFunction(createPerceptionFunction(), node);
    size_t supervision = builder.addConstant(Variable());
    builder.addFunction(createScoringFunction(), perception, supervision);
  }

  const FunctionPtr& getPerceptionFunction() const
    {return functions[0];}

  const FunctionPtr& getScoringFunction() const
    {return functions[1];}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    Variable res = CompositeFunction::computeFunction(context, inputs);
    return res.exists() ? res.getDouble() : 0.0;
  }

protected:
  virtual FunctionPtr createPerceptionFunction() const = 0; // SearchNode -> Features
  virtual FunctionPtr createScoringFunction() const = 0;    // Features -> Score
};

typedef ReferenceCountedObjectPtr<LearnableSearchHeuristic> LearnableSearchHeuristicPtr;


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
      setBatchLearner(learnerParameters->createBatchLearner(context, inputVariables, outputType));

      std::vector<OnlineLearnerPtr> onlineLearners;
      onlineLearners.push_back(searchFunctionOnlineLearner(learnerParameters->getLossFunction().dynamicCast<RankingLossFunction>()));
      if (learnerParameters->getEvaluator())
        onlineLearners.push_back(evaluatorOnlineLearner(learnerParameters->getEvaluator()));
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
    SearchTreePtr searchTree = new SearchTree(problem, initialState, maxSearchNodes);
    jassert(searchTree->getBestReturn() == 0.0);
    double lastReward = 0.0;
    double bestReturn = 0.0;

    PolicyPtr policy = isCurrentlyLearning() ? explorationPolicy : searchPolicy;

    for (size_t i = 0; i < maxSearchNodes; ++i)
    {
      ContainerPtr actions;
      Variable selectedNode;
      if (i == 0)
        selectedNode = policy->policyStart(context, searchTree, actions);
      else
        selectedNode = policy->policyStep(context, lastReward, searchTree, actions);
      if (!selectedNode.exists())
      {
        context.errorCallback(T("No selected node"));
        break;
      }

      searchTree->exploreNode(context, (size_t)selectedNode.getInteger());
      double newBestReturn = searchTree->getBestReturn();
      lastReward = newBestReturn - bestReturn;
      jassert(lastReward >= 0.0);
      bestReturn = newBestReturn;
    }
    policy->policyEnd(context, lastReward, searchTree);
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

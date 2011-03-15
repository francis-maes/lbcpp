/*-----------------------------------------.---------------------------------.
| Filename: SearchFunctionOnlineLearner.h  | Search Function Online Learner  |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 12:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_ONLINE_LEARNER_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_ONLINE_LEARNER_H_

# include "SearchFunction.h"
# include "SearchPolicy.h"
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Data/RandomVariable.h>
# include "../../../src/Learning/Numerical/GradientDescentOnlineLearner.h"

namespace lbcpp
{

class SearchFunctionOnlineLearner : public OnlineLearner
{
public:
  SearchFunctionOnlineLearner(RankingLossFunctionPtr rankingLoss)
    : context(NULL), rankingLoss(rankingLoss)
  {
    if (!rankingLoss)
      this->rankingLoss = allPairsRankingLossFunction(hingeDiscriminativeLossFunction());
  }

  SearchFunctionOnlineLearner() : context(NULL) {}

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& f, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->context = &context;
    searchFunction = f.staticCast<SearchFunction>();
    
    searchPolicy = searchFunction->getSearchPolicy().dynamicCast<BestFirstSearchPolicy>();
    jassert(searchPolicy);
    LearnableSearchHeuristicPtr learnedHeuristic = searchPolicy->getHeuristic().dynamicCast<LearnableSearchHeuristic>();
    jassert(learnedHeuristic);
    featuresFunction = learnedHeuristic->getPerceptionFunction();
    scoringFunction = learnedHeuristic->getScoringFunction().staticCast<NumericalLearnableFunction>();
    jassert(scoringFunction);
    return true;
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    const SearchTreePtr& searchTree = output.getObjectAndCast<SearchTree>();

    size_t n = searchTree->getNumNodes();
    std::vector<DoubleVectorPtr> nodeFeatures(n);
    std::vector<double> nodeScores(n);
    std::vector<double> nodeCosts(n);
    for (size_t i = 0; i < nodeFeatures.size(); ++i)
    {
      const SearchTreeNodePtr& node = searchTree->getNode(i);
      nodeFeatures[i] = featuresFunction->compute(*context, node).getObjectAndCast<DoubleVector>();
      nodeScores[i] = scoringFunction->compute(*context, nodeFeatures[i], Variable()).getDouble();
      nodeCosts[i] = -node->getCurrentReturn();
    }

    // compute ranking loss
    std::vector<double> rankingLossGradient(n, 0.0);
    double exampleLoss = 0.0;
    rankingLoss->computeRankingLoss(nodeScores, nodeCosts, &exampleLoss, &rankingLossGradient);

    jassert(false); // broken
#if 0
    // apply episode gradient
    DoubleVectorPtr parametersGradient = scoringFunction->createParameters();
    double gradientNorm = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double g = rankingLossGradient[i];
      if (g)
      {
        scoringFunction->addGradient(g, nodeFeatures[i], parametersGradient, 1.0);
        gradientNorm += g * g;
      }
    }
    GradientDescentOnlineLearnerPtr onlineLearner = scoringFunction->getOnlineLearner().dynamicCast<GradientDescentOnlineLearner>();
    jassert(onlineLearner);
    if (parametersGradient)
      onlineLearner->addComputedGradient(scoringFunction, parametersGradient, exampleLoss);

    this->gradientNorm.push(sqrt(gradientNorm));
#endif // 0
  }

#if 0
  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    const SearchTreePtr& searchTree = output.getObjectAndCast<SearchTree>();

    size_t numOpenedNodes = searchTree->getNumOpenedNodes();
    size_t beamSize = searchFunction->getMaxSearchNodes();
    if (searchPolicy.dynamicCast<BeamSearchPolicy>())
      beamSize = searchPolicy.staticCast<BeamSearchPolicy>()->getBeamSize();

    std::vector<DoubleVectorPtr> nodeFeatures(searchTree->getNumNodes());
    std::vector<double> nodeHeuristicScores(searchTree->getNumNodes());
    for (size_t i = 0; i < nodeFeatures.size(); ++i)
    {
      nodeFeatures[i] = featuresFunction->compute(*context, searchTree->getNode(i)).getObjectAndCast<DoubleVector>();
      nodeHeuristicScores[i] = scoringFunction->compute(*context, nodeFeatures[i], Variable()).getDouble();
    }

    std::set<size_t> candidates;
    candidates.insert(0);
    std::vector<double> episodeGradient(searchTree->getNumNodes(), 0.0);
    double selectedNodesCostSum = 0.0;
    double lossValue = 0.0;
    for (size_t i = 0; i < numOpenedNodes; ++i)
    {
      size_t selectedNodeIndex = searchTree->getOpenedNodeIndex(i);

      if (i > 0)
      {
        jassert(candidates.size() > 1);
        std::vector<double> scores(candidates.size());
        std::vector<double> costs(candidates.size());

        size_t c = 0;
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
        {
          SearchTreeNodePtr node = searchTree->getNode(*it);
          scores[c] = nodeHeuristicScores[*it];
          //double cost = node->getParentNode()->getBestReturn() - node->getBestReturn();
          double cost = -node->getCurrentReturn();

          //double cost = node->getParentNode()->getBestReturnWithoutChild(node) - node->getParentNode()->getBestReturn();
          //double cost = node->getParentNode()->getBestReturn() - node->getBestReturn();
            //(node == node->getParentNode()->getBestChildNode()) ? 0.0 : 1.0; // bipartite ranking for the moment
          costs[c] = cost;
          if (*it == selectedNodeIndex)
            selectedNodesCostSum += cost;
        }

        // compute ranking loss
        std::vector<double> rankingLossGradient(candidates.size(), 0.0);
        double exampleLoss = 0.0;
        rankingLoss->computeRankingLoss(scores, costs, &exampleLoss, &rankingLossGradient);
       
        // update episode gradient
        c = 0;
        double invZ = 1.0 / (double)candidates.size();
        lossValue += invZ * exampleLoss;
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
          episodeGradient[*it] += invZ * rankingLossGradient[c];
      }

      // update candidates list
      candidates.erase(selectedNodeIndex);
      SearchTreeNodePtr node = searchTree->getNode(selectedNodeIndex);
      int begin = node->getChildBeginIndex();
      if (begin >= 0)
        for (int childIndex = begin; childIndex < node->getChildEndIndex(); ++childIndex)
          candidates.insert(childIndex);
      if (beamSize)
        while (candidates.size() > beamSize)
          candidates.erase(--candidates.end());
    }

    // apply episode gradient
    DoubleVectorPtr parametersGradient;
    double gradientNorm = 0.0;
    for (size_t i = 0; i < episodeGradient.size(); ++i)
    {
      double g = episodeGradient[i];
      if (g)
      {
        scoringFunction->addGradient(g, nodeFeatures[i], parametersGradient, 1.0);
        gradientNorm += g * g;
      }
    }
    GradientDescentOnlineLearnerPtr onlineLearner = scoringFunction->getOnlineLearner().dynamicCast<GradientDescentOnlineLearner>();
    jassert(onlineLearner);
    if (parametersGradient)
      onlineLearner->addComputedGradient(scoringFunction, parametersGradient, lossValue);

    this->selectedNodesCost.push(selectedNodesCostSum / (numOpenedNodes - 1));
    this->gradientNorm.push(sqrt(gradientNorm));
    this->bestReturn.push(searchTree->getBestReturn());
  }
#endif // 0

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    DoubleVectorPtr parameters = scoringFunction->getParameters();

    //context->resultCallback(T("Best Return"), bestReturn.getMean());
    //context->resultCallback(T("Mean Selected Node Cost"), selectedNodesCost.getMean());
    context->resultCallback(T("Episode Gradient Norm"), gradientNorm.getMean());

    objectiveValueToMinimize = -bestReturn.getMean();
    selectedNodesCost.clear();
    gradientNorm.clear();
    bestReturn.clear();
    return false;
  }

private:
  friend class SearchFunctionOnlineLearnerClass;

  ExecutionContext* context;
  RankingLossFunctionPtr rankingLoss;

  SearchFunctionPtr searchFunction;
  BestFirstSearchPolicyPtr searchPolicy;
  FunctionPtr featuresFunction;
  NumericalLearnableFunctionPtr scoringFunction;

  ScalarVariableStatistics selectedNodesCost;
  ScalarVariableStatistics gradientNorm;
  ScalarVariableStatistics bestReturn;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_FUNCTION_ONLINE_LEARNER_H_

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

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& f, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
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
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    const SearchTreePtr& searchSpace = output.getObjectAndCast<SearchTree>();

    size_t numOpenedNodes = searchSpace->getNumOpenedNodes();
    size_t beamSize = searchFunction->getMaxSearchNodes();
    if (searchPolicy.dynamicCast<BeamSearchPolicy>())
      beamSize = searchPolicy.staticCast<BeamSearchPolicy>()->getBeamSize();

    std::set<size_t> candidates;
    candidates.insert(0);
    std::vector<double> episodeGradient(searchSpace->getNumNodes(), 0.0);
    double selectedNodesCostSum = 0.0;
    for (size_t i = 0; i < numOpenedNodes; ++i)
    {
      size_t selectedNodeIndex = searchSpace->getOpenedNodeIndex(i);

      if (i > 0)
      {
        jassert(candidates.size() > 1);
        std::vector<double> scores(candidates.size());
        std::vector<double> costs(candidates.size());

        size_t c = 0;
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
        {
          SearchTreeNodePtr node = searchSpace->getNode(*it);
          scores[c] = node->getHeuristicScore();

          double cost = node->getParentNode()->getBestReturn() - node->getBestReturn();

          //double cost = node->getParentNode()->getBestReturnWithoutChild(node) - node->getParentNode()->getBestReturn();
          //double cost = node->getParentNode()->getBestReturn() - node->getBestReturn();
            //(node == node->getParentNode()->getBestChildNode()) ? 0.0 : 1.0; // bipartite ranking for the moment
          costs[c] = cost;
          if (*it == selectedNodeIndex)
            selectedNodesCostSum += cost;
        }

        // compute ranking loss
        std::vector<double> rankingLossGradient(candidates.size(), 0.0);
        rankingLoss->computeRankingLoss(scores, costs, NULL, &rankingLossGradient);

        // update episode gradient
        c = 0;
        double invZ = 1.0 / (double)candidates.size();
        for (std::set<size_t>::const_iterator it = candidates.begin(); it != candidates.end(); ++it, ++c)
          episodeGradient[*it] += invZ * rankingLossGradient[c];
      }

      // update candidates list
      candidates.erase(selectedNodeIndex);
      SearchTreeNodePtr node = searchSpace->getNode(selectedNodeIndex);
      int begin = node->getChildBeginIndex();
      if (begin >= 0)
        for (int childIndex = begin; childIndex < node->getChildEndIndex(); ++childIndex)
          candidates.insert(childIndex);
      if (beamSize)
        while (candidates.size() > beamSize)
          candidates.erase(--candidates.end());
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

    context->resultCallback(T("Best Return"), bestReturn.getMean());
    context->resultCallback(T("Mean Selected Node Cost"), selectedNodesCost.getMean());
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

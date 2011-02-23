/*-----------------------------------------.---------------------------------.
| Filename: LookAheadTreeSearchOnlineLearner.h | Online Learner              |
| Author  : Francis Maes                   |                                 |
| Started : 23/02/2011 12:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_ONLINE_LEARNER_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_ONLINE_LEARNER_H_

# include "LookAHeadTreeSearchFunction.h"
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class LookAheadTreeSearchOnlineLearner : public OnlineLearner
{
public:
  LookAheadTreeSearchOnlineLearner(RankingLossFunctionPtr rankingLoss = RankingLossFunctionPtr())
    : context(NULL), rankingLoss(rankingLoss)
  {
    if (!rankingLoss)
      this->rankingLoss = allPairsRankingLossFunction(hingeDiscriminativeLossFunction());
  }

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
          double cost = -node->getBestReturn();
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

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_LOOK_AHEAD_TREE_SEARCH_FUNCTION_H_

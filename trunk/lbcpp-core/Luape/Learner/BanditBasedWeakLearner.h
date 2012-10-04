/*-----------------------------------------.---------------------------------.
| Filename: BanditBasedWeakLearner.h       | Bandit Based Weak Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 22/12/2011 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_BANDIT_BASED_H_
# define LBCPP_LUAPE_WEAK_LEARNER_BANDIT_BASED_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <algorithm>
# include <queue> // for priority queue in bandits pool

namespace lbcpp
{

class BanditBasedWeakLearner : public NodeBuilderBasedLearner
{
public:
  BanditBasedWeakLearner(ExpressionBuilderPtr nodeBuilder, double relativeBudget, double miniBatchRelativeSize)
    : NodeBuilderBasedLearner(nodeBuilder), relativeBudget(relativeBudget), miniBatchRelativeSize(miniBatchRelativeSize)  {}
  BanditBasedWeakLearner() {}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const ExpressionDomainPtr& problem, const IndexSetPtr& examples)
  {
    // make initial weak learners
    std::vector<ExpressionPtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 0, weakNodes);
    if (!weakNodes.size())
      return ExpressionPtr();

    // reset current-episode information
    BanditsQueue banditsQueue;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      ArmInfo& arm = arms[weakNodes[i]];
      arm.episodeStats.clear();
      banditsQueue.push(std::make_pair(i, getArmScore(arm)));
    }

    // play bandits
    if (verbose)
      context.enterScope(T("Playing bandits"));
    std::vector<IndexSetPtr> subsets;

    size_t numTrainingSamples = problem->getTrainingCache()->getNumSamples();
    size_t miniBatchSize = (size_t)(miniBatchRelativeSize * numTrainingSamples + 0.5);
    size_t budget = (size_t)(relativeBudget * numTrainingSamples);

    size_t effectiveBudget = 0;
    size_t t;
    for (t = 0; effectiveBudget < budget; ++t)
    {
      // select arm
      size_t armIndex = banditsQueue.top().first;
      banditsQueue.pop();

      // retrieve arm info and evaluate weak node
      ExpressionPtr weakNode = weakNodes[armIndex];
      ArmInfo& arm = arms[weakNode];
      size_t subsetIndex = (size_t)arm.episodeStats.getCount();
      IndexSetPtr subset = getSubset(context, examples, subsetIndex, miniBatchSize, subsets);
      effectiveBudget += subset->size();
      double weakObjective = objective->computeObjectiveWithEventualStump(context, problem, weakNode, subset); // side effect on weakNode (that we do not keep)
      weakObjective *= numTrainingSamples / (double)subset->size();
      // todo: here, we can retrieve the best threshold if it is a stump

      // update arm statistics
      arm.episodeStats.push(weakObjective);
      if (subset->size() < examples->size()) // once the arm has been evaluated on all examples it is put on side
        banditsQueue.push(std::make_pair(armIndex, getArmScore(arm)));
      //context.informationCallback(T("Arm: ") + String((int)armIndex) + T(", Node: ") + weakNodes[armIndex]->toShortString() + T(", EpStats: ") + arm.episodeStats.toShortString() + T(" tk = ") + String(arm.episodeStats.getCount()));
    }

    // finalize
    ExpressionPtr bestWeakNode;
    double bestWeakObjective = -DBL_MAX;
    std::multimap<double, ExpressionPtr> sortedNodes;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      const ExpressionPtr& weakNode = weakNodes[i];
      ArmInfo& arm = arms[weakNodes[i]];
      double empiricalWeakObjective = arm.episodeStats.getMean();
      arm.previousEpisodesScore += arm.episodeStats.getCount();

      sortedNodes.insert(std::make_pair(empiricalWeakObjective, weakNode));
      if (empiricalWeakObjective > bestWeakObjective)
        bestWeakObjective = empiricalWeakObjective, bestWeakNode = weakNode;
    }

    // display ten best arms
    if (verbose)
    {
#ifndef JUCE_MAC    
      size_t index = 0;
      for (std::multimap<double, ExpressionPtr>::const_reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend() && index < 10; ++it, ++index)
        context.informationCallback(T("[") + String(it->first) + T("]: ") + it->second->toShortString() + T(" (tk = ") + String(arms[it->second].episodeStats.getCount()) + T(")"));
#endif
    }

    bestObjectiveValue = objective->computeObjectiveWithEventualStump(context, problem, bestWeakNode, examples); // side effect on bestWeakNode
    if (verbose)
    {
      context.informationCallback(T("Num Steps: ") + String((int)t) + T(" Effective budget: ") + String((int)effectiveBudget) + T(" normalized = ") + String((double)effectiveBudget / problem->getTrainingCache()->getNumSamples()));
      context.leaveScope(bestObjectiveValue);
    }
    return bestWeakNode;
  }

protected:
  friend class BanditBasedWeakLearnerClass;

  double relativeBudget;
  double miniBatchRelativeSize;

  IndexSetPtr getSubset(ExecutionContext& context, const IndexSetPtr& examples, size_t index, size_t miniBatchSize, std::vector<IndexSetPtr>& subsets)
  {
    size_t subsetSize = miniBatchSize * (index + 1);
    if (subsetSize >= examples->size())
      return examples;

    if (index >= subsets.size())
    {
      size_t size = subsets.size();
      subsets.resize(index + 1);
      for (size_t i = size; i < subsets.size(); ++i)
      {
        IndexSetPtr subset;
        if (i == 0)
          subset = new IndexSet();
        else
          subset = subsets[i - 1]->cloneAndCast<IndexSet>();
        subset = examples->sampleSubset(context.getRandomGenerator(), miniBatchSize * (i + 1));
        subsets[i] = subset;
      }
    }
    return subsets[index];
  }

  struct ArmInfo
  {
    ArmInfo() : previousEpisodesScore(0.0) {}

    ScalarVariableStatistics episodeStats;
    double previousEpisodesScore;
  };

  typedef std::map<ExpressionPtr, ArmInfo> ArmMap;
  ArmMap arms;

  double getArmScore(const ArmInfo& arm) const
  {
    static const double C1 = 0.1;
    //static const double C2 = 0.1;
    if (!arm.episodeStats.getCount())
      return DBL_MAX;

    double res = arm.episodeStats.getMean() + C1 / arm.episodeStats.getCount();
    //if (arm.previousEpisodesScore)
    //  res -= C2 / arm.previousEpisodesScore;
    return res;
  }

  struct BanditScoresComparator
  {
    bool operator()(const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      if (left.second != right.second)
        return left.second < right.second;
      else
        return left.first < right.first;
    }
  };
  
  typedef std::priority_queue<std::pair<size_t, double>, std::vector<std::pair<size_t, double> >, BanditScoresComparator  > BanditsQueue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_

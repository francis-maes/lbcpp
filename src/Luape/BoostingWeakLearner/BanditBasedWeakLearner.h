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

class BanditBasedWeakLearner : public DecoratorBoostingWeakLearner
{
public:
  BanditBasedWeakLearner(BoostingWeakLearnerPtr weakLearner, double relativeBudget, double miniBatchRelativeSize)
    : DecoratorBoostingWeakLearner(weakLearner), relativeBudget(relativeBudget), miniBatchRelativeSize(miniBatchRelativeSize)  {}
  BanditBasedWeakLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective)
  {
    // make initial weak learners
    std::vector<LuapeNodePtr> weakNodes;
    if (!getDecoratedCandidateWeakNodes(context, structureLearner, weakNodes))
      return LuapeNodePtr();

    // reset current-episode information
    BanditsQueue banditsQueue;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      ArmInfo& arm = arms[weakNodes[i]];
      arm.episodeStats.clear();
      banditsQueue.push(std::make_pair(i, getArmScore(arm)));
    }

    // play bandits
    context.enterScope(T("Playing bandits"));
    std::vector<IndexSetPtr> subsets;

    size_t numTrainingSamples = structureLearner->getTrainingCache()->getNumSamples();
    size_t miniBatchSize = (size_t)(miniBatchRelativeSize * numTrainingSamples + 0.5);
    size_t budget = (size_t)(relativeBudget * weakNodes.size() * numTrainingSamples);

    size_t effectiveBudget = 0;
    size_t t;
    for (t = 0; effectiveBudget < budget; ++t)
    {
      // select arm
      size_t armIndex = banditsQueue.top().first;
      banditsQueue.pop();

      // retrieve arm info and evaluate weak node
      LuapeNodePtr weakNode = weakNodes[armIndex];
      ArmInfo& arm = arms[weakNode];
      size_t subsetIndex = (size_t)arm.episodeStats.getCount();
      IndexSetPtr subset = getSubset(context, examples, subsetIndex, miniBatchSize, subsets);
      effectiveBudget += subset->size();
      double weakObjective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, subset); // side effect on weakNode (that we do not keep)
      weakObjective *= numTrainingSamples / (double)subset->size();
      // todo: here, we can retrieve the best threshold if it is a stump

      // update arm statistics
      arm.episodeStats.push(weakObjective);
      if (subset->size() < examples->size()) // once the arm has been evaluated on all examples it is put on side
        banditsQueue.push(std::make_pair(armIndex, getArmScore(arm)));
      //context.informationCallback(T("Arm: ") + String((int)armIndex) + T(", Node: ") + weakNodes[armIndex]->toShortString() + T(", EpStats: ") + arm.episodeStats.toShortString() + T(" tk = ") + String(arm.episodeStats.getCount()));
    }

    // finalize
    LuapeNodePtr bestWeakNode;
    double bestWeakObjective = -DBL_MAX;
    std::multimap<double, LuapeNodePtr> sortedNodes;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      const LuapeNodePtr& weakNode = weakNodes[i];
      double empiricalWeakObjective = arms[weakNodes[i]].episodeStats.getMean();
      sortedNodes.insert(std::make_pair(empiricalWeakObjective, weakNode));
      if (empiricalWeakObjective > bestWeakObjective)
        bestWeakObjective = empiricalWeakObjective, bestWeakNode = weakNode;
    }

    // display ten best arms
    size_t index = 0;
    for (std::multimap<double, LuapeNodePtr>::const_reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend() && index < 10; ++it, ++index)
      context.informationCallback(T("[") + String(it->first) + T("]: ") + it->second->toShortString() + T(" (tk = ") + String(arms[it->second].episodeStats.getCount()) + T(")"));


    weakObjective = computeWeakObjectiveWithEventualStump(context, structureLearner, bestWeakNode, examples); // side effect on bestWeakNode
    context.informationCallback(T("Num Steps: ") + String((int)t) + T(" Effective budget: ") + String((int)effectiveBudget) + T(" normalized = ") + String((double)effectiveBudget / (weakNodes.size() * structureLearner->getTrainingCache()->getNumSamples())));
    context.leaveScope(weakObjective);
    return makeContribution(context, structureLearner, bestWeakNode, weakObjective, examples);
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
        subset->randomlyExpandUsingSource(context, miniBatchSize * (i + 1), examples);
        subsets[i] = subset;
      }
    }
    return subsets[index];
  }

  struct ArmInfo
  {
    // todo: previous episodes statistics
    ScalarVariableStatistics episodeStats;
  };

  typedef std::map<LuapeNodePtr, ArmInfo> ArmMap;
  ArmMap arms;

  double getArmScore(const ArmInfo& arm) const
  {
    static const double C = 0.005;
    if (!arm.episodeStats.getCount())
      return DBL_MAX;
    return arm.episodeStats.getMean() + C / arm.episodeStats.getCount();
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

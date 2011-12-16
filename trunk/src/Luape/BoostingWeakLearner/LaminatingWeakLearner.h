/*-----------------------------------------.---------------------------------.
| Filename: LaminatingWeakLearner.h        | Laminating Weak Learner         |
| Author  : Francis Maes                   | from dubout & fleuret, 2011     |
| Started : 16/12/2011 17:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_
# define LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <algorithm>

namespace lbcpp
{

class LaminatingWeakLearner : public BoostingWeakLearner
{
public:
  LaminatingWeakLearner(BoostingWeakLearnerPtr weakLearner, size_t numInitialExamples)
    : weakLearner(weakLearner), numInitialExamples(numInitialExamples)  {}
  LaminatingWeakLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
    {return weakLearner->initialize(context, function);}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const
  {
    context.enterScope(T("Generating candidate weak learners"));
    // make initial weak learners
    std::vector<LuapeNodePtr> weakNodes;
    bool ok = weakLearner->getCandidateWeakNodes(context, structureLearner, weakNodes);
    context.leaveScope(weakNodes.size());
    if (!ok)
    {
      context.errorCallback(T("Could not get finite set of candidate weak nodes"));
      return false;
    }

    context.enterScope("Laminating");
    std::vector<std::pair<LuapeNodePtr, double> > weakNodesByScore;
    weakNodesByScore.resize(weakNodes.size());
    for (size_t i = 0; i < weakNodes.size(); ++i)
      weakNodesByScore[i] = std::make_pair(weakNodes[i], 0.0);

    // make initial examples subset
    std::vector<size_t> examplesOrder;
    context.getRandomGenerator()->sampleOrder(examples.size(), examplesOrder);
    std::vector<size_t> examplesSubset;
    examplesSubset.reserve(examples.size());
    examplesSubset.resize(numInitialExamples);
    for (size_t i = 0; i < examplesSubset.size(); ++i)
      examplesSubset[i] = examples[examplesOrder[i]];

    // laminating main loop
    size_t numWeakLearners = weakNodes.size();
    for (size_t iteration = 1; numWeakLearners > 1; ++iteration)
    {
      context.enterScope(String((int)numWeakLearners) + T(" weak learners, ") + String((int)examplesSubset.size()) + T(" examples"));

      // evaluate weak nodes
      for (size_t i = 0; i < numWeakLearners; ++i)
      {
        LuapeNodePtr weakNode = weakNodesByScore[i].first;
        weakNodesByScore[i].second = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, examplesSubset); // side effect on weakNode (that we do not keep)
      }
      // sort by decreasing score
      std::sort(weakNodesByScore.begin(), weakNodesByScore.begin() + numWeakLearners, SortDoubleValuesOperator());

      // results
      context.resultCallback(T("iteration"), iteration);
      context.resultCallback(T("numWeakLearners"), numWeakLearners);
      context.resultCallback(T("numExamples"), examplesSubset.size());
      context.resultCallback(T("bestWeakNode"), weakNodesByScore[0].first);
      context.resultCallback(T("bestWeakObjective"), weakNodesByScore[0].second);
      context.leaveScope();

      // update num weak learners
      numWeakLearners = numWeakLearners / 2;

      // grow examples subset
      size_t previousNumExamples = examplesSubset.size();
      if (previousNumExamples == examples.size())
        break; // all examples have been included in the last evaluation, so the current best weak node is the final best weak node
      
      size_t numExamples = previousNumExamples * 2;
      if (numExamples > examples.size())
        numExamples = examples.size();
      examplesSubset.resize(numExamples);
      for (size_t i = previousNumExamples; i < numExamples; ++i)
        examplesSubset[i] = examples[examplesOrder[i]];
    }
    LuapeNodePtr weakNode = weakNodesByScore[0].first;
    weakObjective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, examples); // side effect on weakNode
    context.leaveScope(weakObjective);
    return makeContribution(context, structureLearner, weakNode, examples);
  }

protected:
  friend class LaminatingWeakLearnerClass;

  BoostingWeakLearnerPtr weakLearner;
  size_t numInitialExamples;

  struct SortDoubleValuesOperator
  {
    bool operator()(const std::pair<LuapeNodePtr, double>& a, const std::pair<LuapeNodePtr, double>& b) const
      {return a.second == b.second ? a.first < b.first : a.second > b.second;} // decreasing sort
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_

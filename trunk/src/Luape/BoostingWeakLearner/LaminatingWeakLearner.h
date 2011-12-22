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

class LaminatingWeakLearner : public DecoratorBoostingWeakLearner
{
public:
  LaminatingWeakLearner(BoostingWeakLearnerPtr weakLearner, double relativeBudget)
    : DecoratorBoostingWeakLearner(weakLearner), relativeBudget(relativeBudget)  {}
  LaminatingWeakLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective)
  {
    context.enterScope("Laminating");

    // make initial weak learners
    std::vector<LuapeNodePtr> weakNodes;
    if (!getDecoratedCandidateWeakNodes(context, structureLearner, weakNodes))
      return LuapeNodePtr();
    std::vector<std::pair<LuapeNodePtr, double> > weakNodesByScore;
    weakNodesByScore.resize(weakNodes.size());
    for (size_t i = 0; i < weakNodes.size(); ++i)
      weakNodesByScore[i] = std::make_pair(weakNodes[i], 0.0);

    // compute number of initial examples
    // W = numWeak, N = num examples
    // W * N0 * log2(W) = relativeBudget * N
    // N0 = relativeBudget * N / (W * log2(W))
    double N = (double)structureLearner->getTrainingCache()->getNumSamples();
    double W = (double)weakNodes.size();
    size_t numInitialExamples = (size_t)(relativeBudget * N / (W * log2(W)));
    if (numInitialExamples < 2)
      numInitialExamples = 2;

    size_t effectiveBudget = 0;
    IndexSetPtr examplesSubset;
    if (numInitialExamples >= examples->size())
      examplesSubset = examples;
    else
    {
      // make initial examples subset
      examplesSubset = new IndexSet();
      examplesSubset->randomlyExpandUsingSource(context, numInitialExamples, examples);
    }

    // laminating main loop
    size_t numWeakLearners = weakNodes.size();
    for (size_t iteration = 1; numWeakLearners > 1; ++iteration)
    {
      context.enterScope(String((int)numWeakLearners) + T(" weak learners, ") + String((int)examplesSubset->size()) + T(" examples"));

      // evaluate weak nodes
      for (size_t i = 0; i < numWeakLearners; ++i)
      {
        LuapeNodePtr weakNode = weakNodesByScore[i].first;
        double objective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, examplesSubset); // side effect on weakNode (that we do not keep)
        weakNodesByScore[i].second = objective;
      }
      effectiveBudget += numWeakLearners * examplesSubset->size();
      // sort by decreasing score
      std::sort(weakNodesByScore.begin(), weakNodesByScore.begin() + numWeakLearners, SortDoubleValuesOperator());

      // results
      context.resultCallback(T("iteration"), iteration);
      context.resultCallback(T("numWeakLearners"), numWeakLearners);
      context.resultCallback(T("numExamples"), examplesSubset->size());
      context.resultCallback(T("bestWeakNode"), weakNodesByScore[0].first);
      context.resultCallback(T("bestWeakObjective"), weakNodesByScore[0].second);
      context.leaveScope();

      // update num weak learners
      numWeakLearners = numWeakLearners / 2;

      // grow examples subset
      if (examplesSubset == examples)
        break; // all examples have been included in the last evaluation, so the current best weak node is the final best weak node
      
      size_t numExamples = examplesSubset->size() * 2;
      if (numExamples > examples->size())
      {
        numExamples = examples->size();
        examplesSubset = examples;
      }
      else
        examplesSubset->randomlyExpandUsingSource(context, numExamples, examples);
    }
    LuapeNodePtr weakNode = weakNodesByScore[0].first;
    weakObjective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, examples); // side effect on weakNode
    context.informationCallback(T("Effective budget: ") + String((int)effectiveBudget) + T(" normalized = ") + String((double)effectiveBudget / structureLearner->getTrainingCache()->getNumSamples()));
    context.leaveScope(weakObjective);
    return makeContribution(context, structureLearner, weakNode, weakObjective, examples);
  }

protected:
  friend class LaminatingWeakLearnerClass;

  double relativeBudget;

  struct SortDoubleValuesOperator
  {
    bool operator()(const std::pair<LuapeNodePtr, double>& a, const std::pair<LuapeNodePtr, double>& b) const
      {return a.second == b.second ? a.first < b.first : a.second > b.second;} // decreasing sort
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_

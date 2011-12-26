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

  enum {minNumExamples = 5};

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective)
  {
    if (verbose)
      context.enterScope("Laminating");

    // make initial weak learners
    std::vector<LuapeNodePtr> weakNodes;
    if (!getDecoratedCandidateWeakNodes(context, structureLearner, weakNodes))
      return LuapeNodePtr();

    size_t W0, N0;
    double totalBudget = relativeBudget * structureLearner->getTrainingCache()->getNumSamples();
    determineInitialCounts(totalBudget, weakNodes.size(), examples->size(), W0, N0);
    if (!W0 || !N0)
    {
      if (verbose)
        context.leaveScope(false);
      return LuapeNodePtr();
    }
    jassert(N0 >= minNumExamples && N0 <= examples->size() && W0 <= weakNodes.size());

    // create initial examples subset
    size_t effectiveBudget = 0;
    IndexSetPtr examplesSubset;
    if (N0 == examples->size())
      examplesSubset = examples;
    else
    {
      // make initial examples subset
      examplesSubset = new IndexSet();
      examplesSubset->randomlyExpandUsingSource(context, N0, examples);
    }

    // create initial weak learners subset
    std::vector<std::pair<LuapeNodePtr, double> > weakNodesByScore(W0);
    std::vector<size_t> order;
    if (W0 < weakNodes.size())
      context.getRandomGenerator()->sampleOrder(weakNodes.size(), order);
    for (size_t i = 0; i < W0; ++i)
    {
      size_t index = order.size() ? order[i] : i;
      weakNodesByScore[i] = std::make_pair(weakNodes[index], 0.0);
    }

    // laminating main loop
    size_t numWeakLearners = W0;
    for (size_t iteration = 1; numWeakLearners > 1; ++iteration)
    {
      if (verbose)
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
      if (verbose)
      {
        context.resultCallback(T("iteration"), iteration);
        context.resultCallback(T("numWeakLearners"), numWeakLearners);
        context.resultCallback(T("numExamples"), examplesSubset->size());
        context.resultCallback(T("bestWeakNode"), weakNodesByScore[0].first);
        context.resultCallback(T("bestWeakObjective"), weakNodesByScore[0].second);
        context.leaveScope();
      }

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
    if (verbose)
    {
      context.informationCallback(T("Effective budget: ") + String((int)effectiveBudget) + T(" / ") + String(totalBudget) + T(" normalized = ") + String((double)effectiveBudget / totalBudget));
      context.leaveScope(weakObjective);
    }
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

  static double computeBudget(size_t W0, size_t N0, double Wmin, double Nmax, size_t totalNumExamples)
  {
    if (!W0 || !N0)
      return 0.0;
    size_t numIterationsWrtN = 1 + (size_t)ceil(Nmax - log2((double)N0));
    size_t numIterationsWrtW = (size_t)(log2((double)W0) - Wmin);
    size_t estimatedNumIterations = numIterationsWrtN < numIterationsWrtW ? numIterationsWrtN : numIterationsWrtW;

#if 0
    size_t numExamples = N0;
    size_t numWeakLearners = W0;
    size_t numIterations = 0;
    for (size_t iteration = 1; numWeakLearners > 1; ++iteration)
    {
      ++numIterations;

      numWeakLearners = numWeakLearners / 2;
      if (numExamples == totalNumExamples)
        break; 
      numExamples *= 2;
      if (numExamples > totalNumExamples)
        numExamples = totalNumExamples;
    }
    jassert(estimatedNumIterations == numIterations);
#endif // 0

    return estimatedNumIterations * W0 * N0;
  }

  void determineInitialCounts(double budget, size_t numWeakNodes, size_t numExamples, size_t& W0, size_t& N0)
  {
    static const double Wmin = 0.0;
    const double Nmax = log2((double)numExamples);

    // first try to take all weak nodes
    W0 = numWeakNodes;
    N0 = (size_t)(budget / (W0 * log2((double)W0))); // lower bound on N0
    jassert(computeBudget(W0, N0, Wmin, Nmax, numExamples) <= budget);
    while (computeBudget(W0, N0 + 1, Wmin, Nmax, numExamples) < budget)
      ++N0;

    if (N0 < minNumExamples)
    {
      // we cannot use all weak nodes, select a subset of them
      N0 = minNumExamples;
      W0 = (size_t)(budget / (N0 * (Nmax - log2((double)N0) + 1))); // lower bound on W0
      jassert(computeBudget(W0, N0, Wmin, Nmax, numExamples) <= budget);
      while (computeBudget(W0 + 1, N0, Wmin, Nmax, numExamples) < budget)
        ++W0;
    }

    jassert(computeBudget(W0, N0, Wmin, Nmax, numExamples) <= budget);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_LAMINATING_H_

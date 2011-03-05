/*-----------------------------------------.---------------------------------.
| Filename: SearchPolicy.h                 | Search policies                 |
| Author  : Francis Maes                   |                                 |
| Started : 05/03/2011 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_POLICY_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_POLICY_H_

# include "Policy.h"
# include "SearchTree.h"

namespace lbcpp
{

class SearchPolicy : public Policy, public SearchTreeCallback
{
public:
  virtual void searchStart(ExecutionContext& context, const SearchTreePtr& searchTree) {}
  virtual size_t searchNext(ExecutionContext& context, const SearchTreePtr& searchTree) = 0;

  virtual void candidateAdded(ExecutionContext& context, const SearchTreePtr& searchTree, size_t nodeIndex) {}

public:
  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions)
  {
    const SearchTreePtr& tree = state.getObjectAndCast<SearchTree>();
    searchStart(context, tree);

    tree->addCallback(this);
    jassert(tree->getNumNodes() == 1);
    candidateAdded(context, tree, 0);

    return searchNext(context, tree);
  }

  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions)
  {
    const SearchTreePtr& tree = state.getObjectAndCast<SearchTree>();
    return searchNext(context, tree);
  }

   virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState)
   {
     const SearchTreePtr& tree = finalState.getObjectAndCast<SearchTree>();
     tree->removeCallback(this);
   }
};

typedef ReferenceCountedObjectPtr<SearchPolicy> SearchPolicyPtr;

extern SearchPolicyPtr randomSearchPolicy();
extern SearchPolicyPtr bestFirstSearchPolicy(FunctionPtr heuristic);
extern SearchPolicyPtr beamSearchPolicy(FunctionPtr heuristic, size_t beamSize);

class RandomSearchPolicy : public SearchPolicy
{
public:
  virtual size_t searchNext(ExecutionContext& context, const SearchTreePtr& searchTree)
  {
    RandomGeneratorPtr random = RandomGenerator::getInstance();

    // in average, there should be more nodes to explore, than already explored nodes
    // this loop should thus terminate relatively quick
    while (true)
    {
      size_t index = random->sampleSize(searchTree->getNumNodes());
      if (!searchTree->getNode(index)->isExplored())
        return index;
    }
    return 0;
  }
};

class BestFirstSearchPolicy : public SearchPolicy
{
public:
  BestFirstSearchPolicy(FunctionPtr heuristic = FunctionPtr())
    : heuristic(heuristic) {}

  virtual void candidateAdded(ExecutionContext& context, const SearchTreePtr& searchTree, size_t nodeIndex)
  {
    double scoreToMinimize = computeScoreToMinimize(context, searchTree->getNode(nodeIndex));
    candidates.insert(std::make_pair(scoreToMinimize, nodeIndex));
  }

  virtual void searchStart(ExecutionContext& context, const SearchTreePtr& searchTree)
  {
    candidates.clear();
  }

  virtual size_t searchNext(ExecutionContext& context, const SearchTreePtr& searchTree)
  {
    if (candidates.empty())
    {
      context.errorCallback(T("No more candidates to explore"));
      return (size_t)-1;
    }
    return popBestCandidate();
  }

  const FunctionPtr& getHeuristic() const
    {return heuristic;}

protected:
  friend class BestFirstSearchPolicyClass;

  FunctionPtr heuristic;
  std::multimap<double, size_t> candidates;

  size_t popBestCandidate()
  {
    jassert(candidates.size());
    size_t res = candidates.begin()->second;
    candidates.erase(candidates.begin());
    return res;
  }

  double computeScoreToMinimize(ExecutionContext& context, const SearchTreeNodePtr& node) const
    {return -heuristic->compute(context, Variable(node, searchTreeNodeClass)).getDouble();}
};

typedef ReferenceCountedObjectPtr<BestFirstSearchPolicy> BestFirstSearchPolicyPtr;

class BeamSearchPolicy : public BestFirstSearchPolicy
{
public:
  BeamSearchPolicy(FunctionPtr heuristic, size_t beamSize)
    : BestFirstSearchPolicy(heuristic), beamSize(beamSize) {}
  BeamSearchPolicy() : beamSize(0) {}

  virtual void searchStart(ExecutionContext& context, const SearchTreePtr& searchTree)
  {
    worstScore = DBL_MAX;
    BestFirstSearchPolicy::searchStart(context, searchTree);
  }

  virtual void candidateAdded(ExecutionContext& context, const SearchTreePtr& searchTree, size_t nodeIndex)
  {
    double scoreToMinimize = computeScoreToMinimize(context, searchTree->getNode(nodeIndex));
    if (candidates.size() >= beamSize && scoreToMinimize > worstScore)
      return; // score is not good enough

    candidates.insert(std::make_pair(scoreToMinimize, nodeIndex));

    if (candidates.size() > beamSize)
    {
      jassert(candidates.size() == beamSize + 1);
      candidates.erase(--candidates.end());
      worstScore = candidates.rbegin()->first;
    }
  }

  size_t getBeamSize() const
    {return beamSize;}

protected:
  friend class BeamSearchPolicyClass;

  size_t beamSize;
  double worstScore;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_POLICY_H_

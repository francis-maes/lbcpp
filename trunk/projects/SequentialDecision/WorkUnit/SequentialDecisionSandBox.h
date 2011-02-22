/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionSandBox.h    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

# include "../System/LinearPointPhysicSystem.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class SearchSpaceNode;
typedef ReferenceCountedObjectPtr<SearchSpaceNode> SearchSpaceNodePtr;

class SearchSpaceNode : public Object
{
public:
  SearchSpaceNode(const Variable& initialState = Variable())
    : state(initialState), depth(0), reward(0.0), currentReturn(0.0),
      parentIndex(-1), childrenBeginIndex(-1), childrenEndIndex(-1) {}

  SearchSpaceNode(const SearchSpaceNodePtr& parentNode, size_t parentIndex, const Variable& action)
    : depth(parentNode->depth + 1), previousAction(action), reward(0.0), currentReturn(parentNode->currentReturn),
      parentIndex((int)parentIndex), childrenBeginIndex(-1), childrenEndIndex(-1)
  {
  }

  const Variable& getState() const
    {return state;}

  size_t getDepth() const
    {return depth;}
  
  double getCurrentReturn() const
    {return currentReturn;}

  void openNode(const SequentialDecisionSystemPtr& system, const SearchSpaceNodePtr& parentNode, double discount)
  {
    if (!state.exists())
    {
      jassert(parentNode && depth >= 1);

      state = system->computeTransition(parentNode->state, previousAction);
      reward = system->computeReward(parentNode->state, previousAction, state);
      jassert(state.exists());
      if (reward)
        currentReturn += reward * pow(discount, (double)(depth - 1));
    }
  }

  void setChildrenIndices(size_t begin, size_t end)
    {childrenBeginIndex = (int)begin; childrenEndIndex = (int)end;}

  int getParentIndex() const
    {return parentIndex;}

protected:
  friend class SearchSpaceNodeClass;

  Variable state;
  size_t depth;

  Variable previousAction;
  double reward; // received when entering state
  double currentReturn; // received from the beginning until entering state

  int parentIndex;
  int childrenBeginIndex;
  int childrenEndIndex;
};

extern ClassPtr searchSpaceNodeClass;

class SearchHeuristic : public SimpleUnaryFunction
{
public:
  SearchHeuristic() : SimpleUnaryFunction(searchSpaceNodeClass, doubleType) {}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeHeuristic(input.getObjectAndCast<SearchSpaceNode>());}
};

typedef ReferenceCountedObjectPtr<SearchHeuristic> SearchHeuristicPtr;

class SortedSearchSpace : public Object
{
public:
  SortedSearchSpace(SequentialDecisionSystemPtr system, SearchHeuristicPtr heuristic, double discount, const Variable& initialState)
    : system(system), heuristic(heuristic), discount(discount)
  {
    addCandidate(new SearchSpaceNode(initialState));
  }
  SortedSearchSpace() {}

  // returns the current return
  double exploreBestNode(ExecutionContext& context)
  {
    if (candidates.empty())
    {
      context.errorCallback(T("No more candidates to explore"));
      return 0.0;
    }

    size_t nodeIndex;
    SearchSpaceNodePtr node = popBestCandidate(nodeIndex);
    jassert(node);
    int parentIndex = node->getParentIndex();
    SearchSpaceNodePtr parentNode = (parentIndex >= 0 ? nodes[parentIndex] : SearchSpaceNodePtr());
    
    node->openNode(system, parentNode, discount);

    std::vector<Variable> actions;
    system->getAvailableActions(node->getState(), actions);
    size_t firstChildIndex = nodes.size();
    node->setChildrenIndices(firstChildIndex, firstChildIndex + actions.size());

    nodes.reserve(nodes.size() + actions.size());
    for (size_t i = 0; i < actions.size(); ++i)
      addCandidate(new SearchSpaceNode(node, nodeIndex, actions[i]));

    return node->getCurrentReturn();
  }

  void addCandidate(SearchSpaceNodePtr node)
  {
    size_t index = nodes.size();
    nodes.push_back(node);
    candidates.insert(std::make_pair(-heuristic->computeHeuristic(node), index));
  }

  SearchSpaceNodePtr popBestCandidate(size_t& nodeIndex)
  {
    nodeIndex = candidates.begin()->second;
    candidates.erase(candidates.begin());
    return nodes[nodeIndex];
  }

private:
  SequentialDecisionSystemPtr system;
  SearchHeuristicPtr heuristic;
  double discount;
  std::vector<SearchSpaceNodePtr> nodes;
  std::multimap<double, size_t> candidates;
};

typedef ReferenceCountedObjectPtr<SortedSearchSpace> SortedSearchSpacePtr;

///////////////////////////////////////////////

class MinDepthSearchHeuristic : public SearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return -(double)node->getDepth();}
};

class OptimisticPlanningSearchHeuristic : public SearchHeuristic
{
public:
  OptimisticPlanningSearchHeuristic(double discountFactor = 0.9) : discountFactor(discountFactor)
    {jassert(discountFactor > 0.0 && discountFactor < 1.0);}

  virtual double computeHeuristic(const SearchSpaceNodePtr& node) const
    {return node->getCurrentReturn() + pow(discountFactor, (double)node->getDepth()) / (1.0 - discountFactor);}

private:
  friend class OptimisticPlanningSearchHeuristicClass;

  double discountFactor;
};

/////////////////////////////////////////////////

class SequentialDecisionSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    SequentialDecisionSystemPtr system = linearPointPhysicSystem();
    if (!system->initialize(context))
      return false;
    
    static const double discountFactor = 0.9;
    static const size_t numSamples = 1000;

    double maximumReturn = 1.0 / (1.0 - discountFactor);

    for (size_t d = 2; d <= 18; ++d)
    {
      context.enterScope(T("Computing scores for depth = ") + String((int)d));
      context.resultCallback(T("depth"), d);

      size_t maxSearchNodes = (size_t)pow(2.0, (double)(d + 1)) - 1;
      context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
      
      double minDepthScore = evaluateSearchHeuristic(context, system, new MinDepthSearchHeuristic(), maxSearchNodes, numSamples, discountFactor);
      context.resultCallback(T("uniform"), minDepthScore);
      context.resultCallback(T("uniformRegret"), maximumReturn - minDepthScore);
      
      double optimisticScore = evaluateSearchHeuristic(context, system, new OptimisticPlanningSearchHeuristic(discountFactor), maxSearchNodes, numSamples, discountFactor);
      context.resultCallback(T("optimistic"), optimisticScore);
      context.resultCallback(T("optimisticRegret"), maximumReturn - optimisticScore);

      context.leaveScope(true);
    }
    return true;
  }

  double evaluateSearchHeuristic(ExecutionContext& context, SequentialDecisionSystemPtr system, SearchHeuristicPtr heuristic, size_t maxSearchNodes, size_t numSamples, double discountFactor)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    ScalarVariableStatistics stats;
    for (size_t i = 0; i < numSamples; ++i)
    {
      Variable state = system->sampleInitialState(random);
      SortedSearchSpacePtr searchSpace = new SortedSearchSpace(system, heuristic, discountFactor, state);

      double highestReturn = 0.0;
      for (size_t j = 0; j < maxSearchNodes; ++j)
      {
        double value = searchSpace->exploreBestNode(context);
        if (value > highestReturn)
          highestReturn = value;
      }
      stats.push(highestReturn);
    }
    
    //context.informationCallback(stats.toString());
    return stats.getMean();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

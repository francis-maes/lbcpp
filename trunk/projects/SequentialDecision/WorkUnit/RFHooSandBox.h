/*-----------------------------------------.---------------------------------.
| Filename: RFHooSandBox.h                 | Random Forest HOO Optimization  |
| Author  : Francis Maes                   |  Algorithm                      |
| Started : 27/11/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_RF_HOO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_RF_HOO_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>

# include "../../../src/Optimizer/Optimizer/HOOOptimizer.h"

namespace lbcpp
{

class RFHOOOptimizerState : public OptimizerState
{
public:
  RFHOOOptimizerState(OptimizationProblemPtr problem,
                      size_t numTrees, size_t K, size_t nMin, size_t maxDepth,
                      size_t numIterations,
                      double nu, double rho, double C)
    : problem(problem), forest(numTrees), K(K), nMin(nMin), maxDepth(maxDepth),
      numIterations(numIterations), nu(nu), rho(rho), C(C), numIterationsDone(0)
  {
    MeasurableRegionPtr region;
    TypePtr inputType = problem->getSolutionsType();
    if (inputType->inheritsFrom(doubleType))
    {
      region = new ScalarMeasurableRegion(0.0, 1.0);
      if (!K)
        K = 1;
    }
    else if (inputType->inheritsFrom(denseDoubleVectorClass()))
    {
      DenseDoubleVectorPtr initial = problem->getInitialGuess().getObjectAndCast<DenseDoubleVector>();
      jassert(initial);
      DenseDoubleVectorPtr minValues = new DenseDoubleVector(inputType, initial->getNumValues(), 0.0);
      DenseDoubleVectorPtr maxValues = new DenseDoubleVector(inputType, initial->getNumValues(), 1.0);
      region = new VectorialMeasurableRegion(minValues, maxValues);
      if (!K)
        K = initial->getNumValues();
    }
    jassert(region);

    for (size_t i = 0; i < numTrees; ++i)
      forest[i] = new Node(region, 0);

    jassert((double)numIterations > 1.0 / (nu * nu));
  }

  RFHOOOptimizerState() {}

  size_t getNumIterationsDone() const
    {return numIterationsDone;}

  class Node;
  typedef ReferenceCountedObjectPtr<Node> NodePtr;

  NodePtr selectNode(ExecutionContext& context, const NodePtr& rootNode) const
  {
    NodePtr node = rootNode;
    while (node->depth < maxDepth && !node->isLeaf())
    {
      NodePtr left = node->left;
      NodePtr right = node->right;
      jassert(left && right);

      MeasurableRegionPtr region;
      if (right->B > left->B ||
          (right->B == left->B && context.getRandomGenerator()->sampleBool(0.5)))
        node = right;
      else
        node = left;
    }
    return node;
  }

  NodePtr selectNode(ExecutionContext& context) const
  {
    std::vector<size_t> bestTrees;
    double bestBValue = -DBL_MAX;
    for (size_t i = 0; i < forest.size(); ++i)
      if (forest[i]->B >= bestBValue)
      {
        if (forest[i]->B > bestBValue)
        {
          bestBValue = forest[i]->B;
          bestTrees.clear();
        }
        bestTrees.push_back(i);
      }
    jassert(bestTrees.size());
    size_t treeIndex = bestTrees.size() > 1 ? bestTrees[context.getRandomGenerator()->sampleSize(bestTrees.size())] : bestTrees[0];
    return selectNode(context, forest[treeIndex]);
  }

  double computeSplitScore(const NodePtr& node, const Variable& split)
  {
    ScalarVariableMeanAndVariance leftStats, rightStats;
    size_t n = node->samples.size();
    jassert(n == (size_t)node->stats.getCount());
    for (size_t i = 0; i < n; ++i)
    {
      double reward = node->samples[i].second;
      if (node->region->testSplit(split, node->samples[i].first))
        rightStats.push(reward);
      else
        leftStats.push(reward);
    }

    // From "Extremely randomized trees", Geurts et al.
    double varyS = node->stats.getVariance();
    jassert(varyS);
    return (varyS - leftStats.getCount() * leftStats.getVariance() / (double)n
                  - rightStats.getCount() * rightStats.getVariance() / (double)n) / varyS;
  }

  void splitNode(ExecutionContext& context, const NodePtr& node)
  {
    // sample candidate splits
    ContainerPtr splits = node->region->sampleSplits(context.getRandomGenerator(), K);

    // find best candidate split
    Variable bestSplit;
    double bestSplitScore = -DBL_MAX;
    for (size_t i = 0; i < K; ++i)
    {
      Variable split = splits->getElement(i);
      double splitScore = computeSplitScore(node, split);
      if (splitScore > bestSplitScore)
      {
        bestSplitScore = splitScore;
        bestSplit = split;
      }
    }

    if (bestSplit.exists())
    {
      // split node
      std::pair<MeasurableRegionPtr, MeasurableRegionPtr> subRegions = node->region->makeBinarySplit(bestSplit);
      node->split = bestSplit;
      node->left = new Node(subRegions.first, node->depth + 1);
      node->right = new Node(subRegions.second, node->depth + 1);

      // dispatch samples to sub nodes
      for (size_t i = 0; i < node->samples.size(); ++i)
      {
        const std::pair<Variable, double>& sample = node->samples[i];
        node->getSubNode(sample.first)->observe(sample.first, sample.second);
      }
      node->samples.clear();
      node->left->updateValues(numIterations, nu, rho, C);
      node->right->updateValues(numIterations, nu, rho, C);
    }
  }

  void updateTree(ExecutionContext& context, const NodePtr& node, const Variable& candidate, double reward)
  {
    if (!node->isLeaf())
    {
      // update sub node
      updateTree(context, node->getSubNode(candidate), candidate, reward);
    }

    node->observe(candidate, reward);
    node->updateValues(numIterations, nu, rho, C);

    // too big leaf -> split
    if (node->isLeaf() && node->samples.size() > nMin && node->stats.getVariance() > 0)
      splitNode(context, node);
  }

  Variable doIteration(ExecutionContext& context)
  {
    // select node
    NodePtr leaf = selectNode(context);
    MeasurableRegionPtr region = leaf->getRegion();
    jassert(region);
    
    // play bandit
    Variable candidate = region->samplePosition(context.getRandomGenerator());
    double reward = problem->getObjective()->compute(context, candidate).toDouble();

    // update scores recursively
    for (size_t i = 0; i < forest.size(); ++i)
      updateTree(context, forest[i], candidate, reward);

    ++numIterationsDone;
    return Variable::pair(reward, candidate);
  }

  void getBestSolution(ExecutionContext& context, NodePtr node, size_t depth, size_t& bestDepth, double& bestSum, double& score, Variable& solution)
  {
    if (depth > bestDepth || (depth == bestDepth && node->stats.getSum() > bestSum))
    {
      bestDepth = depth;
      bestSum = node->stats.getSum();
      score = node->stats.getMean();
      solution = node->getRegion()->getCenter();
    }

    if (!node->isLeaf())
    {
      getBestSolution(context, node->left, depth + 1, bestDepth, bestSum, score, solution);
      getBestSolution(context, node->right, depth + 1, bestDepth, bestSum, score, solution);
    }
  }

  void finishIteration(ExecutionContext& context)
  {
    for (size_t i = 0; i < forest.size(); ++i)
    {
      double bestIterationScore;
      Variable bestIterationSolution;
      size_t bestDepth = 0;
      double bestSum = 0.0;
      getBestSolution(context, forest[i], 0, bestDepth, bestSum, bestIterationScore, bestIterationSolution);
      OptimizerState::finishIteration(context, problem, numIterationsDone, -bestIterationScore, bestIterationSolution);
    }

    forest.clear();
  }

  class Node : public Object
  {
  public:
    Node(const MeasurableRegionPtr& region, size_t depth)
      : region(region), depth(depth), U(DBL_MAX), B(DBL_MAX) {}

    const MeasurableRegionPtr& getRegion() const
      {return region;}

    MeasurableRegionPtr region;
    size_t depth;
    ScalarVariableStatistics stats;
    double U;
    double B;

    void observe(const Variable& candidate, double reward)
    {
      stats.push(reward);
      if (isLeaf())
        samples.push_back(std::make_pair(candidate, reward));
    }

    void updateValues(size_t numIterations, double nu, double rho, double C)
    {
//      node->U = node->stats.getMean() + C / (double)count + nu * pow(rho, i);
      U = stats.getMean() + sqrt(C * log((double)numIterations) / (double)stats.getCount()) + nu * pow(rho, (double)depth);
      if (isLeaf())
        B = U;
      else
        B = juce::jmin(U, juce::jmax(left->B, right->B));
    }

    bool isLeaf() const
      {return !left || !right;}

    // internal nodes
    Variable split;
    NodePtr left;
    NodePtr right;

    bool testSplit(const Variable& candidate) const
      {return region->testSplit(split, candidate);}

    NodePtr getSubNode(const Variable& candidate) const
      {return testSplit(candidate) ? right : left;}

    // leaf nodes
    std::vector< std::pair<Variable, double> > samples;
  };

protected:
  friend class RFHOOOptimizerStateClass;

  OptimizationProblemPtr problem;

  size_t K;
  size_t nMin;
  size_t maxDepth;

  size_t numIterations;
  double nu;  // > 0
  double rho; // [0,1]
  double C;

  std::vector<NodePtr> forest;
  size_t numIterationsDone;
};

typedef ReferenceCountedObjectPtr<RFHOOOptimizerState> RFHOOOptimizerStatePtr;

class RFHOOOptimizer : public Optimizer
{
public:
  RFHOOOptimizer()
    : numTrees(100), K(0), nMin(10), maxDepth(0), numIterations(1000), nu(1.0), rho(0.5), C(2.0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    RFHOOOptimizerStatePtr hooState = optimizerState.staticCast<RFHOOOptimizerState>();
    while (hooState->getNumIterationsDone() < numIterations)
    {
      context.enterScope("Iteration " + String((int)hooState->getNumIterationsDone()+1));
      Variable res = hooState->doIteration(context);
      context.leaveScope(res);
    }
    hooState->finishIteration(context);
    return hooState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
  {
    size_t maxDepth = this->maxDepth;
    if (!maxDepth)
    {
      double logNumIterations = log((double)numIterations);
      maxDepth = (size_t)ceil((logNumIterations / 2 - log(1.0 / nu)) / log(1 / rho));
    }
    context.resultCallback(T("maxDepth"), maxDepth);
    return new RFHOOOptimizerState(problem, numTrees, K, nMin, maxDepth, numIterations, nu, rho, C);
  }

protected:
  friend class RFHOOOptimizerClass;

  size_t numTrees;
  size_t K;
  size_t nMin;
  size_t maxDepth;

  size_t numIterations;
  double nu;  // > 0
  double rho; // [0,1]
  double C;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_RF_HOO_SAND_BOX_H_

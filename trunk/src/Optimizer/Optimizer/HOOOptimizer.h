/*-----------------------------------------.---------------------------------.
| Filename: HOOOptimizer.h                 | Hierarchical Optimistic         |
| Author  : Francis Maes                   |  Optimization                   |
| Started : 29/08/2011                     |   from "X-Armed Bandits"        |
`------------------------------------------/       Bubeck et al 2011         |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_HOO_H_
#define LBCPP_OPTIMIZER_HOO_H_

# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{

class MeasurableRegion;
typedef ReferenceCountedObjectPtr<MeasurableRegion> MeasurableRegionPtr;

class MeasurableRegion : public Object
{
public:
  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit() const = 0;
  virtual Variable getCenter() const = 0;
};

class ScalarMeasurableRegion : public MeasurableRegion
{
public:
  ScalarMeasurableRegion(double minValue, double maxValue)
    : minValue(minValue), maxValue(maxValue) {}
  ScalarMeasurableRegion() : minValue(-DBL_MAX), maxValue(DBL_MAX) {}

  virtual Variable getCenter() const
    {return (minValue + maxValue) / 2.0;}

  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit() const
  {
    double middleValue = (maxValue + minValue) / 2.0;
    return std::make_pair(
      MeasurableRegionPtr(new ScalarMeasurableRegion(minValue, middleValue)),
      MeasurableRegionPtr(new ScalarMeasurableRegion(middleValue, maxValue)));
  }

protected:
  double minValue;
  double maxValue;
};

class VectorialMeasurableRegion : public MeasurableRegion
{
public:
  VectorialMeasurableRegion(const DenseDoubleVectorPtr& minValues, const DenseDoubleVectorPtr& maxValues, size_t axisToSplit = 0)
    : minValues(minValues), maxValues(maxValues), axisToSplit(axisToSplit) {}
  VectorialMeasurableRegion() : axisToSplit(0) {}

  virtual Variable getCenter() const
  {
    DenseDoubleVectorPtr res = minValues->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(0.5);
    maxValues->addWeightedTo(res, 0, 0.5);
    return res;
  }

  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit() const
  {
    double middleValue = (minValues->getValue(axisToSplit) + maxValues->getValue(axisToSplit)) / 2.0;
    DenseDoubleVectorPtr splitMaxValues = maxValues->cloneAndCast<DenseDoubleVector>();
    splitMaxValues->setValue(axisToSplit, middleValue);
    DenseDoubleVectorPtr splitMinValues = minValues->cloneAndCast<DenseDoubleVector>();
    splitMinValues->setValue(axisToSplit, middleValue);

    size_t newAxisToSplit = (axisToSplit + 1) % minValues->getNumValues();
    return std::make_pair(
      MeasurableRegionPtr(new VectorialMeasurableRegion(minValues, splitMaxValues, newAxisToSplit)),
      MeasurableRegionPtr(new VectorialMeasurableRegion(splitMinValues, maxValues, newAxisToSplit)));
  }

protected:
  DenseDoubleVectorPtr minValues;
  DenseDoubleVectorPtr maxValues;
  size_t axisToSplit;
};

class HOOOptimizerState : public OptimizerState
{
public:
  HOOOptimizerState(OptimizationProblemPtr problem, double nu, double rho, size_t numIterations, size_t maxDepth)
    : problem(problem), nu(nu), rho(rho), numIterations(numIterations), maxDepth(maxDepth), numIterationsDone(0)
  {
    MeasurableRegionPtr region;
    TypePtr inputType = problem->getSolutionsType();
    if (inputType->inheritsFrom(doubleType))
      region = new ScalarMeasurableRegion(0.0, 1.0);
    else if (inputType->inheritsFrom(denseDoubleVectorClass()))
    {
      DenseDoubleVectorPtr initial = problem->getInitialGuess().getObjectAndCast<DenseDoubleVector>();
      jassert(initial);
      DenseDoubleVectorPtr minValues = new DenseDoubleVector(inputType, initial->getNumValues(), 0.0);
      DenseDoubleVectorPtr maxValues = new DenseDoubleVector(inputType, initial->getNumValues(), 1.0);
      region = new VectorialMeasurableRegion(minValues, maxValues);
    }
    jassert(region);

    root = new Node(region);
    logNumIterations = log((double)numIterations);
    if (!maxDepth)
      maxDepth = (size_t)ceil((logNumIterations / 2 - log(1.0 / nu)) / log(1 / rho));
  }

  HOOOptimizerState() {}

  size_t getNumIterationsDone() const
    {return numIterationsDone;}

  class Node;
  typedef ReferenceCountedObjectPtr<Node> NodePtr;


  void selectNode(ExecutionContext& context, size_t maxDepth, std::vector<NodePtr>& path)
  {
    NodePtr* ptr = &root;
    path.push_back(*ptr);

    while (path.size() < maxDepth)
    {
      NodePtr& left = (*ptr)->left;
      NodePtr& right = (*ptr)->right;
      MeasurableRegionPtr leftRegion, rightRegion;
      double leftValue = getRegionAndBValue(*ptr, left, leftRegion);
      double rightValue = getRegionAndBValue(*ptr, right, rightRegion);
      
      MeasurableRegionPtr region;
      if (rightValue > leftValue ||
          (rightValue == leftValue && context.getRandomGenerator()->sampleBool(0.5)))
      {
        ptr = &right;
        region = rightRegion;
      }
      else
      {
        ptr = &left;
        region = leftRegion;
      }

      if (!*ptr)
      {
        *ptr = new Node(region);
        path.push_back(*ptr);
        break;
      }
      else
        path.push_back(*ptr);
    }
  }

  Variable doIteration(ExecutionContext& context)
  {
    jassert((double)numIterations > 1.0 / (nu * nu));

    // select node (and expand tree)
    std::vector<NodePtr> path;
    selectNode(context, maxDepth, path);
    
    // play bandit
    MeasurableRegionPtr region = path.back()->getRegion();
    jassert(region);
    Variable candidate = region->getCenter();
    double reward = problem->getObjective()->compute(context, candidate).toDouble();
        
    // update scores recursively
    double nun = nu;
    for (int i = path.size() - 1; i >= 0; --i)
    {
      const NodePtr& node = path[i];
      node->observeReward(reward);
      size_t count = (size_t)node->stats.getCount();
      node->U = node->stats.getMean() + sqrt(2.0 * logNumIterations / (double)count) + nun;
      node->B = juce::jmin(node->U, juce::jmax(node->left ? node->left->B : DBL_MAX, node->right ? node->right->B : DBL_MAX));
      node->meanReward = node->stats.getMean();
      node->expectedReward = juce::jmin(node->meanReward, juce::jmax(node->left ? node->left->expectedReward : DBL_MAX, node->right ? node->right->expectedReward : DBL_MAX));

    }
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

    if (node->left)
      getBestSolution(context, node->left, depth + 1, bestDepth, bestSum, score, solution);
    if (node->right)
      getBestSolution(context, node->right, depth + 1, bestDepth, bestSum, score, solution);
  }

  void finishIteration(ExecutionContext& context)
  {
    double bestIterationScore;
    Variable bestIterationSolution;
    size_t bestDepth = 0;
    double bestSum = 0.0;
    getBestSolution(context, root, 0, bestDepth, bestSum, bestIterationScore, bestIterationSolution);
    OptimizerState::finishIteration(context, problem, numIterationsDone, -bestIterationScore, bestIterationSolution);
  }

  class Node : public Object
  {
  public:
    Node(const MeasurableRegionPtr& region)
      : region(region), U(DBL_MAX), B(DBL_MAX), meanReward(meanReward), expectedReward(expectedReward) {}

    const MeasurableRegionPtr& getRegion() const
      {return region;}

    void observeReward(double reward)
      {stats.push(reward);}

    MeasurableRegionPtr region;
    ScalarVariableStatistics stats;
    double U;
    double B;
    double meanReward;
    double expectedReward;

    NodePtr left;
    NodePtr right;
  };

protected:
  friend class HOOOptimizerStateClass;

  OptimizationProblemPtr problem;
  double nu;
  double rho;
  size_t numIterations;

  NodePtr root;
  double logNumIterations;
  size_t maxDepth;
  size_t numIterationsDone;
  
  double getRegionAndBValue(const NodePtr& parentNode, NodePtr& node, MeasurableRegionPtr& region) const
  {
    if (node == NodePtr())
    {
      std::pair<MeasurableRegionPtr, MeasurableRegionPtr> subRegions = parentNode->getRegion()->makeBinarySplit(); // parentRegion
      region = &node == &parentNode->left ? subRegions.first : subRegions.second;
      return DBL_MAX;
    }
    else
    {
      region = node->region;
      return node->B;
    }
  }
};

typedef ReferenceCountedObjectPtr<HOOOptimizerState> HOOOptimizerStatePtr;

class HOOOptimizer : public Optimizer
{
public:
  HOOOptimizer(size_t numIterations, double nu, double rho)
    : numIterations(numIterations), nu(nu), rho(rho) {}
  HOOOptimizer() : numIterations(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    HOOOptimizerStatePtr hooState = optimizerState.staticCast<HOOOptimizerState>();
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
    {return new HOOOptimizerState(problem, nu, rho, numIterations, maxDepth);}

protected:
  friend class HOOOptimizerClass;

  size_t numIterations;
  double nu;  // > 0
  double rho; // [0,1]
  size_t maxDepth;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_CMA_ES_H_

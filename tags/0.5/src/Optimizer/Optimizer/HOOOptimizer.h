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
  virtual Variable getCenter() const = 0;
  virtual Variable samplePosition(const RandomGeneratorPtr& random) const = 0;

  virtual Variable getDefaultSplit() const = 0;
  virtual ContainerPtr sampleSplits(const RandomGeneratorPtr& random, size_t count) const = 0;
  virtual bool testSplit(const Variable& split, const Variable& candidate) const = 0;
  virtual double getTrueRatio(const Variable& split) const = 0;

  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit(const Variable& split) const = 0;
};

class ScalarMeasurableRegion : public MeasurableRegion
{
public:
  ScalarMeasurableRegion(double minValue, double maxValue)
    : minValue(minValue), maxValue(maxValue) {}
  ScalarMeasurableRegion() : minValue(-DBL_MAX), maxValue(DBL_MAX) {}

  virtual Variable getCenter() const
    {return (minValue + maxValue) / 2.0;}

  virtual Variable samplePosition(const RandomGeneratorPtr& random) const
    {return random->sampleDouble(minValue, maxValue);}

  virtual Variable getDefaultSplit() const
    {return (maxValue + minValue) / 2.0;}

  virtual ContainerPtr sampleSplits(const RandomGeneratorPtr& random, size_t count) const
  {
    DenseDoubleVectorPtr res(new DenseDoubleVector(count, 0.0));
    for (size_t i = 0; i < count; ++i)
      res->setValue(i, random->sampleDouble(minValue, maxValue));
    return res;
  }

  virtual bool testSplit(const Variable& split, const Variable& candidate) const
    {return candidate.getDouble() > split.getDouble();}
  virtual double getTrueRatio(const Variable& split) const
    {return (maxValue - split.getDouble()) / (maxValue - minValue);}

  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit(const Variable& split) const
  {
    double middleValue = split.getDouble();
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
  VectorialMeasurableRegion(const DenseDoubleVectorPtr& minValues, const DenseDoubleVectorPtr& maxValues, size_t defaultAxisToSplit = 0)
    : minValues(minValues), maxValues(maxValues), defaultAxisToSplit(defaultAxisToSplit) {}
  VectorialMeasurableRegion() : defaultAxisToSplit(0) {}

  virtual Variable getCenter() const
  {
    DenseDoubleVectorPtr res = minValues->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(0.5);
    maxValues->addWeightedTo(res, 0, 0.5);
    return res;
  }

  virtual Variable samplePosition(const RandomGeneratorPtr& random) const
  {
    size_t n = minValues->getNumValues();
    DenseDoubleVectorPtr res = new DenseDoubleVector(minValues->getClass(), n);
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, random->sampleDouble(minValues->getValue(i), maxValues->getValue(i)));
    return res;
  }

  virtual Variable getDefaultSplit() const
  {
    size_t axisToSplit = defaultAxisToSplit;
    double middleValue = (minValues->getValue(axisToSplit) + maxValues->getValue(axisToSplit)) / 2.0;
    return new Pair(axisToSplit, middleValue);
  }

  virtual ContainerPtr sampleSplits(const RandomGeneratorPtr& random, size_t count) const
  {
    size_t n = minValues->getNumValues(); // dimensionality
    ObjectVectorPtr res = new ObjectVector(pairClass(positiveIntegerType, doubleType), count);

    std::vector<size_t> attributes;
    random->sampleOrder(n, attributes);
    for (size_t i = 0; i < count; ++i)
    {
      size_t axisToSplit = attributes[i % n];
      double splitValue = random->sampleDouble(minValues->getValue(axisToSplit), maxValues->getValue(axisToSplit));
      res->set(i, new Pair(axisToSplit, splitValue));
    }
    return res;
  }

  virtual bool testSplit(const Variable& split, const Variable& candidate) const
  {
    const PairPtr& splitPair = split.getObjectAndCast<Pair>();
    size_t axisToSplit = (size_t)splitPair->getFirst().getInteger();
    double splitValue = splitPair->getSecond().getDouble();
    return candidate.getObjectAndCast<DenseDoubleVector>()->getValue(axisToSplit) > splitValue;
  }

  virtual double getTrueRatio(const Variable& split) const
  {
    const PairPtr& splitPair = split.getObjectAndCast<Pair>();
    size_t axisToSplit = (size_t)splitPair->getFirst().getInteger();
    double splitValue = splitPair->getSecond().getDouble();
    return (maxValues->getValue(axisToSplit) - splitValue) 
            / (maxValues->getValue(axisToSplit) - minValues->getValue(axisToSplit));
  }

  virtual std::pair<MeasurableRegionPtr, MeasurableRegionPtr> makeBinarySplit(const Variable& split) const
  {
    size_t axisToSplit = (size_t)split.getObjectAndCast<Pair>()->getFirst().getInteger();
    double splitValue = split.getObjectAndCast<Pair>()->getSecond().getDouble();

    DenseDoubleVectorPtr splitMaxValues = maxValues->cloneAndCast<DenseDoubleVector>();
    splitMaxValues->setValue(axisToSplit, splitValue);
    DenseDoubleVectorPtr splitMinValues = minValues->cloneAndCast<DenseDoubleVector>();
    splitMinValues->setValue(axisToSplit, splitValue);

    size_t newDefaultAxisToSplit = (defaultAxisToSplit + 1) % minValues->getNumValues();
    return std::make_pair(
      MeasurableRegionPtr(new VectorialMeasurableRegion(minValues, splitMaxValues, newDefaultAxisToSplit)),
      MeasurableRegionPtr(new VectorialMeasurableRegion(splitMinValues, maxValues, newDefaultAxisToSplit)));
  }

protected:
  DenseDoubleVectorPtr minValues;
  DenseDoubleVectorPtr maxValues;
  size_t defaultAxisToSplit;
};

class HOOOptimizerState : public OptimizerState
{
public:
  HOOOptimizerState(const OptimizationProblemPtr& problem, double nu, double rho, size_t numIterations, size_t maxDepth, double C, bool playCenteredArms)
    : OptimizerState(problem), nu(nu), rho(rho), numIterations(numIterations), maxDepth(maxDepth), C(C), playCenteredArms(playCenteredArms), numIterationsDone(0)
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
  //  jassert((double)numIterations > 1.0 / (nu * nu));

    // select node (and expand tree)
    std::vector<NodePtr> path;
    selectNode(context, maxDepth, path);
    
    // play bandit
    MeasurableRegionPtr region = path.back()->getRegion();
    jassert(region);
    Variable candidate = playCenteredArms ? region->getCenter() : region->samplePosition(context.getRandomGenerator());
    double reward = problem->getObjective()->compute(context, candidate).toDouble();
        
    // update scores recursively
    for (int i = path.size() - 1; i >= 0; --i)
    {
      const NodePtr& node = path[i];
      node->observeReward(reward);
      size_t count = (size_t)node->stats.getCount();
      node->U = node->stats.getMean() + sqrt(C * logNumIterations / (double)count) + nu * pow(rho, i);
//      node->U = node->stats.getMean() + C / (double)count + nu * pow(rho, i);
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
    OptimizerState::finishIteration(context, numIterationsDone, -bestIterationScore, bestIterationSolution);
  }

  void predict(ExecutionContext& context, const Variable& candidate, double& expectation, double& upperBound)
  {
    jassert(root);
    NodePtr node = root;
    upperBound = node->B;
    while (node->isInternal())
    {
      node = node->getSubNode(candidate);
      upperBound = juce::jmin(upperBound, node->B);
    }
    expectation = node->stats.getMean();
  }

  void clearTree()
    {root = NodePtr();}

  class Node : public Object
  {
  public:
    Node(const MeasurableRegionPtr& region)
      : region(region), U(DBL_MAX), B(DBL_MAX), meanReward(0.0), expectedReward(0.0), split(region->getDefaultSplit()) {}

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

    Variable split;
    NodePtr left;
    NodePtr right;

    bool isInternal() const
      {return left && right;}

    NodePtr getSubNode(const Variable& candidate) const
      {return region->testSplit(split, candidate) ? right : left;}
  };

protected:
  friend class HOOOptimizerStateClass;

  double nu;
  double rho;
  size_t numIterations;

  NodePtr root;
  double logNumIterations;
  size_t maxDepth;
  double C;
  bool playCenteredArms;
  size_t numIterationsDone;
  
  double getRegionAndBValue(const NodePtr& parentNode, NodePtr& node, MeasurableRegionPtr& region) const
  {
    if (node == NodePtr())
    {
      MeasurableRegionPtr parentRegion = parentNode->region;
      std::pair<MeasurableRegionPtr, MeasurableRegionPtr> subRegions = parentRegion->makeBinarySplit(parentNode->split);
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
  HOOOptimizer(size_t numIterations = 10000, double nu = 1.0, double rho = 0.5, double C = 2.0)
    : numIterations(numIterations), nu(nu), rho(rho), maxDepth(0), C(C), playCenteredArms(true) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    HOOOptimizerStatePtr hooState = optimizerState.staticCast<HOOOptimizerState>();
    while (hooState->getNumIterationsDone() < numIterations)
    {
      //context.enterScope("Iteration " + String((int)hooState->getNumIterationsDone()+1));
      /*Variable res = */hooState->doIteration(context);
      //context.leaveScope(res);
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
    return new HOOOptimizerState(problem, nu, rho, numIterations, maxDepth, C, playCenteredArms);
  }

protected:
  friend class HOOOptimizerClass;

  size_t numIterations;
  double nu;  // > 0
  double rho; // [0,1]
  size_t maxDepth;
  double C;
  bool playCenteredArms;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_CMA_ES_H_

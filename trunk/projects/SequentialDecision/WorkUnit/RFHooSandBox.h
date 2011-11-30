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
# include "../../Examples/OptimizerTestBed.h"

namespace lbcpp
{

class RFHOOOptimizerState : public OptimizerState
{
public:
  RFHOOOptimizerState(OptimizationProblemPtr problem,
                      size_t numTrees, size_t K, size_t nMin, size_t maxDepth,
                      size_t numIterations,
                      double nu, double rho, double C)
    : problem(problem), K(K), nMin(nMin), maxDepth(maxDepth),
      numIterations(numIterations), nu(nu), rho(rho), C(C), 
      forest(numTrees), forestStats(numTrees), numIterationsDone(0)
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
      forest[i] = new Node(region, 1.0, 0);

    //jassert((double)numIterations > 1.0 / (nu * nu));
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

  NodePtr selectNode(ExecutionContext& context, size_t& treeIndex) const
  {
    std::vector<size_t> bestTrees;
    double bestScore = -DBL_MAX;
    
    for (size_t i = 0; i < forest.size(); ++i)
    {
      double score = forestStats[i].getCount() ? forestStats[i].getMean() + sqrt(C * log((double)numIterations) / forestStats[i].getCount()) : DBL_MAX;
      if (score >= bestScore)
      {
        if (score > bestScore)
        {
          bestScore = score;
          bestTrees.clear();
        }
        bestTrees.push_back(i);
      }
    }
    jassert(bestTrees.size());
    treeIndex = bestTrees.size() > 1 ? bestTrees[context.getRandomGenerator()->sampleSize(bestTrees.size())] : bestTrees[0];
    //treeIndex = context.getRandomGenerator()->sampleSize(forest.size());
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

    if (!leftStats.getCount() || !rightStats.getCount())
      return 0.0;

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
    double bestSplitScore = 0.0;//-DBL_MAX;
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
      double p = node->region->getTrueRatio(bestSplit);
      node->left = new Node(subRegions.first, node->diameter * (1.0 - p), node->depth + 1);
      node->right = new Node(subRegions.second, node->diameter * p, node->depth + 1);

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
    if (node->isLeaf() && node->samples.size() > nMin && node->stats.getVariance() > 0 && node->depth < maxDepth)
      splitNode(context, node);
  }

  Variable selectCandidate(ExecutionContext& context, size_t& treeIndex) const
  {
    double bestScore = -DBL_MAX;
    std::vector<Variable> bestCandidates;
    //for (size_t i = 0; i < forest.size(); ++i)
    for (double x = 0.0; x <= 1.0; x += 0.005)
    {
      //NodePtr node = selectNode(context, forest[i]);
      //Variable candidate = node->region->samplePosition(context.getRandomGenerator());
      Variable candidate(x);

      double expectation, meanUpperBound, minUpperBound, simpleUpperBound;
      predict(context, candidate, expectation, meanUpperBound, minUpperBound, simpleUpperBound);
      double score = minUpperBound;
      if (score >= bestScore)
      {
        if (score > bestScore)
        {
          bestScore = score;
          bestCandidates.clear();
        }
        bestCandidates.push_back(candidate);
      }
    }
    return bestCandidates[context.getRandomGenerator()->sampleSize(bestCandidates.size())];
/*
    NodePtr leaf = selectNode(context, treeIndex);
    MeasurableRegionPtr region = leaf->getRegion();
    jassert(region);
    return region->samplePosition(context.getRandomGenerator());*/
  }

  Variable doIteration(ExecutionContext& context)
  {
    // play bandit
    size_t treeIndex = (size_t)-1;
    Variable candidate = selectCandidate(context, treeIndex);
    double reward = problem->getObjective()->compute(context, candidate).toDouble();
    if (treeIndex !=(size_t)-1)
      forestStats[treeIndex].push(reward);

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
  }

  class Node : public Object
  {
  public:
    Node(const MeasurableRegionPtr& region, double diameter, size_t depth)
      : region(region), diameter(diameter), depth(depth), U(DBL_MAX), B(DBL_MAX) {}

    const MeasurableRegionPtr& getRegion() const
      {return region;}

    MeasurableRegionPtr region;
    double diameter;
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
      jassert(stats.getCount());
//      node->U = node->stats.getMean() + C / (double)count + nu * pow(rho, i);
      // double diameter = pow(rho, (double)depth);
      U = stats.getMean() + sqrt(C * log((double)numIterations) / (double)stats.getCount()) + nu * diameter;
      if (isLeaf())
        B = U;
      else
        B = juce::jmin(U, juce::jmax(left->B, right->B));
      jassert(isNumberValid(B));
    }

    bool isLeaf() const
      {return !left || !right;}

    bool isInternal() const
      {return left && right;}

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

  void predict(ExecutionContext& context, const Variable& candidate, double& expectation, double& meanUpperBound, double& minUpperBound, double& simpleUpperBound) const
  {
    jassert(forest.size());
    expectation = 0.0;
    double sum = 0.0;
    double count = 0.0;
    meanUpperBound = 0.0;
    minUpperBound = DBL_MAX;
    simpleUpperBound = 0.0;
    for (size_t i = 0; i < forest.size(); ++i)
    {
      NodePtr node = forest[i];
      double B = node->B;
      while (node->isInternal())
      {
        node = node->getSubNode(candidate);
        B = juce::jmin(B, node->B);
      }
      expectation += node->stats.getMean();
      sum += node->stats.getSum();
      count += node->stats.getCount();
      simpleUpperBound += node->stats.getMean() + C / (1.0 + node->stats.getCount());
      meanUpperBound += B;
      minUpperBound = juce::jmin(minUpperBound, B);
    }
    expectation /= forest.size();
    meanUpperBound /= forest.size();
    simpleUpperBound /= forest.size();
   // simpleUpperBound = count ? (sum + 5.0) / count : DBL_MAX;
  }

  void clearForest()
    {forest.clear();}

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
  std::vector<ScalarVariableStatistics> forestStats;
  size_t numIterationsDone;
};

typedef ReferenceCountedObjectPtr<RFHOOOptimizerState> RFHOOOptimizerStatePtr;

class RFHOOOptimizer : public Optimizer
{
public:
  RFHOOOptimizer(size_t numTrees = 100, size_t K = 0, size_t nMin = 10, size_t maxDepth = 0, size_t numIterations = 1000, double nu = 1.0, double rho = 0.5, double C = 2.0)
    : numTrees(numTrees), K(K), nMin(nMin), maxDepth(maxDepth), numIterations(numIterations), nu(nu), rho(rho), C(C) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    static const bool verbose = false;

    size_t episodeLength = numIterations / 100;
    RFHOOOptimizerStatePtr hooState = optimizerState.staticCast<RFHOOOptimizerState>();
    while (hooState->getNumIterationsDone() < numIterations)
    {
      ScalarVariableStatisticsPtr rewardStats;
      if (verbose)
      {
        context.enterScope("Iteration " + String((int)hooState->getNumIterationsDone()+1));
        context.resultCallback(T("iteration"), hooState->getNumIterationsDone());
        rewardStats = new ScalarVariableStatistics("reward");
      }

      size_t numRemainingIterations = numIterations - hooState->getNumIterationsDone();
      size_t limit = episodeLength && numRemainingIterations > episodeLength ? episodeLength : numRemainingIterations;
      for (size_t i = 0; i < limit; ++i)
      {
        PairPtr res = hooState->doIteration(context).getObjectAndCast<Pair>();
        if (verbose)
          rewardStats->push(res->getFirst().getDouble());
      }
      if (verbose)
      {
        context.resultCallback(T("rewards mean"), rewardStats->getMean());
        context.resultCallback(T("rewards"), rewardStats);
        context.leaveScope();
      }
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

/////////////////////////////////////////////////////

class Simple1DTestFunction : public SimpleUnaryFunction
{
public:
  Simple1DTestFunction() : SimpleUnaryFunction(doubleType, doubleType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    double x = juce::jlimit(0.0, 1.0, input.getDouble());
    return (sin(13.0 * x) * sin(27.0 * x) + 1.0) / 2.0;
  }
};

class FunctionToBernoulliArms : public SimpleUnaryFunction
{
public:
  FunctionToBernoulliArms(ExecutionContext& context, FunctionPtr expectationFunction, size_t dimension)
    : SimpleUnaryFunction(expectationFunction->getRequiredInputType(0, 1), doubleType), expectationFunction(expectationFunction),
      regretCurve(new DenseDoubleVector(20, 0.0)),
      playHistogram(new DenseDoubleVector(101, 0.0))
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    maxValue = 1.0;
    if (expectationFunction->getClassName() == T("RosenbrockFunction"))
      maxValue = 100.0 * (dimension - 1);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    double energy = expectationFunction->compute(context, input).getDouble();
    double probability = energy;
    if (expectationFunction->getClassName() == T("RosenbrockFunction"))
      probability = juce::jlimit(0.0, 1.0, (maxValue - energy) / maxValue);
    else
    {
      jassert(probability >= 0.0 && probability <= 1.0);
    }
    const_cast<FunctionToBernoulliArms* >(this)->regret.push(1.0 - probability);
    if (input.isDouble())
      const_cast<FunctionToBernoulliArms* >(this)->updateRegret(input.getDouble(), probability);
    return context.getRandomGenerator()->sampleBool(probability) ? 1.0 : 0.0;
  }

  void updateRegret(double x, double e) // x = arm, e = expected reward for this arm
  {
    static const size_t powersOfTwo[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

    size_t iteration = (size_t)regret.getCount();

    int powerOfTwoIndex = -1;
    for (size_t i = 0; i < sizeof (powersOfTwo) / sizeof (size_t); ++i)
      if (powersOfTwo[i] == iteration)
      {
        powerOfTwoIndex = (int)i;
        break;
      }
    if (powerOfTwoIndex >= 0)
      regretCurve->setValue(powerOfTwoIndex, regret.getSum());

    if (x >= 0.0 && x <= 1.0)
      playHistogram->incrementValue((int)(x * 100.0), 1.0);
  }

  double getCumulativeRegret() const
    {return regret.getSum();}

  const DenseDoubleVectorPtr& getRegretCurve() const
    {return regretCurve;}

  const DenseDoubleVectorPtr& getPlayHistogram() const
    {return playHistogram;}

protected:
  FunctionPtr expectationFunction;
  ScalarVariableStatistics regret;
  DenseDoubleVectorPtr regretCurve;
  DenseDoubleVectorPtr playHistogram;
  double maxValue;
};

typedef ReferenceCountedObjectPtr<FunctionToBernoulliArms> FunctionToBernoulliArmsPtr;

class RFHOOO1DSandBox : public WorkUnit
{
public:
  RFHOOO1DSandBox() : K(1), nMin(10), maxDepth(7), maxHorizon(10000), nu(2.0), rho(0.5), C(2.0) {}

  struct RunInfo
  {
    String name;
    DenseDoubleVectorPtr cumulativeRegret;
    DenseDoubleVectorPtr playHistogram;
    DenseDoubleVectorPtr expectation;
    DenseDoubleVectorPtr meanUpperBound;
    DenseDoubleVectorPtr minUpperBound;
    DenseDoubleVectorPtr simpleUpperBound;
  };

  void runOptimizer(ExecutionContext& context, const String& name, const OptimizerPtr& optimizer, const FunctionPtr& objectiveExpectation,
                                        std::vector<RunInfo>& res) const
  {
    context.enterScope(name);
    FunctionToBernoulliArmsPtr objective = new FunctionToBernoulliArms(context, objectiveExpectation, 1);
    OptimizationProblemPtr problem = new OptimizationProblem(objective, 0.5);
    OptimizerStatePtr finalState = optimizer->optimize(context, problem);
    context.leaveScope(objective->getCumulativeRegret());

    RunInfo runInfo;
    runInfo.name = name;
    runInfo.cumulativeRegret = objective->getRegretCurve();
    runInfo.playHistogram = objective->getPlayHistogram();
    runInfo.expectation = new DenseDoubleVector(100, 0.0);
    runInfo.meanUpperBound = new DenseDoubleVector(100, 0.0);
    runInfo.minUpperBound = new DenseDoubleVector(100, 0.0);
    runInfo.simpleUpperBound = new DenseDoubleVector(100, 0.0);

    HOOOptimizerStatePtr hooState = finalState.dynamicCast<HOOOptimizerState>();
    RFHOOOptimizerStatePtr rfHooState = finalState.dynamicCast<RFHOOOptimizerState>();

    double predictionError = 0.0;
    for (size_t i = 0; i < 100; ++i)
    {
      double x = (i + 0.5) / 100.0;
      double trueY = objectiveExpectation->compute(context, x).getDouble();
      double expectation = 0.0, meanUpperBound = 0.0, minUpperBound = 0.0, simpleUpperBound = 0.0;
      
      if (rfHooState)
        rfHooState->predict(context, x, expectation, meanUpperBound, minUpperBound, simpleUpperBound);
      else
      {
        hooState->predict(context, x, expectation, meanUpperBound);
        minUpperBound = meanUpperBound;
      }
      runInfo.expectation->setValue(i, expectation);
      runInfo.meanUpperBound->setValue(i, meanUpperBound);
      runInfo.minUpperBound->setValue(i, minUpperBound);
      runInfo.simpleUpperBound->setValue(i, simpleUpperBound);
      predictionError += (trueY - expectation) * (trueY - expectation);
    }
    predictionError = sqrt(predictionError / 100.0);
    context.informationCallback(T("Prediction error: ") + String(predictionError));

    if (rfHooState)
      rfHooState->clearForest();
    else
      hooState->clearTree();
    res.push_back(runInfo);
  }

  virtual Variable run(ExecutionContext& context)
  {
    FunctionPtr objectiveExpectation = new Simple1DTestFunction();
    size_t numIterations = maxHorizon;

    std::vector<RunInfo> runInfos;

    runOptimizer(context, "hoo", new HOOOptimizer(numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-1", new RFHOOOptimizer(1, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-5", new RFHOOOptimizer(5, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-10", new RFHOOOptimizer(10, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-20", new RFHOOOptimizer(20, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    /*runOptimizer(context, "rfhoo-50", new RFHOOOptimizer(50, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-100", new RFHOOOptimizer(100, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);

    runOptimizer(context, "hoo bis", new HOOOptimizer(numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-1 bis", new RFHOOOptimizer(1, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-5 bis", new RFHOOOptimizer(5, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);
    runOptimizer(context, "rfhoo-10 bis", new RFHOOOptimizer(10, K, nMin, maxDepth, numIterations, nu, rho, C), objectiveExpectation, runInfos);*/

    context.enterScope(T("Cumulative Regrets"));
    for (size_t i = 0; i < 14; ++i)
    {
      context.enterScope(T("Hor ") + String((int)i));
      context.resultCallback(T("log2(T)"), i);
      for (size_t j = 0; j < runInfos.size(); ++j)
        context.resultCallback(runInfos[j].name, runInfos[j].cumulativeRegret->getValue(i));
      context.leaveScope();
    }
    context.leaveScope();

    context.enterScope(T("Play histograms"));
    for (size_t i = 0; i < 100; ++i)
    {
      double x = (i + 0.5) / 100.0;
      context.enterScope(String((int)i));
      context.resultCallback(T("x"), x);
      context.resultCallback(T("e[r]"), objectiveExpectation->compute(context, x).getDouble());
      for (size_t j = 0; j < runInfos.size(); ++j)
      {
        context.resultCallback(runInfos[j].name + " histogram", runInfos[j].playHistogram->getValue(i));
        context.resultCallback(runInfos[j].name + " expectation", runInfos[j].expectation->getValue(i));
        context.resultCallback(runInfos[j].name + " meanUpperBound", runInfos[j].meanUpperBound->getValue(i));
        context.resultCallback(runInfos[j].name + " minUpperBound", runInfos[j].minUpperBound->getValue(i));
        context.resultCallback(runInfos[j].name + " simpleUpperBound", runInfos[j].simpleUpperBound->getValue(i));
      }
      context.leaveScope();
    }
    context.leaveScope();

    return true;
  }

protected:
  friend class RFHOOO1DSandBoxClass;

  size_t K;
  size_t nMin;
  size_t maxDepth;
  size_t maxHorizon;
  
  double nu;
  double rho;
  double C;
};

class HOOOptimizeWorkUnit : public WorkUnit
{
public:
  HOOOptimizeWorkUnit(const OptimizerPtr& optimizer, const FunctionPtr& objective, size_t dimension, const String& description)
    : WorkUnit(), optimizer(optimizer), objective(objective), dimension(dimension), description(description) {}
  HOOOptimizeWorkUnit() : dimension(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    FunctionToBernoulliArmsPtr bernoulliObjective = new FunctionToBernoulliArms(context, objective, dimension);
    
    SamplerPtr sampler;
    if (objective->getRequiredInputType(0, 1) == doubleType)
      sampler = gaussianSampler(0.5);
    else
      sampler = independentDoubleVectorSampler(dimension, gaussianSampler(0.5));
    
    OptimizationProblemPtr problem = new OptimizationProblem(bernoulliObjective, new DenseDoubleVector(dimension, 0.5), sampler);
    problem->setMaximisationProblem(true);
    /*OptimizerStatePtr finalState =*/ optimizer->optimize(context, problem);
    return bernoulliObjective->getCumulativeRegret();
  }

  virtual String toShortString() const
    {return description;}

protected:
  OptimizerPtr optimizer;
  FunctionPtr objective;
  size_t dimension;
  String description;
};

class RFHOOOSandBox : public WorkUnit
{
public:
  RFHOOOSandBox() : dimension(5), K(1), nMin(10), maxDepth(7), maxHorizon(10000), nu(2.0), rho(0.5), C(2.0) {}

  void createObjectiveFunctions(const RandomGeneratorPtr& random, std::vector<FunctionPtr>& res)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      DenseDoubleVectorPtr xopt = new DenseDoubleVector(dimension, 0.0);
      for (size_t i = 0; i < dimension; ++i)
        xopt->setValue(i, random->sampleDouble(0.0, 1.0));
      res.push_back(new RosenbrockFunction(xopt, 0.0));
    }
  }

  double runOptimizer(ExecutionContext& context, const String& name, const OptimizerPtr& optimizer, const std::vector<FunctionPtr>& objectives, size_t numRuns = 1) const
  {
    context.informationCallback(T("Optimizer: ") + optimizer->toShortString());
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(name, objectives.size() * numRuns);
    for (size_t i = 0; i < workUnit->getNumWorkUnits(); ++i)
    {
      FunctionPtr objective = objectives[i % objectives.size()];
      workUnit->setWorkUnit(i, new HOOOptimizeWorkUnit(optimizer->cloneAndCast<Optimizer>(), objective, dimension, String((int)i+1) + T(" - ") + objective->toShortString()));
    }
    workUnit->setPushChildrenIntoStackFlag(true);
    workUnit->setProgressionUnit(T("Runs"));

    ScalarVariableStatistics regretStats;
    VariableVectorPtr res = context.run(workUnit, false).getObjectAndCast<VariableVector>();
    for (size_t i = 0; i < res->getNumElements(); ++i)
      regretStats.push(res->getElement(i).getDouble());
    return regretStats.getMean();
/*

    context.enterScope(name);
    ScalarVariableStatistics regretStats;
    for (size_t i = 0; i < objectives.size() * numRuns; ++i)
    {
      FunctionPtr objective = objectives[i % objectives.size()];
      context.enterScope(String((int)i+1) + T(" - ") + objective->toShortString());

      FunctionToBernoulliArmsPtr bernoulliObjective = new FunctionToBernoulliArms(context, objective, dimension);
      
      SamplerPtr sampler;
      if (objective->getRequiredInputType(0, 1) == doubleType)
        sampler = gaussianSampler(0.5);
      else
        sampler = independentDoubleVectorSampler(dimension, gaussianSampler(0.5));
      
      OptimizationProblemPtr problem = new OptimizationProblem(bernoulliObjective, new DenseDoubleVector(dimension, 0.5), sampler);
      problem->setMaximisationProblem(true);
      OptimizerStatePtr finalState = optimizer->optimize(context, problem);

      context.leaveScope(bernoulliObjective->getCumulativeRegret());
      regretStats.push(bernoulliObjective->getCumulativeRegret());
    }
    context.leaveScope(regretStats.getMean());
    return regretStats.getMean();*/
  }

  void setOptimizerVariable(OptimizerPtr optimizer, const String& variableName, double value)
  {
    size_t variableIndex = (size_t)optimizer->getClass()->findMemberVariable(variableName);
    if (variableName == T("nMin"))
      optimizer->setVariable(variableIndex, (size_t)pow(2.0, value));
    else if (variableName == T("maxDepth"))
      optimizer->setVariable(variableIndex, (size_t)value);
    else
      optimizer->setVariable(variableIndex, value);
  }

  void tuneOptimizerVariable(ExecutionContext& context, OptimizerPtr optimizer, const String& variableName, double minValue, double maxValue, const std::vector<FunctionPtr>& objectives)
  {
    double bestRegret = DBL_MAX;
    double bestValue = 0.0;
    context.enterScope(T("Tune ") + variableName + T(" in ") + optimizer->toShortString());
    double step = (maxValue - minValue) / 20.0;
    for (double value = minValue; value <= maxValue; value += step)
    {
      context.enterScope(T("Value = ") + String(value));
      context.resultCallback(T("value"), value);

      setOptimizerVariable(optimizer, variableName, value);
      double regret = runOptimizer(context, variableName + T(" = ") + String(value), optimizer, objectives);
      if (regret < bestRegret)
        bestRegret = regret, bestValue = value;
      context.resultCallback("regret", regret);
      context.leaveScope(regret);
    }
    setOptimizerVariable(optimizer, variableName, bestValue);
    context.leaveScope(new Pair(bestValue, bestRegret));
  }

  virtual Variable run(ExecutionContext& context)
  {
    std::vector<FunctionPtr> objectives;
    //createObjectiveFunctions(context.getRandomGenerator(), objectives);

    //FunctionPtr objectiveExpectation = new Simple1DTestFunction();
    for (size_t i = 0; i < 10; ++i)
      objectives.push_back(new Simple1DTestFunction());

    size_t numIterations = maxHorizon;

   /* for (double C = 0.0; C <= 2.0; C += 0.1)
    {
      runOptimizer(context, "hoo C = " + String(C), new HOOOptimizer(numIterations, 2.0, rho, 0.1), objectives);
    }
    for (double C = 0.0; C <= 2.0; C += 0.1)
    {
      runOptimizer(context, "rfhoo-10 C = " + String(C), new RFHOOOptimizer(10, K, nMin, maxDepth, numIterations, 0.2, rho, 0.1), objectives);
    }*/

    OptimizerPtr optimizer;
    double meanCumulativeRegret;
/*
    size_t populationSize = 10;
    optimizer = edaOptimizer(numIterations / populationSize, populationSize, populationSize / 10, StoppingCriterionPtr(), 0);
    runOptimizer(context, "EDA(10)", optimizer, objectives);

    populationSize = 100;
    optimizer = edaOptimizer(numIterations / populationSize, populationSize, populationSize / 10, StoppingCriterionPtr(), 0);
    runOptimizer(context, "EDA(100)", optimizer, objectives);
    
    populationSize = 1000;
    optimizer = edaOptimizer(numIterations / populationSize, populationSize, populationSize / 10, StoppingCriterionPtr(), 0);
    runOptimizer(context, "EDA(1000)", optimizer, objectives);

    optimizer = new HOOOptimizer(numIterations, nu, rho, C);
    runOptimizer(context, "HOO", optimizer, objectives);
*/
    /*
    context.enterScope(T("HOO"));
    optimizer = new HOOOptimizer(numIterations, nu, rho, C);
    tuneOptimizerVariable(context, optimizer, "C", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nu", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "rho", 0.0, 1.0, objectives);
    tuneOptimizerVariable(context, optimizer, "maxDepth", 1.0, 30.0, objectives);
    tuneOptimizerVariable(context, optimizer, "C", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nu", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "rho", 0.0, 1.0, objectives);
    tuneOptimizerVariable(context, optimizer, "maxDepth", 1.0, 30.0, objectives);
    context.enterScope(T("evaluate"));
    meanCumulativeRegret = runOptimizer(context, "HOO", optimizer, objectives, 10);
    context.leaveScope();
    context.leaveScope(meanCumulativeRegret);*/

    context.enterScope("RF(10) HOO");
    optimizer = new RFHOOOptimizer(10, K, nMin, maxDepth, numIterations, nu, rho, C);
    tuneOptimizerVariable(context, optimizer, "C", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nu", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nMin", 0.0, 16.0, objectives);
    tuneOptimizerVariable(context, optimizer, "maxDepth", 1.0, 30.0, objectives);
    tuneOptimizerVariable(context, optimizer, "C", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nu", 0.0, 2.0, objectives);
    tuneOptimizerVariable(context, optimizer, "nMin", 0.0, 16.0, objectives);
    tuneOptimizerVariable(context, optimizer, "maxDepth", 1.0, 30.0, objectives);
    context.enterScope(T("evaluate"));
    meanCumulativeRegret = runOptimizer(context, "RF(10) HOO", optimizer, objectives, 10);
    context.leaveScope();
    context.leaveScope(meanCumulativeRegret);
    
  
    /*runOptimizer(context, "hoo", new HOOOptimizer(numIterations, nu, rho, C), objectives);
    runOptimizer(context, "rfhoo-1", new RFHOOOptimizer(1, K, nMin, maxDepth, numIterations, nu, rho, C), objectives);
    runOptimizer(context, "rfhoo-5", new RFHOOOptimizer(5, K, nMin, maxDepth, numIterations, nu, rho, C), objectives);
    runOptimizer(context, "rfhoo-10", new RFHOOOptimizer(10, K, nMin, maxDepth, numIterations, nu, rho, C), objectives);
    runOptimizer(context, "rfhoo-50", new RFHOOOptimizer(50, K, nMin, maxDepth, numIterations, nu, rho, C), objectives);
    runOptimizer(context, "rfhoo-100", new RFHOOOptimizer(100, K, nMin, maxDepth, numIterations, nu, rho, C), objectives);*/
    return true;
  }

protected:
  friend class RFHOOOSandBoxClass;

  size_t dimension;

  size_t K;
  size_t nMin;
  size_t maxDepth;
  size_t maxHorizon;
  
  double nu;
  double rho;
  double C;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_RF_HOO_SAND_BOX_H_

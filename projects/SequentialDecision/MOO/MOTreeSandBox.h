/*-----------------------------------------.---------------------------------.
| Filename: MOTreeSandBox.h                | Multi Objective Regression Tree |
| Author  : Francis Maes                   | SandBox                         |
| Started : 16/09/2012 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_MO_TREE_SANDBOX_H_
# define LBCPP_MOO_MO_TREE_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "MOOCore.h"
# include "SharkProblems.h"
# include "UniformContinuousSampler.h"

namespace lbcpp
{

class MultiObjectiveRegressionTree : public Object
{
public: 
  MultiObjectiveRegressionTree(size_t minSamplesToSplit)
    : numNodes(0), minSamplesToSplit(minSamplesToSplit)
    {root = createNode();}
  MultiObjectiveRegressionTree()
    : root(NULL), numNodes(0), minSamplesToSplit(0) {}
  virtual ~MultiObjectiveRegressionTree()
    {if (root) delete root;}

  typedef std::pair<DenseDoubleVectorPtr, std::vector<double> > Sample;

  struct Node
  {
    Node(size_t uid)
      : uid(uid), splitDimension((size_t)-1), splitThreshold(0.0), left(NULL), right(NULL) {}
    ~Node()
      {if (left) delete left; if (right) delete right;}

    size_t uid;
    std::vector<ScalarVariableStatistics> stats;

    void observeOutput(const std::vector<double>& output)
    {
      if (stats.empty())
        stats.resize(output.size());
      for (size_t i = 0; i < output.size(); ++i)
        stats[i].push(output[i]);
    }

    // internal nodes
    bool isInternal() const
      {return left && right;}
    
    size_t splitDimension;
    double splitThreshold;
    Node* left;
    Node* right;

    // leaf nodes
    bool isLeaf() const
      {return !isInternal();}

    std::vector<Sample> samples;
  };

  Node* getRoot() const
    {return root;}

  size_t getNumNodes() const
    {return numNodes;}

  std::vector<bool> getTrajectory(const DenseDoubleVectorPtr& input) const
  {
    Node* node = root;
    std::vector<bool> res;
    while (node->isInternal())
    {
      if (input->getValue(node->splitDimension) < node->splitThreshold)
      {
        res.push_back(false);
        node = node->left;
      }
      else
      {
        res.push_back(true);
        node = node->right;
      }
    }
    return res;
  }

  Node* findLeafForInput(const DenseDoubleVectorPtr& input) const
  {
    Node* node = root;
    while (node->isInternal())
    {
      if (input->getValue(node->splitDimension) < node->splitThreshold)
        node = node->left;
      else
        node = node->right;
    }
    return node;
  }

  std::vector<double> predict(const DenseDoubleVectorPtr& input) const
  {
    Node* leaf = findLeafForInput(input);
    jassert(leaf);
    std::vector<double> res(leaf->stats.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = leaf->stats[i].getMean();
    return res;
  }

  void addSample(ExecutionContext& context, const DenseDoubleVectorPtr& input, const std::vector<double>& output)
  {
    Node* node = root;
    while (node->isInternal())
    {
      node->observeOutput(output);
      if (input->getValue(node->splitDimension) < node->splitThreshold)
        node = node->left;
      else
        node = node->right;
    }
    addSample(context, node, input, output);
  }

  void storeSample(Node* node, const DenseDoubleVectorPtr& input, const std::vector<double>& output)
  {
    node->samples.push_back(std::make_pair(input, output));
    node->observeOutput(output);
  }

  void addSample(ExecutionContext& context, Node* node, const DenseDoubleVectorPtr& input, const std::vector<double>& output)
  {
    jassert(node->isLeaf());

    // store sample
    storeSample(node, input, output);

    // see if node must be splitted
    if (node->samples.size() >= minSamplesToSplit && !isConstant(node->stats))
      splitNode(context, node);
  }

  void splitNode(ExecutionContext& context, Node* node)
  {
    if (chooseSplit(context, node))
    {
      // create sub nodes
      node->left = createNode();
      node->right = createNode();

      // dispatch samples
      for (size_t i = 0; i < node->samples.size(); ++i)
      {
        const Sample& sample = node->samples[i];
        Node* subNode;
        if (sample.first->getValue(node->splitDimension) < node->splitThreshold)
          subNode = node->left;
        else
          subNode = node->right;
        storeSample(subNode, sample.first, sample.second);
      }
      jassert(node->left->samples.size() < node->samples.size());
      jassert(node->right->samples.size() < node->samples.size());
      node->samples.clear();
    }
  }

  bool chooseSplit(ExecutionContext& context, Node* node)
  {
    size_t d = node->samples[0].first->getNumValues();
    double currentVariance = computeSplitVariance(node->stats, std::vector<ScalarVariableStatistics>(node->stats.size())); // current Variance

    std::vector<std::pair<size_t, double> > bestSplits;
    double bestVariance = currentVariance;
    for (size_t i = 0; i < d; ++i)
    {
      double threshold;
      double variance = findBestThreshold(node->samples, i, threshold);
      //std::cout << "Dim " << i << " Variance = " << score << " Threshold = " << threshold << std::endl;
      if (variance <= bestVariance)
      {
        if (variance < bestVariance)
        {
          bestSplits.clear();
          bestVariance = variance;
        }
        bestSplits.push_back(std::make_pair(i, threshold));
      }
    }
    if (bestVariance == currentVariance || bestSplits.empty())
      return false;

    // sample one split randomly
    size_t index = (bestSplits.size() == 1 ? 0 : context.getRandomGenerator()->sampleSize(bestSplits.size()));
    node->splitDimension = bestSplits[index].first;
    node->splitThreshold = bestSplits[index].second;
    return true;
  }

  bool isConstant(const std::vector<ScalarVariableStatistics>& stats) const
  {
    for (size_t i = 0; i < stats.size(); ++i)
      if (stats[i].getStandardDeviation() > 0.0)
        return false;
    return true;
  }

  void observe(std::vector<ScalarVariableStatistics>& stats, const std::vector<double>& value, double weight)
  {
    jassert(value.size() == stats.size());
    for (size_t i = 0; i < stats.size(); ++i)
      stats[i].push(value[i], weight);
  }

  // returns the variance (should be minimized)
  double findBestThreshold(const std::vector<Sample>& samples, size_t splitDimension, double& bestThreshold)
  {
    jassert(samples.size());

    size_t numOutputs = samples[0].second.size();

    std::vector<ScalarVariableStatistics> negatives(numOutputs);
    std::vector<ScalarVariableStatistics> positives(numOutputs);

    std::map<double, std::vector<size_t> > sorted;
    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first->getValue(splitDimension);
      jassert(isNumberValid(value));
      observe(positives, samples[i].second, 1.0);
      sorted[value].push_back(i);
    }

    if (sorted.size() == 1 || (sorted.rbegin()->first - sorted.begin()->first < 1e-9))
      return DBL_MAX;

    double bestVariance = DBL_MAX;
    bestThreshold = -DBL_MAX;

    std::map<double, std::vector<size_t> >::const_iterator it, nxt;
    it = sorted.begin();
    nxt = it; ++nxt;
    for ( ; nxt != sorted.end(); ++it, ++nxt)
    {
      double threshold = (it->first + nxt->first) / 2.0;
      for (size_t i = 0; i < it->second.size(); ++i)
      {
        const std::vector<double>& output = samples[it->second[i]].second;
        observe(positives, output, -1.0);
        observe(negatives, output, 1.0);
      }
      double variance = computeSplitVariance(negatives, positives);
      if (variance < bestVariance)
        bestVariance = variance, bestThreshold = threshold;
    }
    return bestVariance;
  }

  double computeSplitVariance(const std::vector<ScalarVariableStatistics>& negatives,
                              const std::vector<ScalarVariableStatistics>& positives) const
  {
    size_t n = negatives.size();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += computeSplitVariance(negatives[i], positives[i]);
    return res;
  }

  double computeSplitVariance(const ScalarVariableStatistics& negatives, const ScalarVariableStatistics& positives) const
  {
    size_t p = (size_t)positives.getCount();
    size_t n = (size_t)negatives.getCount();
    double res = 0.0;
    if (p)
      res += p * positives.getVariance();
    if (n)
      res += n * negatives.getVariance();
    if (p || n)
      res /= (double)(p + n);
    return res;
  }

private:
  Node* root;
  size_t numNodes;
  size_t minSamplesToSplit;

  Node* createNode()
    {return new Node(numNodes++);}
};

typedef ReferenceCountedObjectPtr<MultiObjectiveRegressionTree> MultiObjectiveRegressionTreePtr;

class MOTreeSampler : public MOOSampler
{
public:
  MOTreeSampler(MultiObjectiveRegressionTreePtr tree = MultiObjectiveRegressionTreePtr())
    : tree(tree) {}

  typedef MultiObjectiveRegressionTree::Node Node;
 
  virtual double getProbabilityToSelectRight(Node* node) const = 0;

  virtual void initialize(ExecutionContext& context, const MOODomainPtr& d)
    {domain = d.staticCast<ContinuousMOODomain>(); jassert(domain);}
 
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    Node* node = tree->getRoot();
    std::vector<std::pair<double, double> > subDomain = domain->getLimits();
    while (node->isInternal())
    {
      if (random->sampleBool(getProbabilityToSelectRight(node)))
      {
        subDomain[node->splitDimension].first = node->splitThreshold;
        node = node->right;
      }
      else
      {
        subDomain[node->splitDimension].second = node->splitThreshold;
        node = node->left;
      }
    }
    return sampleUniformly(random, subDomain);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<MOTreeSampler>& target = t.staticCast<MOTreeSampler>();
    target->tree = tree;
    target->domain = domain;
  }

protected:
  friend class MOTreeSamplerClass;

  MultiObjectiveRegressionTreePtr tree;
  ContinuousMOODomainPtr domain;

  DenseDoubleVectorPtr sampleUniformly(RandomGeneratorPtr random, const std::vector<std::pair<double, double> >& subDomain) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(subDomain.size(), 0.0);
    for (size_t i = 0; i < res->getNumValues(); ++i)
      res->setValue(i, random->sampleDouble(subDomain[i].first, subDomain[i].second));
    return res;
  }
};

class EpsilonGreedyMOTreeSampler : public MOTreeSampler
{
public:
  EpsilonGreedyMOTreeSampler(MultiObjectiveRegressionTreePtr tree, double epsilon = 0.05)
    : MOTreeSampler(tree), epsilon(epsilon) {}
  EpsilonGreedyMOTreeSampler() : epsilon(0.0) {}
 
  virtual double getProbabilityToSelectRight(Node* node) const
  {
    bool isRightBest = node->right->stats[0].getMinimum() < node->left->stats[0].getMinimum(); // BIG HACK for 1D minimization !
    return isRightBest ? 1.0 - epsilon : epsilon;
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects) {}
  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object) {}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    MOTreeSampler::clone(context, t);
    const ReferenceCountedObjectPtr<EpsilonGreedyMOTreeSampler>& target = t.staticCast<EpsilonGreedyMOTreeSampler>();
    target->epsilon = epsilon;
  }

protected:
  friend class EpsilonGreedyMOTreeSamplerClass;

  double epsilon;
};

class AdaptativeMOTreeSampler : public MOTreeSampler
{
public:
  AdaptativeMOTreeSampler(MultiObjectiveRegressionTreePtr tree = MultiObjectiveRegressionTreePtr(), double learningRate = 1.0)
    : MOTreeSampler(tree), learningRate(learningRate) {}

  virtual void initialize(ExecutionContext& context, const MOODomainPtr& domain)
  {
    MOTreeSampler::initialize(context, domain);
    size_t n = this->domain->getNumDimensions();
    if (!weights)
      weights = new DenseDoubleVector(tree->getNumNodes(), 0.0);
  }
 
  virtual double getProbabilityToSelectRight(Node* node) const
  {
    double a = exp(getWeight(node->left));
    double b = exp(getWeight(node->right));
    return b / (a + b);
  }
 
  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
  {
    std::vector< std::vector<bool> > trajectories(objects.size());
    std::vector<size_t> indices(objects.size());
    for (size_t i = 0; i < objects.size(); ++i)
    {
      trajectories[i] = tree->getTrajectory(objects[i].staticCast<DenseDoubleVector>());
      indices[i] = i;
    }
    learnRecursively(trajectories, indices, 0, tree->getRoot());
  }

  void learnRecursively(const std::vector< std::vector<bool> >& trajectories, const std::vector<size_t>& indices, size_t depth, Node* node)
  {
    if (indices.empty() || node->isLeaf())
      return;
    
    size_t numRight = 0;
    std::vector<size_t> leftIndices, rightIndices;
    leftIndices.reserve(indices.size());
    rightIndices.reserve(indices.size());

    for (size_t i = 0; i < indices.size(); ++i)
    {
      size_t index = indices[i];
      const std::vector<bool>& trajectory = trajectories[index];
      if (trajectory[depth])
        rightIndices.push_back(index);
      else
        leftIndices.push_back(index);
    }
    
    learnRecursively(trajectories, leftIndices, depth + 1, node->left);
    setProbability(node->left, leftIndices.size() / (double)indices.size());

    learnRecursively(trajectories, rightIndices, depth + 1, node->right);
    setProbability(node->right, rightIndices.size() / (double)indices.size());
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
  {
    std::vector<bool> trajectory = tree->getTrajectory(object.staticCast<DenseDoubleVector>());

    Node* node = tree->getRoot();
    for (size_t i = 0; i < trajectory.size(); ++i)
    {
      double a = exp(getWeight(node->left));
      double b = exp(getWeight(node->right));
      double pRight = b / (a + b);
      bool isRight = trajectory[i];

      incrementWeight(node->left, learningRate * ((!isRight ? 1.0 : 0.0) - (1.0 - pRight)));
      incrementWeight(node->right, learningRate * ((isRight ? 1.0 : 0.0) - pRight));
      node = isRight ? node->right : node->left;
    }
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    MOTreeSampler::clone(context, t);

    const ReferenceCountedObjectPtr<AdaptativeMOTreeSampler>& target = t.staticCast<AdaptativeMOTreeSampler>();
    target->learningRate = learningRate;
    target->weights = weights ? weights->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
  }

protected:
  friend class AdaptativeMOTreeSamplerClass;

  double learningRate;
  DenseDoubleVectorPtr weights;

  void setProbability(Node* node, double probability)
    {setWeight(node, probability ? log(probability) : -DBL_MAX);}

  void setWeight(Node* node, double weight)
  {
    size_t index = node->uid;
    weights->ensureSize(index + 1);
    weights->setValue(index, weight);
  }
  
  void incrementWeight(Node* node, double delta)
  {
    size_t index = node->uid;
    weights->ensureSize(index + 1);
    weights->incrementValue(index, delta);
  }

  double getWeight(Node* node) const
  {
    size_t uid = node->uid;
    return uid < weights->getNumValues() ? weights->getValue(uid) : 0.0;
  }
};

class LearnMultiObjectiveRegressionTreeDecorator : public DecoratorMOOProblem
{
public:
  LearnMultiObjectiveRegressionTreeDecorator(MOOProblemPtr problem, MultiObjectiveRegressionTreePtr tree)
    : DecoratorMOOProblem(problem), tree(tree) {}

  virtual String toShortString() const
    {return problem->toShortString();}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    MOOFitnessPtr res = DecoratorMOOProblem::evaluate(context, solution);
    tree->addSample(context, solution.staticCast<DenseDoubleVector>(), res->getValues());
    return res;
  }

protected:
  MultiObjectiveRegressionTreePtr tree;
};

class MOTreeSandBox : public WorkUnit
{
public:
  MOTreeSandBox() : numEvaluations(1000), minSamplesToSplit(10), verbosity(1) {}
  
  typedef MultiObjectiveRegressionTree::Sample Sample;
    
  virtual Variable run(ExecutionContext& context)
  {
    testSingleObjectiveOptimizers(context);
    //testMultiObjectiveOptimization(context);
    //testLearningND(context);
    //testLearning1D(context);
    return true;
  }

protected:
  friend class MOTreeSandBoxClass;

  size_t numEvaluations;
  size_t minSamplesToSplit;
  size_t verbosity;

  /*
  ** Single Objective
  */
  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new AckleyProblem(5));
    problems.push_back(new GriewangkProblem(5));
    problems.push_back(new RastriginProblem(5));
    problems.push_back(new RosenbrockProblem(5));
    problems.push_back(new RosenbrockRotatedProblem(5));

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, randomOptimizer(new UniformContinuousSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, false));
      solveWithSingleObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));
      MultiObjectiveRegressionTreePtr tree;
      MOOProblemPtr decoratedProblem;
/*
      tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
      decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
      solveWithSingleObjectiveOptimizer(context, decoratedProblem, new CrossEntropyOptimizer(new TreeBasedContinuousSampler(tree), 100, 50, numEvaluations / 100, false));

      tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
      decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
      solveWithSingleObjectiveOptimizer(context, decoratedProblem, new CrossEntropyOptimizer(new TreeBasedContinuousSampler(tree), 100, 50, numEvaluations / 100, true));*/
/*
      for (double epsilon = 0.0; epsilon <= 0.5; epsilon *= 2)
      {
        MultiObjectiveRegressionTreePtr tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
        MOOProblemPtr decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
        solveWithSingleObjectiveOptimizer(context, decoratedProblem, randomOptimizer(new EpsilonGreedyMOTreeSampler(tree, epsilon), numEvaluations));
        if (!epsilon)
          epsilon = 0.0001;
      }
      */

      tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
      decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
      solveWithSingleObjectiveOptimizer(context, decoratedProblem, new NRPAOptimizer(new AdaptativeMOTreeSampler(tree), 2, 100));

      context.leaveScope(); 
    }
  }

  double solveWithSingleObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    SingleObjectiveEvaluatorDecoratorProblemPtr decorator(new SingleObjectiveEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr front = optimizer->optimize(context, decorator, (MOOOptimizer::Verbosity)verbosity);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("front", front);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    if (verbosity >= 1)
    {
      context.enterScope("curve");
      std::vector<double> cpuTimes = decorator->getCpuTimes();
      std::vector<double> scores = decorator->getScores();

      for (size_t i = 0; i < scores.size(); ++i)
      {
        size_t numEvaluations = i * decorator->getEvaluationPeriod();
        context.enterScope(String((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("score", scores[i]);
        context.resultCallback("cpuTime", cpuTimes[i]);
        context.leaveScope();
      }
      context.leaveScope();
    }

    jassert(!front->isEmpty());
    double score = front->getSolution(0)->getFitness()->getValue(0);
    context.resultCallback("score", score);
    context.leaveScope(score);
    return score;
  }

  /*
  ** Test MultiObjective optimization
  */
  void testMultiObjectiveOptimization(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new ZDT1MOProblem());
    problems.push_back(new ZDT2MOProblem());
    problems.push_back(new ZDT3MOProblem());
    problems.push_back(new ZDT4MOProblem());
    problems.push_back(new ZDT6MOProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithMultiObjectiveOptimizer(context, problem, randomOptimizer(new UniformContinuousSampler(), numEvaluations));
      //solveWithMultiObjectiveOptimizer(context, problem, new NSGA2MOOptimizer(100, numEvaluations / 100));
      //solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100, numEvaluations / 100));

      //solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, false));
      //solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));

      MultiObjectiveRegressionTreePtr tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
      MOOProblemPtr decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
      solveWithMultiObjectiveOptimizer(context, decoratedProblem, new CrossEntropyOptimizer(new AdaptativeMOTreeSampler(tree), 100, 50, numEvaluations / 100, false));

      tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
      decoratedProblem = new LearnMultiObjectiveRegressionTreeDecorator(problem, tree);
      solveWithMultiObjectiveOptimizer(context, decoratedProblem, new CrossEntropyOptimizer(new AdaptativeMOTreeSampler(tree), 100, 50, numEvaluations / 100, true));

      context.leaveScope();
    }
  }

  void solveWithMultiObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr front = optimizer->optimize(context, decorator, (MOOOptimizer::Verbosity)verbosity);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    if (verbosity >= 1)
    {
      context.enterScope("curve");
      std::vector<double> hyperVolumes = decorator->getHyperVolumes();
      std::vector<double> cpuTimes = decorator->getCpuTimes();

      for (size_t i = 0; i < hyperVolumes.size(); ++i)
      {
        size_t numEvaluations = i * decorator->getEvaluationPeriod();
        context.enterScope(String((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("hyperVolume", hyperVolumes[i]);
        context.resultCallback("cpuTime", cpuTimes[i]);
        context.leaveScope();
      }
      context.leaveScope();
    }

    context.leaveScope(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }

  /*
  ** Test Learning ND
  */
  void testLearningND(ExecutionContext& context)
  {
    MOOProblemPtr problem = new ZDT1MOProblem();

    std::vector<Sample> trainingSamples = makeSamples(context, problem, 1000);
    std::vector<Sample> testingSamples = makeSamples(context, problem, 1000);

    context.enterScope("learning");
    MultiObjectiveRegressionTreePtr tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
    double startTime = Time::getMillisecondCounterHiRes();
    for (size_t i = 0; i < trainingSamples.size(); ++i)
    {
      context.enterScope(String((int)i));
      context.resultCallback("i", i);
      tree->addSample(context, trainingSamples[i].first, trainingSamples[i].second);
      context.resultCallback("trainError", computeError(tree, trainingSamples));
      context.resultCallback("testError", computeError(tree, testingSamples));
      context.resultCallback("cpuTime", Time::getMillisecondCounterHiRes() - startTime);
      context.leaveScope();
    }
    context.leaveScope();
  }

  std::vector<Sample> makeSamples(ExecutionContext& context, MOOProblemPtr problem, size_t count)
  {
    ContinuousMOODomainPtr domain = problem->getObjectDomain().staticCast<ContinuousMOODomain>();
    std::vector<Sample> res(count);
    for (size_t i = 0; i < count; ++i)
    {
      DenseDoubleVectorPtr input = domain->sampleUniformly(context.getRandomGenerator());
      MOOFitnessPtr fitness = problem->evaluate(context, input);
      res[i] = Sample(input, fitness->getValues());
    }
    return res;
  }

  double computeError(MultiObjectiveRegressionTreePtr tree, const std::vector<Sample>& samples)
  {
    double res = 0.0;
    for (size_t i = 0; i < samples.size(); ++i)
    {
      std::vector<double> predicted = tree->predict(samples[i].first);
      for (size_t j = 0; j < predicted.size(); ++j)
        res += fabs(predicted[j] - samples[i].second[j]);
    }
    return res / (samples.size() * samples[0].second.size());
  }

  /*
  ** Test Learning 1D
  */
  void testLearning1D(ExecutionContext& context)
  {
    MOOProblemPtr problem = new AckleyProblem(1);
    ContinuousMOODomainPtr domain = problem->getObjectDomain().staticCast<ContinuousMOODomain>();

    context.enterScope("problem");
    double xmin = domain->getLowerLimit(0);
    double xmax = domain->getUpperLimit(0);
    double xstep = (xmax - xmin) / 100.0;
    for (double x = xmin; x <= xmax; x += xstep)
    {
      context.enterScope(String(x));
      context.resultCallback("x", x);
      MOOFitnessPtr fitness = problem->evaluate(context, new DenseDoubleVector(1, x));
      for (size_t i = 0; i < fitness->getNumValues(); ++i)
        context.resultCallback("fitness" + String((int)i+1), fitness->getValue(i));
      context.leaveScope();
    }
    context.leaveScope();

    MOOSamplerPtr sampler = new UniformContinuousSampler();
    sampler->initialize(context, domain);
    
    MultiObjectiveRegressionTreePtr tree = new MultiObjectiveRegressionTree(minSamplesToSplit);
    addSamplesToTree(context, problem, sampler, tree, 10);
    displayTreePredictions(context, "After 10 samples", domain, tree);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.001)", domain, tree, 0.001);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.01)", domain, tree, 0.01);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.1)", domain, tree, 0.1);

    addSamplesToTree(context, problem, sampler, tree, 90);
    displayTreePredictions(context, "After 100 samples", domain, tree);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.001)", domain, tree, 0.001);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.01)", domain, tree, 0.01);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.1)", domain, tree, 0.1);

    addSamplesToTree(context, problem, sampler, tree, 900);
    displayTreePredictions(context, "After 1000 samples", domain, tree);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.001)", domain, tree, 0.001);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.01)", domain, tree, 0.01);
    displayEpsilonGreedyHistogram(context, "EpsGreedy(0.1)", domain, tree, 0.1);
  }

  void addSamplesToTree(ExecutionContext& context, const MOOProblemPtr& problem, const MOOSamplerPtr& sampler, MultiObjectiveRegressionTreePtr tree, size_t count)
  {
    for (size_t i = 0; i < count; ++i)
    {
      DenseDoubleVectorPtr input = sampler->sample(context).staticCast<DenseDoubleVector>();
      MOOFitnessPtr fitness = problem->evaluate(context, input);
      tree->addSample(context, input, fitness->getValues());
    }
  }

  void displayEpsilonGreedyHistogram(ExecutionContext& context, const String& name, ContinuousMOODomainPtr domain, MultiObjectiveRegressionTreePtr tree, double epsilon)
  {
    context.enterScope(name);
    double xmin = domain->getLowerLimit(0);
    double xmax = domain->getUpperLimit(0);
    double xstep = (xmax - xmin) / 100.0;

    MOOSamplerPtr sampler(new EpsilonGreedyMOTreeSampler(tree, epsilon));
    sampler->initialize(context, domain);
    
    std::vector<size_t> histogram((size_t)(xmax - xmin) / xstep + 1, 0);
    size_t count = 100000;
    for (size_t i = 0; i < count; ++i)
    {
      double x = sampler->sample(context).staticCast<DenseDoubleVector>()->getValue(0);
      histogram[(size_t)((x - xmin) / xstep)]++;
    }

    for (size_t i = 0; i < histogram.size() - 1; ++i)
    {
      context.enterScope(String((int)i));
      context.resultCallback("x", xmin + xstep * (i + 0.5));
      context.resultCallback("frequency", histogram[i] / (double)count);
      context.leaveScope();
    }
    context.leaveScope();
  }

  void displayTreePredictions(ExecutionContext& context, const String& name, ContinuousMOODomainPtr domain, MultiObjectiveRegressionTreePtr tree)
  {
    context.enterScope(name);
    double xmin = domain->getLowerLimit(0);
    double xmax = domain->getUpperLimit(0);
    double xstep = (xmax - xmin) / 100.0;
    for (double x = xmin; x <= xmax; x += xstep)
    {
      context.enterScope(String(x));
      context.resultCallback("x", x);
      std::vector<double> values = tree->predict(new DenseDoubleVector(1, x));
      for (size_t i = 0; i < values.size(); ++i)
        context.resultCallback("fitness" + String((int)i+1), values[i]);
      context.leaveScope();
    }
    context.leaveScope();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_MO_TREE_SANDBOX_H_

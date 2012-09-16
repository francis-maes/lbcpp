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

    std::vector<Sample> samples;
    std::vector<ScalarVariableStatistics> stats;

    size_t splitDimension;
    double splitThreshold;

    bool isInternal() const
      {return left && right;}

    bool isLeaf() const
      {return !isInternal();}

    Node* left;
    Node* right;
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

  void addSample(const DenseDoubleVectorPtr& input, const std::vector<double>& output)
    {addSample(findLeafForInput(input), input, output);}

  void addSample(Node* node, const DenseDoubleVectorPtr& input, const std::vector<double>& output)
  {
    jassert(node->isLeaf());
    // update statistics
    node->stats.resize(output.size());
    for (size_t i = 0; i < output.size(); ++i)
      node->stats[i].push(output[i]);
    node->samples.push_back(std::make_pair(input, output));

    // see if node must be splitted
    if (node->samples.size() >= minSamplesToSplit && !isConstant(node->stats))
      splitNode(node);
  }

  void splitNode(Node* node)
  {
    if (chooseSplit(node))
    {
      // create sub nodes
      node->left = createNode();
      node->right = createNode();

      // dispatch samples
      for (size_t i = 0; i < node->samples.size(); ++i)
      {
        const Sample& sample = node->samples[i];
        if (sample.first->getValue(node->splitDimension) < node->splitThreshold)
          addSample(node->left, sample.first, sample.second);
        else
          addSample(node->right, sample.first, sample.second);
      }
      node->samples.clear();
    }
  }

  bool chooseSplit(Node* node)
  {
    size_t d = node->samples[0].first->getNumValues();
    double bestScore = DBL_MAX;

    for (size_t i = 0; i < d; ++i)
    {
      double threshold;
      double score = findBestThreshold(node->samples, i, threshold);
      //std::cout << "Dim " << i << " Variance = " << score << " Threshold = " << threshold << std::endl;
      if (score < bestScore)
      {
        bestScore = score;
        node->splitDimension = i;
        node->splitThreshold = threshold;
      }
    }
    return bestScore != DBL_MAX;
  }

  bool isConstant(const std::vector<ScalarVariableStatistics>& stats) const
  {
    for (size_t i = 0; i < stats.size(); ++i)
      if (stats[i].getStandardDeviation() > 0.0)
        return false;
    return true;
  }

  void observe(std::vector<ScalarVariableMeanAndVariance>& stats, const std::vector<double>& value, double weight)
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

    std::vector<ScalarVariableMeanAndVariance> negatives(numOutputs);
    std::vector<ScalarVariableMeanAndVariance> positives(numOutputs);

    std::map<double, std::vector<size_t> > sorted;
    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first->getValue(splitDimension);
      jassert(isNumberValid(value));
      observe(positives, samples[i].second, 1.0);
      sorted[value].push_back(i);
    }

    if (sorted.size() == 1)
      return DBL_MAX;

    double bestVariance = computeSplitVariance(negatives, positives);
    bestThreshold = sorted.begin()->first - 1.0;

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

  double computeSplitVariance(const std::vector<ScalarVariableMeanAndVariance>& negatives,
                              const std::vector<ScalarVariableMeanAndVariance>& positives) const
  {
    size_t n = negatives.size();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += computeSplitVariance(negatives[i], positives[i]);
    return res;
  }

  double computeSplitVariance(const ScalarVariableMeanAndVariance& negatives, const ScalarVariableMeanAndVariance& positives) const
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

class TreeBasedContinuousSampler : public MOOSampler
{
public:
  TreeBasedContinuousSampler(MultiObjectiveRegressionTreePtr tree = MultiObjectiveRegressionTreePtr(), double learningRate = 1.0)
    : tree(tree), learningRate(learningRate) {}

  typedef MultiObjectiveRegressionTree::Node Node;

  virtual void initialize(ExecutionContext& context, const MOODomainPtr& d)
  {
    domain = d.staticCast<ContinuousMOODomain>();
    jassert(domain);
    size_t n = domain->getNumDimensions();
    if (!weights)
      weights = new DenseDoubleVector(tree->getNumNodes(), 0.0);
  }
 
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    Node* node = tree->getRoot();
    std::vector<std::pair<double, double> > subDomain = domain->getLimits();
    while (node->isInternal())
    {
      double a = exp(getWeight(node->left));
      double b = exp(getWeight(node->right));
      if (random->sampleBool(a / (a + b)))
      {
        subDomain[node->splitDimension].second = node->splitThreshold;
        node = node->left;
      }
      else
      {
        subDomain[node->splitDimension].first = node->splitThreshold;
        node = node->right;
      }
    }

    DenseDoubleVectorPtr res = new DenseDoubleVector(subDomain.size(), 0.0);
    for (size_t i = 0; i < res->getNumValues(); ++i)
      res->setValue(i, random->sampleDouble(subDomain[i].first, subDomain[i].second));
    return res;
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
    const ReferenceCountedObjectPtr<TreeBasedContinuousSampler>& target = t.staticCast<TreeBasedContinuousSampler>();
    target->tree = tree;
    target->learningRate = learningRate;
    target->weights = weights ? weights->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
  }

protected:
  friend class TreeBasedContinuousSamplerClass;

  MultiObjectiveRegressionTreePtr tree;
  double learningRate;
  DenseDoubleVectorPtr weights;

  ContinuousMOODomainPtr domain;

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

class MOTreeSandBox : public WorkUnit
{
public:
  MOTreeSandBox() : minSamplesToSplit(10) {}
  
  typedef MultiObjectiveRegressionTree::Sample Sample;
    
  virtual Variable run(ExecutionContext& context)
  {
    testLearningND(context);
    return true;
  }

protected:
  friend class MOTreeSandBoxClass;

  size_t minSamplesToSplit;

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
      tree->addSample(trainingSamples[i].first, trainingSamples[i].second);
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
    addSamplesToTree(context, problem, sampler, tree, 90);
    displayTreePredictions(context, "After 100 samples", domain, tree);
    addSamplesToTree(context, problem, sampler, tree, 900);
    displayTreePredictions(context, "After 1000 samples", domain, tree);
  }

  void addSamplesToTree(ExecutionContext& context, const MOOProblemPtr& problem, const MOOSamplerPtr& sampler, MultiObjectiveRegressionTreePtr tree, size_t count)
  {
    for (size_t i = 0; i < count; ++i)
    {
      DenseDoubleVectorPtr input = sampler->sample(context).staticCast<DenseDoubleVector>();
      MOOFitnessPtr fitness = problem->evaluate(context, input);
      tree->addSample(input, fitness->getValues());
    }
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

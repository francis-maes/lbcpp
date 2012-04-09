/*-----------------------------------------.---------------------------------.
| Filename: LuapeFormulaDiscovery.h        | Luape Formula Discovery         |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2012 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FORMULA_DISCOVERY_H_
# define LBCPP_LUAPE_FORMULA_DISCOVERY_H_

# include "MCAlgorithm.h"
# include "MCBanditPool.h"
# include "../GP/BanditFormulaSearchProblem.h"

namespace lbcpp
{

class LuapeNodeSearchProblem : public LuapeInference
{
public:
  virtual bool initializeProblem(ExecutionContext& context) = 0;

//  virtual size_t getNumInstances() const {return 0;} // 0 stands for infinity
  virtual void getObjectiveRange(double& worst, double& best) const = 0;
  virtual double computeObjective(ExecutionContext& context, const LuapeNodePtr& node, size_t instanceIndex) = 0;

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const LuapeNodePtr& node) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeNodeSearchProblem> LuapeNodeSearchProblemPtr;

//////////////////////////////////////////////////////////

class LuapeBanditFormulaSearchProblem : public LuapeNodeSearchProblem
{
public:
  LuapeBanditFormulaSearchProblem(BanditProblemSamplerPtr problemSampler, size_t horizon)
    : problemSampler(problemSampler), horizon(horizon)
  {
  }
  LuapeBanditFormulaSearchProblem() : horizon(0) {}

  virtual bool initializeProblem(ExecutionContext& context)
  {
    addConstant(1.0);
    addConstant(2.0);
    addConstant(3.0);
    addConstant(5.0);
    addConstant(7.0);

    addInput(doubleType, "rk");
    addInput(doubleType, "sk");
    addInput(doubleType, "tk");
    addInput(doubleType, "t");

    addFunction(oppositeDoubleLuapeFunction());
    addFunction(inverseDoubleLuapeFunction());
    addFunction(sqrtDoubleLuapeFunction());
    addFunction(logDoubleLuapeFunction());
    addFunction(absDoubleLuapeFunction());

    addFunction(addDoubleLuapeFunction());
    addFunction(subDoubleLuapeFunction());
    addFunction(mulDoubleLuapeFunction());
    addFunction(divDoubleLuapeFunction());
    addFunction(minDoubleLuapeFunction());
    addFunction(maxDoubleLuapeFunction());

    addTargetType(doubleType);

    samplesCache = sampleInputs(context.getRandomGenerator());
    return true;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = (double)horizon; best = 0.0;}

  virtual double computeObjective(ExecutionContext& context, const LuapeNodePtr& node, size_t instanceIndex)
  {
    const std::vector<SamplerPtr>& arms = getProblem(context, instanceIndex);
    std::vector<double> expectedRewards(arms.size());
    double bestRewardExpectation = 0.0;
    double meanRewardExpectation = 0.0;
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double p = arms[i]->computeExpectation().getDouble();
      if (p > bestRewardExpectation) 
        bestRewardExpectation = p;
      meanRewardExpectation += p;
      expectedRewards[i] = p;
    }
    meanRewardExpectation /= arms.size();

    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);    
    DiscreteBanditPolicyPtr policy = new Policy(refCountedPointerFromThis(this), node);
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }
    return horizon * bestRewardExpectation - sumOfRewards;
  }

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const LuapeNodePtr& node) const
  {
    LuapeSampleVectorPtr samples = samplesCache->getSamples(context, node);
    if (!samples)
      return BinaryKeyPtr();

    LuapeSampleVector::const_iterator it = samples->begin();
    BinaryKeyPtr res = new BinaryKey(numBanditSamples * numArmsPerBanditSample);
    for (size_t i = 0; i < numBanditSamples; ++i)
    {
      std::vector<std::pair<size_t, double> > values(numArmsPerBanditSample);
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        double value = it.getRawDouble(); ++it;
        if (value == doubleMissingValue)
          return BinaryKeyPtr();
        values[j] = std::make_pair(j, value);
      }
      std::sort(values.begin(), values.end(), ValueComparator());
  
      jassert(numArmsPerBanditSample <= 128);
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        bool isHigherThanPrevious = (j > 0 && values[j].second > values[j-1].second);
        res->pushByte((unsigned char)(values[j].first + (isHigherThanPrevious ? 128 : 0)));
      }
    }
    return res;
  }

protected:
  friend class LuapeBanditFormulaSearchProblemClass;

  BanditProblemSamplerPtr problemSampler;
  size_t horizon;

private:
  std::vector< std::vector<SamplerPtr> > problems;
  LuapeSamplesCachePtr samplesCache;

  enum {numBanditSamples = 100, numArmsPerBanditSample = 5};

  LuapeSamplesCachePtr sampleInputs(RandomGeneratorPtr random) const
  {
    LuapeSamplesCachePtr res = createCache(numBanditSamples * numArmsPerBanditSample, 100); // 100 Mb cache

    for (size_t i = 0; i < numBanditSamples; ++i)
    {
      double t = juce::jmax(1, (int)pow(10.0, random->sampleDouble(-0.1, 5)));
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        DenseDoubleVectorPtr input = new DenseDoubleVector(4, 0.0);
        input->setValue(0, juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1))); // rk1
        input->setValue(1, juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1))); // sk1
        input->setValue(2, juce::jlimit(1, (int)t, (int)(t * random->sampleDouble(-0.1, 1.1)))); // tk1
        input->setValue(3, (double)t);
        res->setInputObject(inputs, i * numArmsPerBanditSample + j, input);
      }
    }
    return res;
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
      {return (fabs(left.second - right.second) > 1e-12 ? left.second < right.second : left.first < right.first);}
  };

  const std::vector<SamplerPtr>& getProblem(ExecutionContext& context, size_t index)
  {
    while (index >= problems.size())
      problems.push_back(problemSampler->sampleArms(context.getRandomGenerator()));
    return problems[index];
  }

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }

  struct Policy : public IndexBasedDiscreteBanditPolicy
  {
    Policy(LuapeInferencePtr problem, LuapeNodePtr formula)
      : problem(problem), formula(formula) {}

    LuapeInferencePtr problem;
    LuapeNodePtr formula;

    virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {
      BanditStatisticsPtr arm = banditStatistics[banditNumber];
      DenseDoubleVectorPtr input = new DenseDoubleVector(4, 0.0);
      input->setValue(0, arm->getRewardMean());
      input->setValue(1, arm->getRewardStandardDeviation());
      input->setValue(2, (double)arm->getPlayedCount());
      input->setValue(3, (double)timeStep);

      LuapeInstanceCachePtr cache = new LuapeInstanceCache();
      cache->setInputObject(problem->getInputs(), input);
      return cache->compute(defaultExecutionContext(), formula).toDouble();
    }
  };
};

//////////////////////////////////////////////////////////

class LuapeNodeEquivalenceClass : public Object
{
public:
  LuapeNodeEquivalenceClass(const LuapeNodePtr& node)
    : elements(1, node), representent(node) {}
  LuapeNodeEquivalenceClass() {}

  void add(LuapeNodePtr node)
  {
    elements.push_back(node);
    if (!representent || isSmallerThan(node, representent))
      representent = node;
  }

  size_t getNumElements() const
    {return elements.size();}

  const LuapeNodePtr& getRepresentent() const
    {return representent;}

  virtual String toShortString() const
    {return T("[") + representent->toShortString() + T("]");}

protected:
  std::vector<LuapeNodePtr> elements;
  LuapeNodePtr representent;

  bool isSmallerThan(const LuapeNodePtr& a, const LuapeNodePtr& b) const
  {
    size_t da = a->getDepth();
    size_t db = b->getDepth();
    if (da != db)
      return da < db;
    int ta = getNodeType(a);
    int tb = getNodeType(b);
    if (ta != tb)
      return ta < tb;
    if (ta == 0)
      return a.staticCast<LuapeConstantNode>()->getValue() < b.staticCast<LuapeConstantNode>()->getValue();
    return a->toShortString() < b->toShortString();
  }

  int getNodeType(const LuapeNodePtr& node) const
  {
    if (node.isInstanceOf<LuapeConstantNode>())
      return 0;
    else if (node.isInstanceOf<LuapeInputNode>())
      return 1;
    else
      return 2;
  }
};

typedef ReferenceCountedObjectPtr<LuapeNodeEquivalenceClass> LuapeNodeEquivalenceClassPtr;

class LuapeNodeEquivalenceClasses : public Object
{
public:
  void add(ExecutionContext& context, const std::vector<LuapeNodePtr>& nodes, LuapeNodeSearchProblemPtr problem, bool verbose = false)
  {
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      add(context, node, problem->makeBinaryKey(context, node), verbose);
    }
    if (verbose)
    {
      context.informationCallback(String((int)getNumClasses()) + T(" equivalence classes"));
      context.informationCallback(String((int)getNumInvalids()) + T(" invalid candidates"));
    }
  }

  void add(ExecutionContext& context, LuapeNodePtr node, BinaryKeyPtr key, bool verbose = false)
  {
    if (key)
    {
      Map::const_iterator it = m.find(key);
      if (it == m.end())
      {
        m[key] = new LuapeNodeEquivalenceClass(node);
        if (verbose && (m.size() % 1000) == 0)
          context.informationCallback(String((int)m.size()) + T(" equivalence classes, last: ") + m[key]->toShortString());
      }
      else
        it->second->add(node);
    }
    else
    {
      if (!invalids)
        invalids = new LuapeNodeEquivalenceClass(node);
      else
        invalids->add(node);
    }
  }

  size_t getNumClasses() const
    {return m.size();}

  size_t getNumInvalids() const
    {return invalids->getNumElements();}

  void addClassesToBanditPool(MCBanditPoolPtr pool)
  {
    for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
      pool->createArm(it->second->getRepresentent());
  }

protected:
  typedef std::map<BinaryKeyPtr, LuapeNodeEquivalenceClassPtr, ObjectComparator> Map; // todo: hash_map
  Map m;

  LuapeNodeEquivalenceClassPtr invalids;
};

typedef ReferenceCountedObjectPtr<LuapeNodeEquivalenceClasses> LuapeNodeEquivalenceClassesPtr;

//////////////////////////////////////////////////////////

class LuapeFormulaDiscoverySandBox : public WorkUnit
{
public:
  LuapeFormulaDiscoverySandBox() 
    : problem(new LuapeBanditFormulaSearchProblem(new Setup1BanditProblemSampler(), 100)),
    complexity(5), explorationCoefficient(5.0), numIterations(100), numStepsPerIteration(100), useMultiThreading(false) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem->initializeProblem(context))
      return false;

    context.enterScope(T("Enumerating candidates"));
    std::vector<LuapeNodePtr> candidates;
    problem->enumerateNodesExhaustively(context, complexity, candidates, true);
    context.leaveScope(candidates.size());
    
    context.enterScope(T("Making equivalence classes"));
    LuapeNodeEquivalenceClassesPtr equivalenceClasses = new LuapeNodeEquivalenceClasses();
    equivalenceClasses->add(context, candidates, problem, true);
    candidates.clear(); // free memory
    context.leaveScope(equivalenceClasses->getNumClasses());

    context.enterScope(T("Creating bandits"));
    MCBanditPoolPtr pool = new MCBanditPool(new ObjectiveWrapper(problem), explorationCoefficient, useMultiThreading); 
    equivalenceClasses->addClassesToBanditPool(pool);
    equivalenceClasses = LuapeNodeEquivalenceClassesPtr(); // free memory
    context.leaveScope();

    context.enterScope(T("Playing bandits"));
    pool->playIterations(context, numIterations, numStepsPerIteration);
    context.leaveScope();
    return true;
  }

protected:
  friend class LuapeFormulaDiscoverySandBoxClass;

  LuapeNodeSearchProblemPtr problem;
  size_t complexity;
  double explorationCoefficient;
  size_t numIterations;
  size_t numStepsPerIteration;
  bool useMultiThreading;

  struct ObjectiveWrapper : public SimpleBinaryFunction
  {
    ObjectiveWrapper(LuapeNodeSearchProblemPtr problem) 
      : SimpleBinaryFunction(luapeNodeClass, positiveIntegerType, doubleType), problem(problem)
      {problem->getObjectiveRange(worst, best);}

    LuapeNodeSearchProblemPtr problem;
    double worst, best;

    virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {
      double score = problem->computeObjective(context, inputs[0].getObjectAndCast<LuapeNode>(), (size_t)inputs[1].getInteger());
      return (score - worst) / (best - worst);
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_H_

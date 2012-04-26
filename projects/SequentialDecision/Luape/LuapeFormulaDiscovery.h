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

  virtual size_t getNumInstances() const {return 0;} // 0 stands for infinity
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
      Variable input[4];
      input[0] = arm->getRewardMean();
      input[1] = arm->getRewardStandardDeviation();
      input[2] = (double)arm->getPlayedCount();
      input[3] = (double)timeStep;
      return formula->compute(defaultExecutionContext(), input).toDouble();
    }
  };
};

//////////////////////////////////////////////////////////

class LuapeNodeEquivalenceClass : public Object
{
public:
  LuapeNodeEquivalenceClass(const LuapeNodePtr& node)
    {add(node);}
  LuapeNodeEquivalenceClass() {}

  void add(LuapeNodePtr node)
  {
    if (elements.insert(node).second)
    {
      sequences.push_back(LuapeRPNSequence::fromNode(node));
      if (!representent || isSmallerThan(node, representent))
        representent = node;
    }
  }

  size_t getNumElements() const
    {return sequences.size();}

  const LuapeRPNSequencePtr& getSequence(size_t index) const
    {jassert(index < sequences.size()); return sequences[index];}

  const LuapeNodePtr& getRepresentent() const
    {return representent;}

  virtual String toShortString() const
    {return T("[") + representent->toShortString() + T("]");}

protected:
  friend class LuapeNodeEquivalenceClassClass;

  std::set<LuapeNodePtr> elements;
  std::vector<LuapeRPNSequencePtr> sequences;
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
  void add(ExecutionContext& context, const std::vector<LuapeNodePtr>& nodes, LuapeNodeSearchProblemPtr problem, bool verbose = false, MCBanditPoolPtr pool = MCBanditPoolPtr())
  {
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      add(context, node, problem->makeBinaryKey(context, node), verbose, pool);
    }
    if (verbose)
    {
      context.informationCallback(String((int)getNumClasses()) + T(" equivalence classes"));
      context.informationCallback(String((int)getNumInvalids()) + T(" invalid candidates"));
    }
  }

  void add(ExecutionContext& context, LuapeNodePtr node, BinaryKeyPtr key, bool verbose = false, MCBanditPoolPtr pool = MCBanditPoolPtr())
  {
    if (key)
    {
      Map::const_iterator it = m.find(key);
      if (it == m.end())
      {
        LuapeNodeEquivalenceClassPtr equivalenceClass = new LuapeNodeEquivalenceClass(node);
        m[key] = equivalenceClass;
        if (pool)
          pool->createArm(equivalenceClass);

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
    {return invalids ? invalids->getNumElements() : 0;}

  void addClassesToBanditPool(MCBanditPoolPtr pool)
  {
    for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
      pool->createArm(it->second);
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

    LuapeNodeEquivalenceClassesPtr equivalenceClasses = new LuapeNodeEquivalenceClasses();
    MCBanditPoolPtr pool = new MCBanditPool(new ObjectiveWrapper(problem), explorationCoefficient, useMultiThreading); 

    LuapeRPNSequencePtr subSequence = new LuapeRPNSequence();
    for (size_t iteration = 0; iteration < numIterations; ++iteration)
    {
      context.enterScope(T("Iteration ") + String((int)iteration+1) + T(", Subsequence: ") + subSequence->toShortString());

      context.enterScope(T("Enumerating candidates"));
      std::vector<LuapeNodePtr> candidates;     
      problem->enumerateNodesExhaustively(context, complexity, candidates, true, subSequence);
      context.leaveScope(candidates.size());
      
      context.enterScope(T("Making equivalence classes"));
      equivalenceClasses->add(context, candidates, problem, true, pool);
      candidates.clear(); // free memory
      context.leaveScope(equivalenceClasses->getNumClasses());

      context.enterScope(T("Playing bandits"));
      pool->playIterations(context, numStepsPerIteration, equivalenceClasses->getNumClasses());

      LuapeRPNSequencePtr newSubSequence = new LuapeRPNSequence();
      for (size_t i = 0; i < subSequence->getLength(); ++i)
      {
        ObjectPtr bestSymbol = findBestSymbolCompletion(context, pool, newSubSequence);
        if (bestSymbol == subSequence->getElement(i))
          newSubSequence->append(bestSymbol);
        else
          break;
      }
      ObjectPtr bestSymbol = findBestSymbolCompletion(context, pool, newSubSequence);
      if (bestSymbol)
        newSubSequence->append(bestSymbol);
      else
        break;
      context.leaveScope();

      context.leaveScope();

      subSequence = newSubSequence;
    }
    return true;
  }

  ObjectPtr findBestSymbolCompletion(ExecutionContext& context, MCBanditPoolPtr pool, const LuapeRPNSequencePtr& subSequence)
  {
    std::vector< std::pair<size_t, double> > order;
    pool->getArmsOrder(order);

    std::set<ObjectPtr> candidates;
    
    for (size_t i = 0; i < order.size() && candidates.size() != 1; ++i)
    {
      LuapeNodeEquivalenceClassPtr equivalenceClass = pool->getArmParameter(order[i].first).getObjectAndCast<LuapeNodeEquivalenceClass>();
      std::set<ObjectPtr> candidateActions = getCandidateActions(equivalenceClass, subSequence);
      /*String info = equivalenceClass->toShortString() + T(" => ");
      for (std::set<ObjectPtr>::const_iterator it = candidateActions.begin(); it != candidateActions.end(); ++it)
        info += (*it)->toShortString() + T(" ");
      context.informationCallback(info);*/

      if (i == 0)
      {
        candidates = candidateActions;
        if (candidates.empty())
          return ObjectPtr();
      }
      else
      {
        std::set<ObjectPtr> remainingCandidates;
        std::set_intersection(candidateActions.begin(), candidateActions.end(), candidates.begin(), candidates.end(),
                                std::inserter(remainingCandidates, remainingCandidates.begin()));
        if (remainingCandidates.empty())
          break;
        candidates.swap(remainingCandidates);
      }
      
      /*info = String::empty;
      for (std::set<ObjectPtr>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
        info += (*it)->toShortString() + T(" ");
      context.informationCallback(info);*/
    }

    jassert(candidates.size());
    String info("Final candidates: ");
    for (std::set<ObjectPtr>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
      info += (*it)->toShortString() + T(" ");
    context.informationCallback(info);

    if (candidates.size() == 1)
      return *candidates.begin();

    size_t n = context.getRandomGenerator()->sampleSize(candidates.size());
    std::set<ObjectPtr>::const_iterator it = candidates.begin();
    for (size_t i = 0; i < n; ++i)
      ++it;
    return *it;
  }

  std::set<ObjectPtr> getCandidateActions(LuapeNodeEquivalenceClassPtr equivalenceClass, const LuapeRPNSequencePtr& subSequence)
  {
    size_t n = equivalenceClass->getNumElements();
    std::set<ObjectPtr> res;
    for (size_t i = 0; i < n; ++i)
    {
      LuapeRPNSequencePtr sequence = equivalenceClass->getSequence(i);
      if (sequence->startsWith(subSequence) && sequence->getLength() > subSequence->getLength())
        res.insert(sequence->getElement(subSequence->getLength()));
    }
    return res;
  }

protected:
  friend class LuapeFormulaDiscoverySandBoxClass;

  LuapeNodeSearchProblemPtr problem;
  size_t complexity;
  double explorationCoefficient;
  size_t numIterations;
  size_t numStepsPerIteration;
  bool useMultiThreading;

  struct ObjectiveWrapper : public MCBanditPoolObjective
  {
    ObjectiveWrapper(LuapeNodeSearchProblemPtr problem) : problem(problem) {}

    LuapeNodeSearchProblemPtr problem;

    virtual size_t getNumInstances() const
      {return problem->getNumInstances();}

    virtual void getObjectiveRange(double& worst, double& best) const
      {problem->getObjectiveRange(worst, best);}
 
    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      LuapeNodeEquivalenceClassPtr equivalenceClass = parameter.getObjectAndCast<LuapeNodeEquivalenceClass>();
      return problem->computeObjective(context, equivalenceClass->getRepresentent(), instanceIndex);
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_H_

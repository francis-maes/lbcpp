/*-----------------------------------------.---------------------------------.
| Filename: MetaMCSandBox.h                | Meta Monte Carlo SandBox        |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2012 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_
# define LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "MCAlgorithm.h"
# include "MCOptimizer.h"
# include "MCProblem.h"
# include "../../../src/Luape/Function/ObjectLuapeFunctions.h"
# include <fstream>

namespace lbcpp
{

/*
** Luape functions
*/
class MCAlgorithmLuapeFunction : public UnaryObjectLuapeFunction<MCAlgorithmLuapeFunction>
{
public:
  MCAlgorithmLuapeFunction() : UnaryObjectLuapeFunction<MCAlgorithmLuapeFunction>(mcAlgorithmClass) {}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return mcAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const = 0;

  Variable computeObject(const ObjectPtr& object) const
    {return createAlgorithm(object.staticCast<MCAlgorithm>());} 
};

class IterateMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
  IterateMCAlgorithmLuapeFunction() : numIterations(2) {}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return iterateMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new IterateMCAlgorithm(input, numIterations);}

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    jassert(index == 0);
    VectorPtr candidates = vector(positiveIntegerType, 5);
    candidates->setElement(0, Variable(2, positiveIntegerType));
    candidates->setElement(1, Variable(3, positiveIntegerType));
    candidates->setElement(2, Variable(5, positiveIntegerType));
    candidates->setElement(3, Variable(10, positiveIntegerType));
    candidates->setElement(4, Variable(100, positiveIntegerType));
    return candidates;
  }

protected:
  friend class IterateMCAlgorithmLuapeFunctionClass;

  size_t numIterations;
};

extern ClassPtr iterateMCAlgorithmLuapeFunctionClass;

class StepMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return stepByStepMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new StepByStepMCAlgorithm(input, true);}
};

class LookAheadMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return lookAheadMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new LookAheadMCAlgorithm(input);}
};

class SelectMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return selectMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new SelectMCAlgorithm(input, explorationCoefficient);}

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    jassert(index == 0);
    DenseDoubleVectorPtr candidates = new DenseDoubleVector(4, 0.0);
    candidates->setValue(0, -1.0);
    candidates->setValue(0, 0.0);
    candidates->setValue(0, 0.1);
    candidates->setValue(0, 1.0);
    return candidates;
  }

private:
  friend class SelectMCAlgorithmLuapeFunctionClass;
  double explorationCoefficient;
};

extern ClassPtr selectMCAlgorithmLuapeFunctionClass;

class MCAlgorithmsUniverse : public LuapeUniverse
{
public:
  size_t getNumIterations(const LuapeNodePtr& node) const
  {
    if (!node.isInstanceOf<LuapeFunctionNode>())
      return 0;
    const LuapeFunctionNodePtr& functionNode = node.staticCast<LuapeFunctionNode>();
    const LuapeFunctionPtr& function = functionNode->getFunction();
    return function.isInstanceOf<IterateMCAlgorithmLuapeFunction>() ? (size_t)function->getVariable(0).getInteger() : 0;
  }

  bool isSelect(const LuapeNodePtr& node) const
  {
    return node.isInstanceOf<LuapeFunctionNode>() && 
      node.staticCast<LuapeFunctionNode>()->getFunction().isInstanceOf<SelectMCAlgorithmLuapeFunction>();
  }

  virtual LuapeNodePtr canonizeNode(const LuapeNodePtr& node)
  {
    // select(pi1, select(pi2, X)) ==> select(pi1, X)
    if (isSelect(node) && isSelect(node->getSubNode(0)))
    {
      LuapeFunctionPtr function = node.staticCast<LuapeFunctionNode>()->getFunction();
      LuapeNodePtr argument = node->getSubNode(0)->getSubNode(0);
      return makeFunctionNode(function, argument);
    }

    // iterate(N_1, iterate(N_2, S)) ==> iterate(N_1 * N_2, S)
    size_t numIterations = getNumIterations(node);
    if (numIterations > 0)
    {
      size_t numIterations2 = getNumIterations(node->getSubNode(0));
      if (numIterations2 > 0)
      {
        LuapeFunctionPtr function = makeFunction(iterateMCAlgorithmLuapeFunctionClass, std::vector<Variable>(1, numIterations * numIterations2));
        LuapeNodePtr argument = node->getSubNode(0)->getSubNode(0);
        return makeFunctionNode(function, argument);
      }
    }
    return node;
  }
};

/////////////////////////////////

class CacheAndFiniteBudgetMCObjective : public MCObjective
{
public:
  CacheAndFiniteBudgetMCObjective(MCObjectivePtr objective, size_t budget, bool useCache)
    : objective(objective), budget(budget), useCache(useCache), numEvaluations(0), numCachedEvaluations(0), bestScoreSoFar(-DBL_MAX), nextCurvePoint(1) {}

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    ++numEvaluations;
    /*
    LuapeNodeBuilderStatePtr builder = finalState.staticCast<LuapeNodeBuilderState>();
    if (builder->getStackSize() != 1)
      return -DBL_MAX;
    LuapeNodePtr node = builder->getStackElement(0);
    if (useCache)
    {
      std::map<LuapeNodePtr, double>::iterator it = cache.find(node);
      if (it != cache.end())
      {
        ++numCachedEvaluations;
        return it->second;
      }
    }*/
    
    //context.informationCallback(finalState->toShortString());
    double res = objective->evaluate(context, finalState);
    bestScoreSoFar = juce::jmax(res, bestScoreSoFar);

    if (numEvaluations == nextCurvePoint)
    {
      curve.push_back(bestScoreSoFar);
      nextCurvePoint *= 2;
    }

//    if (useCache)
//      cache[node] = res;
    return res;
  }

  virtual bool shouldStop() const
    {return numEvaluations >= budget || numCachedEvaluations >= 100 * budget;}

  size_t getNumEvaluations() const
    {return numEvaluations;}

  size_t getNumCachedEvaluations() const
    {return numCachedEvaluations;}

  const std::vector<double>& getCurve() const
    {return curve;}

protected:
  MCObjectivePtr objective;
  size_t budget;
  bool useCache;
  size_t numEvaluations;
  size_t numCachedEvaluations;
//  std::map<LuapeNodePtr, double> cache;

  double bestScoreSoFar;
  size_t nextCurvePoint;
  std::vector<double> curve;
};

typedef ReferenceCountedObjectPtr<CacheAndFiniteBudgetMCObjective> CacheAndFiniteBudgetMCObjectivePtr;

/////////////

class MetaMCSandBox : public WorkUnit
{
public:
  MetaMCSandBox()
    : problem(new F8SymbolicRegressionMCProblem()), budget(1000),
      maxAlgorithmSize(6), explorationCoefficient(5.0)
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
    // create arms
    context.enterScope("Create arms");
    BanditPoolPtr pool = new BanditPool(new AlgorithmObjective(problem, budget), explorationCoefficient);
    generateMCAlgorithms(context, pool);
    context.leaveScope((int)pool->getNumArms());

    // play arms
    context.enterScope("Play arms");
    pool->playIterations(context, 100, pool->getNumArms());
    context.leaveScope();

    // display results
    context.enterScope("Arms");
    pool->displayAllArms(context);
    context.leaveScope();

    // evaluate baselines and discovered algorithms
    context.enterScope("Evaluation");
    evaluateBaselinesAndDiscoveredAlgorithms(context, pool);
    context.leaveScope();
    return true;
  }

protected:
  friend class MetaMCSandBoxClass;
  
  MCProblemPtr problem;
  size_t budget;
  size_t maxAlgorithmSize;
  double explorationCoefficient;

  void generateMCAlgorithms(ExecutionContext& context, BanditPoolPtr pool)
  {
    LuapeInferencePtr problem = new LuapeInference(new MCAlgorithmsUniverse());

    problem->addConstant(rollout());
    problem->addFunction(new IterateMCAlgorithmLuapeFunction());
    problem->addFunction(new StepMCAlgorithmLuapeFunction());
    problem->addFunction(new LookAheadMCAlgorithmLuapeFunction());
    problem->addFunction(new SelectMCAlgorithmLuapeFunction());
    problem->addTargetType(mcAlgorithmClass);

    std::vector<LuapeNodePtr> nodes;
    problem->enumerateNodesExhaustively(context, maxAlgorithmSize + 1, nodes, true);

    std::set<LuapeNodePtr> uniqueNodes;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      MCAlgorithmPtr algorithm = node->compute(context).getObjectAndCast<MCAlgorithm>();
      if (!algorithm.isInstanceOf<IterateMCAlgorithm>())
        uniqueNodes.insert(node);
    }

    pool->reserveArms(uniqueNodes.size());
    size_t count = 0;
    for (std::set<LuapeNodePtr>::const_iterator it = uniqueNodes.begin(); it != uniqueNodes.end(); ++it)
    {
      LuapeNodePtr node = *it;
      MCAlgorithmPtr algorithm = node->compute(context).getObjectAndCast<MCAlgorithm>();
      ++count;
      if (count < 20)
        context.informationCallback(algorithm->toShortString());
      else if (count == 20)
        context.informationCallback("...");

      pool->createArm(algorithm);
    }

    context.informationCallback("Num algorithms exhaustive: " + String((int)nodes.size()) + T(", pruned: ") + String((int)uniqueNodes.size()));
  }

  struct AlgorithmObjective : public BanditPoolObjective
  {
    AlgorithmObjective(MCProblemPtr problem, size_t budget)
      : problem(problem), budget(budget) {}

    virtual void getObjectiveRange(double& worst, double& best) const
      {problem->getObjectiveRange(worst, best);}

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      MCAlgorithmPtr algorithm = parameter.getObjectAndCast<MCAlgorithm>();
      algorithm->initialize(context);
      std::pair<DecisionProblemStatePtr, MCObjectivePtr> stateAndObjective = problem->getInstance(context, instanceIndex);
      CacheAndFiniteBudgetMCObjectivePtr objective = new CacheAndFiniteBudgetMCObjective(stateAndObjective.second, budget, false);

      DecisionProblemStatePtr finalState;
      return iterate(algorithm, 0)->search(context, objective, stateAndObjective.first, NULL, finalState);
    }
    
  private:
    MCProblemPtr problem;
    size_t budget;
  };

  void evaluateBaselinesAndDiscoveredAlgorithms(ExecutionContext& context, BanditPoolPtr pool)
  {
    // baselines
    std::vector< std::pair<String, MCAlgorithmPtr> > baselines;
    baselines.push_back(std::make_pair("RAND", rollout()));
    baselines.push_back(std::make_pair("MCTS", select(rollout())));
    baselines.push_back(std::make_pair("Step(MCTS)", step(iterate(select(rollout()), budget / 10))));

    baselines.push_back(std::make_pair("LA1", step(lookAhead(rollout()))));
    baselines.push_back(std::make_pair("LA2", step(lookAhead(lookAhead(rollout())))));
    baselines.push_back(std::make_pair("LA3", step(lookAhead(lookAhead(lookAhead(rollout()))))));
    baselines.push_back(std::make_pair("LA4", step(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))));
    baselines.push_back(std::make_pair("LA5", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout()))))))));
    baselines.push_back(std::make_pair("LA6", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))))));
    
    baselines.push_back(std::make_pair("NMC2", step(lookAhead(step(lookAhead(rollout()))))));
    baselines.push_back(std::make_pair("NMC3", step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))));
    baselines.push_back(std::make_pair("NMC4", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))));
    baselines.push_back(std::make_pair("NMC5", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))))));
    
    baselines.push_back(std::make_pair("(step2la2rollout)", step(step(lookAhead(lookAhead(rollout()))))));
    baselines.push_back(std::make_pair("(step3la2rollout", step(step(step(lookAhead(lookAhead(rollout())))))));
    
    //baselines.push_back(std::make_pair("LA7", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout()))))))))));
    //baselines.push_back(std::make_pair("LA8", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))))))));
    //baselines.push_back(std::make_pair("NMC6", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))))))));

    for (size_t i = 0; i < baselines.size(); ++i)
      evaluateAlgorithm(context, baselines[i].first, baselines[i].second);

    // discovered
    std::vector< std::pair<size_t, double> > armsOrder;
    pool->getArmsOrder(armsOrder);
    size_t numBests = 10;
    if (armsOrder.size() < numBests)
      numBests = armsOrder.size();
    for (size_t i = 0; i < numBests; ++i)
    {
      MCAlgorithmPtr algorithm = pool->getArmParameter(armsOrder[i].first).getObjectAndCast<MCAlgorithm>();
      evaluateAlgorithm(context, "Discovered " + String((int)i+1) + ": " + algorithm->toShortString(), algorithm);
    }
  }

  void evaluateAlgorithm(ExecutionContext& context, const String& name, const MCAlgorithmPtr& algorithm)
  {
    const size_t numEvaluationProblems = 100;

    ScalarVariableMean mean;
    context.enterScope(name);
    for (size_t i = 0; i < numEvaluationProblems; ++i)
    {
      algorithm->initialize(context);

      std::pair<DecisionProblemStatePtr, MCObjectivePtr> stateAndObjective = problem->getInstance(context, i);
      CacheAndFiniteBudgetMCObjectivePtr objective = new CacheAndFiniteBudgetMCObjective(stateAndObjective.second, budget, false);

      DecisionProblemStatePtr finalState;
      mean.push(iterate(algorithm, 0)->search(context, objective, stateAndObjective.first, NULL, finalState));
    }
    context.leaveScope(mean.getMean());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_

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
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return iterateMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new IterateMCAlgorithm(input, 4);}
};

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
    BanditPoolPtr pool = new BanditPool(new AlgorithmObjective(problem, budget), explorationCoefficient);
    generateMCAlgorithms(context, pool);
    context.informationCallback(String((int)pool->getNumArms()) + T(" Arms"));

    // play arms
    pool->playIterations(context, 100, pool->getNumArms());

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
    LuapeInferencePtr problem = new LuapeInference();

    problem->addConstant(rollout());
    problem->addFunction(new IterateMCAlgorithmLuapeFunction());
    problem->addFunction(new StepMCAlgorithmLuapeFunction());
    problem->addFunction(new LookAheadMCAlgorithmLuapeFunction());
    problem->addTargetType(mcAlgorithmClass);

    std::vector<LuapeNodePtr> nodes;
    problem->enumerateNodesExhaustively(context, maxAlgorithmSize + 1, nodes, true);

    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      MCAlgorithmPtr algorithm = node->compute(context).getObjectAndCast<MCAlgorithm>();
      if (!algorithm.isInstanceOf<IterateMCAlgorithm>())
        pool->createArm(algorithm);
    }
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

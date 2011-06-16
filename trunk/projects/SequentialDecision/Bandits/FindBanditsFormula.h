/*-----------------------------------------.---------------------------------.
| Filename: BanditsSandBox.h               | Bandits Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Sampler/Sampler.h>
# include "../WorkUnit/GPSandBox.h"
# include "DiscreteBanditExperiment.h"

namespace lbcpp
{

// GPExpression -> GPExpression
class MakeGPExpressionUnique : public SimpleUnaryFunction
{
public:
  MakeGPExpressionUnique() : SimpleUnaryFunction(gpExpressionClass, gpExpressionClass, T("unique")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExpressionPtr& expression = input.getObjectAndCast<GPExpression>();
    GPExpressionPtr rewritedExpression = rewrite(expression);
    //context.informationCallback(expression->toShortString() + T(" ==> ") + rewritedExpression->toShortString());
    return rewritedExpression;
  }

  GPExpressionPtr rewriteBinaryExpression(GPOperator op, const GPExpressionPtr& left, const GPExpressionPtr& right) const
  {
    ConstantGPExpressionPtr constantLeft = left.dynamicCast<ConstantGPExpression>();
    ConstantGPExpressionPtr constantRight = right.dynamicCast<ConstantGPExpression>();

    if (constantLeft && constantRight)
      return new ConstantGPExpression(BinaryGPExpression(left, op, right).compute(NULL), constantLeft->isLearnable() || constantRight->isLearnable());

    ConstantGPExpressionPtr learnableLeft;
    if (constantLeft && constantLeft->isLearnable())
    {
      learnableLeft = constantLeft;
      constantLeft = ConstantGPExpressionPtr();
    }
    ConstantGPExpressionPtr learnableRight;
    if (constantRight && constantRight->isLearnable())
    {
      learnableRight = constantRight;
      constantRight = ConstantGPExpressionPtr();
    }
    UnaryGPExpressionPtr unaryLeft = left.dynamicCast<UnaryGPExpression>();
    UnaryGPExpressionPtr unaryRight = right.dynamicCast<UnaryGPExpression>();

    if (op == gpDivision)
    {
      // x / 1 = x
      if (constantRight && constantRight->getValue() == 1.0)
        return left;

      // 1 / x = inverse(x)
      if (constantLeft && constantLeft->getValue() == 1.0)
        return right;

      // x / x = 1
      if (Variable(left).compare(right) == 0)
        return new ConstantGPExpression(1.0);

      // x / inverse(y) = x * y
      if (unaryRight && unaryRight->getOperator() == gpInverse)
        return rewrite(new BinaryGPExpression(left, gpMultiplication, unaryRight->getExpression()));

    }
    else if (op == gpMultiplication)
    {
      // 1 * x = x
      if (constantLeft && constantLeft->getValue() == 1.0)
        return right;

      // x * 1 = x
      if (constantRight && constantRight->getValue() == 1.0)
        return left;

      // inverse(x) * y = y / x
      if (unaryLeft && unaryLeft->getOperator() == gpInverse)
        return rewrite(new BinaryGPExpression(right, gpDivision, unaryLeft->getExpression()));

      // x * inverse(y) = x / y
      if (unaryRight && unaryRight->getOperator() == gpInverse)
        return rewrite(new BinaryGPExpression(left, gpDivision, unaryRight->getExpression()));
    }
    else if (op == gpSubtraction)
    {
      // x - x = 0
      if (Variable(left).compare(right) == 0)
        return new ConstantGPExpression(0.0);
    }

    if (op == gpAddition || op == gpMultiplication)
    {

      // x + y and x * y commutativity
      if (Variable(left).compare(right) > 0)
        return rewrite(new BinaryGPExpression(right, op, left));
    }
    
    return new BinaryGPExpression(left, op, right);
  }

  GPExpressionPtr rewriteUnaryExpression(GPPre op, GPExpressionPtr expression) const
  {
    // precompute constants
    ConstantGPExpressionPtr constant = expression.dynamicCast<ConstantGPExpression>();
    if (constant)
      return new ConstantGPExpression(UnaryGPExpression(op, expression).compute(NULL), constant->isLearnable());

    // inverse(inverse(x)) = x
    if (op == gpInverse)
    {
      UnaryGPExpressionPtr unary = expression.dynamicCast<UnaryGPExpression>();
      if (unary && unary->getOperator() == gpInverse)
        return unary->getExpression();
    }

    return new UnaryGPExpression(op, expression);
  }

  GPExpressionPtr rewrite(const GPExpressionPtr& expression) const
  {
    BinaryGPExpressionPtr binaryExpression = expression.dynamicCast<BinaryGPExpression>();
    if (binaryExpression)
    {
      GPExpressionPtr left = rewrite(binaryExpression->getLeft());
      GPExpressionPtr right = rewrite(binaryExpression->getRight());
      GPOperator op = binaryExpression->getOperator();
      return rewriteBinaryExpression(op, left, right);
    }

    UnaryGPExpressionPtr unaryExpression = expression.dynamicCast<UnaryGPExpression>();
    if (unaryExpression)
    {
      GPExpressionPtr expr = rewrite(unaryExpression->getExpression());
      return rewriteUnaryExpression(unaryExpression->getOperator(), expr);
    }
    return expression;
  }
};


class UnaryCacheFunction : public Function
{
public:
  UnaryCacheFunction(FunctionPtr function = FunctionPtr())
    : function(function), cacheRequests(0), cacheAccesses(0)  {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return function->getRequiredInputType(index, numInputs);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {function->initialize(context, inputVariables); return function->getOutputType();}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    Variable res;
    if (!const_cast<UnaryCacheFunction* >(this)->getResultInCache(context, input, res))
    {
      res = function->compute(context, input);
      const_cast<UnaryCacheFunction* >(this)->addResultInCache(context, input, res);
    }
    return res;
  }

  void displayAndFlushCacheUsage(ExecutionContext& context)
  {
    context.informationCallback(T("Cache usage: ") + String(cacheAccesses) + T(" / ") + String(cacheRequests) +
      T(" (") + Variable(cacheAccesses / (double)cacheRequests, probabilityType).toShortString() + T(")"));
    cacheRequests = cacheAccesses = 0;
  }

protected:
  friend class UnaryCacheFunctionClass;

  FunctionPtr function;

  ReadWriteLock cacheLock;
  std::map<Variable, Variable> cache;

  int cacheRequests;
  int cacheAccesses;

  bool getResultInCache(ExecutionContext& context, const Variable& input, Variable& output)
  {
    ScopedReadLock _(cacheLock);
    juce::atomicIncrement(cacheRequests);
    std::map<Variable, Variable>::const_iterator it = cache.find(input);
    if (it == cache.end())
      return false;
    juce::atomicIncrement(cacheAccesses);
    output = it->second;
    return true;
  }

  void addResultInCache(ExecutionContext& context, const Variable& input, const Variable& output)
  {
    //context.informationCallback(T("cache ") + input.toShortString() + T(" (") + output.toShortString() + T(")"));
    ScopedWriteLock _(cacheLock); cache[input.clone(context)] = output;
  }
};

typedef ReferenceCountedObjectPtr<UnaryCacheFunction> UnaryCacheFunctionPtr;

class SingleParameterIndexBasedDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  SingleParameterIndexBasedDiscreteBanditPolicy(double C = 1.0)
    : C(C) {}

  virtual double getParameterInitialGuess() const
    {return 1.0;}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(getParameterInitialGuess(), 1.0);}

  virtual void setParameters(const Variable& parameters)
    {C = parameters.toDouble();}

  virtual Variable getParameters() const
    {return C;}

protected:
  friend class SingleParameterIndexBasedDiscreteBanditPolicyClass;

  double C;
};


class Formula1IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 0.5;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return juce::jmax(statistics->getRewardMean(), C);
  }
};

class Formula2IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tk = (double)statistics->getPlayedCount();
    double rk = statistics->getRewardMean();
    return tk * (rk - C);
  }
};


class Formula3IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double rk = statistics->getRewardMean();
    return rk * (rk - C);
  }
};

class Formula4IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double sk = statistics->getRewardStandardDeviation();
    return sk * (sk - C);
  }
};

class Formula5IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  { 
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + C / (double)statistics->getPlayedCount();
  }
};

class Formula6IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tk = (double)statistics->getPlayedCount();
    double rk = statistics->getRewardMean();
    double sk = statistics->getRewardStandardDeviation();
    return tk * (rk - C * sk);
  }
};

class Formula7IndexBasedDiscreteBanditPolicy : public SingleParameterIndexBasedDiscreteBanditPolicy
{
public:
  virtual double getParameterInitialGuess() const
    {return 1.0;}
    
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tk = (double)statistics->getPlayedCount();
    double rk = statistics->getRewardMean();
    double sk = statistics->getRewardStandardDeviation();
    return tk * (rk * rk - C * sk);
  }
};

/*
** FindBanditsFormula
*/
class FindBanditsFormula : public WorkUnit
{
public:
  FindBanditsFormula() : problemIndex(0), maxTimeStep(100), numRunsPerEstimation(100) {}
 
  enum {numProblems = 12};
 
  Variable testPolicyOnEachProblem(ExecutionContext& context, const DiscreteBanditPolicyPtr& policy)
  {
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policy"), numProblems);
    for (size_t i = 0; i < numProblems; ++i)
    {
      DiscreteBanditStatePtr problem = createBanditProblem(i);
      String problemName = problem->toShortString();
      std::vector<DiscreteBanditStatePtr> problemSingleton(1, problem);
      workUnit->setWorkUnit(i, makeEvaluationWorkUnit(problemSingleton, problemName, policy->cloneAndCast<DiscreteBanditPolicy>(), true));
    }
    workUnit->setProgressionUnit(T("Problems"));
    workUnit->setPushChildrenIntoStackFlag(true);
    return context.run(workUnit);
  }
  
  Variable tuneAndTestPolicyOnEachProblem(ExecutionContext& context, const DiscreteBanditPolicyPtr& policy)
  {
    for (size_t i = 0; i < numProblems; ++i)
    {
      DiscreteBanditStatePtr problem = createBanditProblem(i);
      String problemName = problem->toShortString();

      DiscreteBanditPolicyPtr policyToOptimize = policy->cloneAndCast<DiscreteBanditPolicy>();
      context.enterScope(T("Problem ") + String((int)i + 1));
      double bestScore;
      Variable bestParameters = optimizePolicy(context, policyToOptimize, std::vector<DiscreteBanditStatePtr>(1, problem), bestScore);
      DiscreteBanditPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policyToOptimize, bestParameters);
      
      double regret = evaluatePolicy(context, problem, problemName, optimizedPolicy);
      context.leaveScope(regret);
    }
    return true;
  }

  bool makePolicyParameterCurves(ExecutionContext& context, const std::vector<DiscreteBanditStatePtr>& problems, const String& problemsName)
  {
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(ucb1DiscreteBanditPolicy());
    policies.push_back(new Formula1IndexBasedDiscreteBanditPolicy());
    policies.push_back(new Formula2IndexBasedDiscreteBanditPolicy());
    policies.push_back(new Formula3IndexBasedDiscreteBanditPolicy());
    policies.push_back(new Formula4IndexBasedDiscreteBanditPolicy());  
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy());  
    policies.push_back(new Formula6IndexBasedDiscreteBanditPolicy());  
    policies.push_back(new Formula7IndexBasedDiscreteBanditPolicy());  

   // for (size_t problemIndex = 0; problemIndex < numProblems; ++problemIndex)
    {
//      DiscreteBanditStatePtr problem = createBanditProblem(problemIndex);
//      String problemName = problem->toShortString();
//      context.enterScope(T("Problem ") + String((int)problemIndex) + T(": ") + problemName);

      context.enterScope(problemsName);
      for (double C = 0.0; C <= 5.0; C += 0.05)
      {
        context.enterScope(T("C = ") + String(C));
        context.resultCallback(T("C"), C);
        for (size_t i = 0; i < policies.size(); ++i)
        {
          double regret = evaluatePolicy(context, problems, problemsName, Parameterized::cloneWithNewParameters(policies[i], C));
          context.resultCallback(policies[i]->getClass()->getShortName(), regret);
        }
        context.leaveScope(true);
      }
      context.leaveScope(true);
    }
    return true;
  }
  
  double evaluatePolicy(ExecutionContext& context, DiscreteBanditStatePtr problem, const String& problemName, const DiscreteBanditPolicyPtr& policy)
  {
    std::vector<DiscreteBanditStatePtr> problemSingleton(1, problem);
    DenseDoubleVectorPtr regrets = context.run(makeEvaluationWorkUnit(problemSingleton, problemName, policy->cloneAndCast<DiscreteBanditPolicy>(), true)).getObjectAndCast<DenseDoubleVector>();
    return regrets->getValue(regrets->getNumValues() - 1);
  }
 
  
  double evaluatePolicy(ExecutionContext& context, const std::vector<DiscreteBanditStatePtr>& problems, const String& problemsName, const DiscreteBanditPolicyPtr& policy)
  {
    DenseDoubleVectorPtr regrets = context.run(makeEvaluationWorkUnit(problems, problemsName, policy->cloneAndCast<DiscreteBanditPolicy>(), true)).getObjectAndCast<DenseDoubleVector>();
    return regrets->getValue(regrets->getNumValues() - 1);
  }

  virtual Variable run(ExecutionContext& context)
  {
//    return tuneAndTestPolicyOnEachProblem(context, new TestDiscreteBanditPolicy());
  
//    DiscreteBanditStatePtr problem = createBanditProblem(problemIndex);
//    String problemName = problem->toShortString();
    std::vector<DiscreteBanditStatePtr> problems(12);
    for (size_t i = 0; i < problems.size(); ++i)
      problems[i] = createBanditProblem(i);
    String problemsName = T("Problems 1--12");
    
    makePolicyParameterCurves(context, problems, problemsName);
    
    /*
    ** Evaluate un-parametrized baseline policies
    */
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    policies.push_back(ucb1NormalDiscreteBanditPolicy());
    policies.push_back(ucb2DiscreteBanditPolicy(0.001));
    policies.push_back(greedyDiscreteBanditPolicy());
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
      workUnit->setWorkUnit(i, makeEvaluationWorkUnit(problems, problemsName, policies[i], true));
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);
    
    /*
    ** Tune and evaluate baseline policies
    */
    std::vector<std::pair<DiscreteBanditPolicyPtr, String> > policiesToOptimize;
    policiesToOptimize.push_back(std::make_pair(ucb1DiscreteBanditPolicy(), T("UCB1")));
    policiesToOptimize.push_back(std::make_pair(ucbvDiscreteBanditPolicy(), T("UCBv")));
    policiesToOptimize.push_back(std::make_pair(epsilonGreedyDiscreteBanditPolicy(0.1, 0.1), T("epsilonGreedy")));
    policiesToOptimize.push_back(std::make_pair(powerDiscreteBanditPolicy(1, false), T("powerFunction1")));
//    policiesToOptimize.push_back(std::make_pair(powerDiscreteBanditPolicy(2, false), T("powerFunction2")));
    for (size_t i = 0; i < policiesToOptimize.size(); ++i)
    {
      DiscreteBanditPolicyPtr policyToOptimize = policiesToOptimize[i].first;
      context.enterScope(T("Optimizing ") + policiesToOptimize[i].second);
      double bestScore;
      Variable bestParameters = optimizePolicy(context, policyToOptimize, problems, bestScore);
      DiscreteBanditPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policyToOptimize, bestParameters);
      DenseDoubleVectorPtr regrets = context.run(makeEvaluationWorkUnit(problems, problemsName, optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true)).getObjectAndCast<DenseDoubleVector>();
      context.leaveScope(regrets->getValue(regrets->getNumValues() - 1));
    }

    /*
    ** Find formula
    */
    ultimatePolicySearch(context, problems);
    return true;
  }

protected:
  friend class FindBanditsFormulaClass;

  size_t problemIndex;
  size_t maxTimeStep;
  size_t numRunsPerEstimation;

  WorkUnitPtr makeEvaluationWorkUnit(const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose) const
    {return new EvaluateDiscreteBanditPolicyWorkUnit(initialStates[0]->getNumBandits(), maxTimeStep, initialStates, initialStatesDescription, policy, numRunsPerEstimation, verbose);}
  
  Variable optimizePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& trainingProblems, double& bestScore)
  {
    TypePtr parametersType = Parameterized::getParametersType(policy);
    jassert(parametersType);
    size_t numParameters = 0;
    EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(parametersType);
    if (enumeration)
      numParameters = enumeration->getNumElements();
    else if (parametersType->inheritsFrom(doubleType))
      numParameters = 1;
    else if (parametersType->inheritsFrom(pairClass(doubleType, doubleType)))
      numParameters = 2;
    else if (parametersType->inheritsFrom(gpExpressionClass))
      numParameters = 100;
    jassert(numParameters);
    context.resultCallback(T("numParameters"), numParameters);

    //context.resultCallback(T("parametersType"), parametersType);

    // eda parameters
    size_t numIterations = 100;
    size_t populationSize = numParameters * 12;
    size_t numBests = numParameters * 3;
    StoppingCriterionPtr stoppingCriterion = maxIterationsWithoutImprovementStoppingCriterion(5);


    // optimizer state
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(Parameterized::get(policy)->createParametersSampler());

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateDiscreteBanditPolicyParameters(policy, trainingProblems[0]->getNumBandits(), maxTimeStep, trainingProblems, numRunsPerEstimation, 51861664);
    objectiveFunction->initialize(context, parametersType);

    FunctionPtr validationFunction = new EvaluateDiscreteBanditPolicyParameters(policy, trainingProblems[0]->getNumBandits(), maxTimeStep, trainingProblems, numRunsPerEstimation);
    validationFunction->initialize(context, parametersType);
    
    OptimizerContextPtr optimizerContext = synchroneousOptimizerContext(context, objectiveFunction, validationFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, stoppingCriterion, 0.0, false);
    //OptimizerPtr optimizer = asyncEDAOptimizer(numIterations*populationSize, populationSize, populationSize/numBests, 1, StoppingCriterionPtr(), 10, populationSize);

    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    Variable bestParameters = optimizerState->getBestVariable();
    context.resultCallback(T("optimizedPolicy"), Parameterized::cloneWithNewParameters(policy, bestParameters));
    bestScore = optimizerState->getBestScore();
    return bestParameters;
  }

  bool ultimatePolicySearch(ExecutionContext& context, const std::vector<DiscreteBanditStatePtr>& trainingStates)
  {
    FunctionPtr makeGPExpressionUniqueFunction = new MakeGPExpressionUnique();
  
    size_t numBandits = trainingStates[0]->getNumBandits();
    FunctionPtr objective = new EvaluateDiscreteBanditPolicyParameters(
      gpExpressionDiscreteBanditPolicy(), numBandits, maxTimeStep, trainingStates, numRunsPerEstimation, 51861664);
    objective = new GPStructureObjectiveFunction(objective);

    UnaryCacheFunctionPtr cacheFunction = new UnaryCacheFunction(objective);
    objective = composeFunction(makeGPExpressionUniqueFunction, cacheFunction);

    if (!objective->initialize(context, gpExpressionClass))
      return false;
      
    FunctionPtr validation = new EvaluateDiscreteBanditPolicyParameters(
      gpExpressionDiscreteBanditPolicy(), numBandits, maxTimeStep, trainingStates, numRunsPerEstimation);
    if (!validation->initialize(context, gpExpressionClass))
      return false;
   
/*
    for (size_t maxSearchNodes = 2; maxSearchNodes <= 2048; maxSearchNodes *= 2)
    {
      context.enterScope(T("maxSearchNodes = ") + String((int)maxSearchNodes));
      breadthFirstSearch(context, gpExpressionDiscreteBanditPolicyVariablesEnumeration, objective, validation, maxSearchNodes);
      context.leaveScope();
    }
    return true;
*/
    for (size_t maxDepth = 1; maxDepth <= 10; ++maxDepth)
    {
      //searchBestFormula(context, cacheFunction, objective, validation, maxDepth, true);
      searchBestFormula(context, cacheFunction, makeGPExpressionUniqueFunction, objective, validation, maxDepth, false);
    }
    return true;
    //return breadthFirstSearch(context, ultimatePolicyVariablesEnumeration, objective, validation);
  }

  bool searchBestFormula(ExecutionContext& context, const UnaryCacheFunctionPtr& cacheFunction, const FunctionPtr& makeGPExpressionUniqueFunction, const FunctionPtr& objective, const FunctionPtr& validation, size_t maxDepth, bool useCompactStateSpace)
  {
    context.enterScope(T("maxDepth = ") + String((int)maxDepth) + T(" ") + (useCompactStateSpace ? T("compact") : T("large")));

    DecisionProblemStatePtr state;
    if (useCompactStateSpace)
      state = new CompactGPExpressionBuilderState(T("toto"), gpExpressionDiscreteBanditPolicyVariablesEnumeration, objective);
    else
      state = new LargeGPExpressionBuilderState(T("toto"), gpExpressionDiscreteBanditPolicyVariablesEnumeration, objective);

    double allTimesBestScore = DBL_MAX;
    double allTimesBestValidation = DBL_MAX;
    GPExpressionPtr allTimesBestExpression;

    for (size_t timeStep = 0; timeStep < 5; ++timeStep)
    {
      context.enterScope(T("TimeStep ") + String((int)timeStep));
      double bestScore = DBL_MAX;
      size_t bestDepth = maxDepth + 1;
      GPExpressionPtr bestExpression;
      Variable bestFirstAction;
      Variable currentFirstAction;
      recursiveExhaustiveSearch(context, state, 0, maxDepth, bestScore, bestDepth, bestExpression, bestFirstAction, currentFirstAction);
      double bestValidation = validation->compute(context, bestExpression).toDouble();

      context.resultCallback(T("timeStep"), timeStep);
      context.resultCallback(T("bestScore"), bestScore);
      context.resultCallback(T("bestDepth"), bestDepth);
      context.resultCallback(T("bestValidation"), bestValidation);
      context.resultCallback(T("bestExpression"), makeGPExpressionUniqueFunction->compute(context, bestExpression));
      context.resultCallback(T("bestFirstAction"), bestFirstAction);
      context.informationCallback(T("Best: ") + bestExpression->toShortString() + T(" -> ") + String(bestScore) + T(", ") + String(bestValidation));
      context.informationCallback(T("Best First Action: ") + bestFirstAction.toShortString());
      cacheFunction->displayAndFlushCacheUsage(context);

      bool hasImproved = bestScore < allTimesBestScore;
      if (hasImproved)
        allTimesBestScore = bestScore, allTimesBestExpression = bestExpression, allTimesBestValidation = bestValidation;        

      double reward;
      state->performTransition(context, bestFirstAction, reward);

      context.leaveScope(bestScore);
      if (state->isFinalState())
      {
        context.informationCallback(T("Reached Final State"));
        break;
      }
      if (!hasImproved)
      {
        context.informationCallback(T("No improvment  "));
        break;
      }
    }
    context.informationCallback(T("Best: ") + allTimesBestExpression->toShortString() + T(" -> ") + String(allTimesBestScore) + T(", ") + String(allTimesBestValidation));

    context.resultCallback(T("bestScore"), allTimesBestScore);
    context.resultCallback(T("bestValidation"), allTimesBestValidation);    
    context.resultCallback(T("bestExpression"), makeGPExpressionUniqueFunction->compute(context, allTimesBestExpression));
    context.leaveScope(new Pair(allTimesBestScore, allTimesBestValidation));
    return true;
  }

  void recursiveExhaustiveSearch(ExecutionContext& context, GPExpressionBuilderStatePtr state, 
                  size_t depth, size_t maxDepth, double& bestScore, size_t& bestDepth, GPExpressionPtr& bestExpression, Variable& bestFirstAction, Variable& currentFirstAction)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = actions->getElement(i);
      if (depth == 0)
        currentFirstAction = action;

      double reward;
      //context.enterScope(action.toShortString());
      state->performTransition(context, action, reward);

      GPExpressionPtr expression = state->getOptimizedExpression();
      if (expression)
      {
        double score = state->getScore();
        
        //context.informationCallback(expression->toShortString() + T(" -> ") + String(score));

        if (score < bestScore || (score == bestScore && depth < bestDepth))
        {
          bestScore = score;
          bestDepth = depth;
          bestExpression = expression->cloneAndCast<GPExpression>();
          //context.informationCallback(bestExpression->toShortString() + T(" -> ") + String(bestScore));
          bestFirstAction = currentFirstAction;
        }
      }

      if (!state->isFinalState() && depth < maxDepth - 1)
        recursiveExhaustiveSearch(context, state, depth + 1, maxDepth, bestScore, bestDepth, bestExpression, bestFirstAction, currentFirstAction);

      state->undoTransition(context, action);
      //context.leaveScope(true);
    }
  }

  DiscreteBanditStatePtr createBanditProblem(size_t index) const
  {
    size_t i3 = index % 3;
    double p1 = 0.0, p2 = 0.0;
    if (i3 == 0)
      p1 = 0.9, p2 = 0.6;
    else if (i3 == 1)
      p1 = 0.9, p2 = 0.8;
    else if (i3 == 2)
      p1 = 0.55, p2 = 0.45;
      
    String str = String(p1) + T(", ") + String(p2);
    if (index < 3)
    {
      std::vector<SamplerPtr> samplers(2);
      samplers[0] = bernoulliSampler(p1);
      samplers[1] = bernoulliSampler(p2);
      DiscreteBanditStatePtr res = new DiscreteBanditState(samplers);
      res->setName(T("bernoulli(") + str + T(")"));
      return res;
    }
    else if (index < 6)
    {
      std::vector<SamplerPtr> samplers(10);
      samplers[0] = bernoulliSampler(p1);
      for (size_t i = 1; i < samplers.size(); ++i)
        samplers[i] = bernoulliSampler(p2);
      DiscreteBanditStatePtr res = new DiscreteBanditState(samplers);
      res->setName(T("bernoulli10(") + str + T(")"));
      return res;
    }
    else if (index < 9)
    {
      std::vector<SamplerPtr> samplers(2);
      samplers[0] = gaussianSampler(p1, 0.5);
      samplers[1] = gaussianSampler(p2, 0.5);
      DiscreteBanditStatePtr res = new DiscreteBanditState(samplers);
      res->setName(T("gaussian(") + str + T(")"));
      return res;
    }
    else
    {
      std::vector<SamplerPtr> samplers(2);
      samplers[0] = rejectionSampler(gaussianSampler(p1, 0.5), logicalAnd(lessThanOrEqualToPredicate(1.0), greaterThanOrEqualToPredicate(0.0)));
      samplers[1] = rejectionSampler(gaussianSampler(p2, 0.5), logicalAnd(lessThanOrEqualToPredicate(1.0), greaterThanOrEqualToPredicate(0.0)));
      DiscreteBanditStatePtr res = new DiscreteBanditState(samplers);
      res->setName(T("clampedGaussian(") + str + T(")"));
      return res;
    }
  }
   
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

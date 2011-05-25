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
# include "GPSandBox.h"

# include "../Bandits/DiscreteBanditExperiment.h"

namespace lbcpp
{
 

/*
** Evaluation Work Unit
*/
class EvaluateDiscreteBanditPolicy : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicy() : numBandits(2), numProblems(1000) {}

  struct EvaluatePolicyOnProblemsWorkUnit : public WorkUnit
  {
  public:
    EvaluatePolicyOnProblemsWorkUnit(const String& description, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& states,
                     const DiscreteBanditPolicyPtr& baselinePolicy, const DiscreteBanditPolicyPtr& optimizedPolicy)
     : description(description), numBandits(numBandits), maxTimeStep(maxTimeStep), states(states), baselinePolicy(baselinePolicy), optimizedPolicy(optimizedPolicy) {}

    virtual String toShortString() const
      {return T("Evaluate ") + description;}

    virtual Variable run(ExecutionContext& context)
    {
      context.resultCallback(T("baselinePolicy"), baselinePolicy);
      context.resultCallback(T("optimizedPolicy"), optimizedPolicy);

      // global evaluation
      context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, states, T("Global evaluation"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, true));

      // detailed evaluation
      context.enterScope(T("Detailed evaluation"));
      double baselineScoreSum = 0.0;
      double optimizedScoreSum = 0.0;
      size_t numberOptimizedOutperforms = 0; 
      for (size_t i = 0; i < states.size(); ++i)
      {
        context.enterScope(T("Problem ") + String((int)i));
        DiscreteBanditStatePtr initialState = states[i];
        context.resultCallback(T("index"), i);
        context.resultCallback(T("problem"), initialState);
        std::vector<DiscreteBanditStatePtr> initialStates(1, initialState);

        double baselineScore = context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Baseline Policy"), baselinePolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, false)).toDouble();
        baselineScoreSum += baselineScore;
        double optimizedScore = context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Optimized Policy"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, false)).toDouble();
        optimizedScoreSum += optimizedScore;

        context.resultCallback(T("baseline"), baselineScore);
        context.resultCallback(T("optimized"), optimizedScore);
        context.resultCallback(T("delta"), optimizedScore - baselineScore);
        if (optimizedScore < baselineScore)
          ++numberOptimizedOutperforms;
        context.leaveScope(new Pair(baselineScore, optimizedScore));
        context.progressCallback(new ProgressionState(i + 1, states.size(), T("Problems")));
      }

      Variable outperformPercentage(numberOptimizedOutperforms / (double)states.size(), probabilityType);
      double baselineScore = baselineScoreSum / states.size();
      double optimizedScore = optimizedScoreSum / states.size();
      PairPtr result = new Pair(new Pair(baselineScore, optimizedScore), outperformPercentage);
      context.leaveScope(result);

      context.resultCallback(T("deltaRegret"), optimizedScore - baselineScore);
      context.resultCallback(T("outperformPercentage"), outperformPercentage);
      return new Pair(optimizedScore, outperformPercentage);
    }

  protected:
    String description;
    size_t numBandits;
    size_t maxTimeStep;
    std::vector<DiscreteBanditStatePtr> states;
    DiscreteBanditPolicyPtr baselinePolicy;
    DiscreteBanditPolicyPtr optimizedPolicy;
  };

  struct EvaluatePolicyWorkUnit : public CompositeWorkUnit
  {
  public:
    EvaluatePolicyWorkUnit(const String& policyFilePath, size_t numBandits, const std::vector<DiscreteBanditStatePtr>& states, const std::vector<DiscreteBanditStatePtr>& states2,
                     const DiscreteBanditPolicyPtr& baselinePolicy, const DiscreteBanditPolicyPtr& optimizedPolicy)
      : CompositeWorkUnit(T("Evaluating policy ") + policyFilePath)
    {
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=10"), numBandits, 10, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=10"), numBandits, 10, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=100"), numBandits, 100, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=100"), numBandits, 100, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=1000"), numBandits, 1000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=1000"), numBandits, 1000, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=10000"), numBandits, 10000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=10000"), numBandits, 10000, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=100000"), numBandits, 100000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=100000"), numBandits, 100000, states2, baselinePolicy, optimizedPolicy));

      setProgressionUnit(T("Evaluations"));
      setPushChildrenIntoStackFlag(true);
    }
  };


  virtual Variable run(ExecutionContext& context)
  {
    if (!policyFile.exists())
    {
      context.errorCallback(policyFile.getFullPathName() + T(" does not exists"));
      return Variable();
    }

    /*
    ** Create problems
    */
    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    ClassPtr gaussianSamplerClass = lbcpp::getType(T("GaussianSampler"));
    ClassPtr rejectionSamplerClass = lbcpp::getType(T("RejectionSampler"));

    SamplerPtr banditSamplerSampler = objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0));
    SamplerPtr initialStateSampler = new DiscreteBanditInitialStateSampler(banditSamplerSampler, numBandits);
    SamplerPtr banditSamplerSampler2 = objectCompositeSampler(rejectionSamplerClass,
                                                objectCompositeSampler(gaussianSamplerClass, uniformScalarSampler(), uniformScalarSampler()),
                                                constantSampler(logicalAnd(lessThanOrEqualToPredicate(1.0), greaterThanOrEqualToPredicate(0.0))));
    SamplerPtr initialStateSampler2 = new DiscreteBanditInitialStateSampler(banditSamplerSampler2, numBandits);

    RandomGeneratorPtr random = new RandomGenerator();
    std::vector<DiscreteBanditStatePtr> states(numProblems);
    for (size_t i = 0; i < numProblems; ++i)
      states[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    std::vector<DiscreteBanditStatePtr> states2(numProblems);
    for (size_t i = 0; i < numProblems; ++i)
      states2[i] = initialStateSampler2->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    /*
    ** Create baseline policy
    */
    DiscreteBanditPolicyPtr baselinePolicy = ucb1TunedDiscreteBanditPolicy();

    /*
    ** Find policies to evaluate
    */
    juce::OwnedArray<File> policyFiles;
    if (policyFile.isDirectory())
      policyFile.findChildFiles(policyFiles, File::findFiles, true, T("*.policy"));
    else
      policyFiles.add(new File(policyFile));

    /*
    ** Evaluate policies
    */
    String description = T("Evaluating ");
    if (policyFiles.size() == 1)
      description += T("one policy");
    else
      description += String(policyFiles.size()) + T(" policies");
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(description));
    for (int i = 0; i < policyFiles.size(); ++i)
    {
      File policyFile = *policyFiles[i];
      String policyFilePath = context.getFilePath(policyFile);

      DiscreteBanditPolicyPtr policy = DiscreteBanditPolicy::createFromFile(context, policyFile);
      if (policy)
        workUnit->addWorkUnit(WorkUnitPtr(new EvaluatePolicyWorkUnit(policyFilePath, numBandits, states, states2, baselinePolicy, policy)));
    }
    workUnit->setPushChildrenIntoStackFlag(true);
    workUnit->setProgressionUnit(T("Policies"));
    return context.run(workUnit);
  }


private:
  friend class EvaluateDiscreteBanditPolicyClass;

  File policyFile;
  size_t numBandits;
  size_t numProblems;
};

// GPExpression -> GPExpression
class MakeGPExpressionUnique : public SimpleUnaryFunction
{
public:
  MakeGPExpressionUnique() : SimpleUnaryFunction(gpExpressionClass, gpExpressionClass, T("unique")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExpressionPtr& expression = input.getObjectAndCast<GPExpression>();
    return rewrite(expression);
  }

  GPExpressionPtr rewrite(const GPExpressionPtr& expression) const
  {
    BinaryGPExpressionPtr binaryExpression = expression.dynamicCast<BinaryGPExpression>();
    if (binaryExpression)
    {
      GPExpressionPtr left = rewrite(binaryExpression->getLeft());
      GPExpressionPtr right = rewrite(binaryExpression->getRight());
      GPOperator op = binaryExpression->getOperator();

      UnaryGPExpressionPtr unaryLeft = left.dynamicCast<UnaryGPExpression>();
      UnaryGPExpressionPtr unaryRight = right.dynamicCast<UnaryGPExpression>();

      if (op == gpDivision)
      {
        // x / inverse(y) = x * y
        if (unaryRight && unaryRight->getOperator() == gpInverse)
          return rewrite(new BinaryGPExpression(left, gpMultiplication, unaryRight->getExpression()));

        // inverse(x) / y = inverse(x * y)
      }
      else if (op == gpMultiplication)
      {
        // inverse(x) * y = y / x
        if (unaryLeft && unaryLeft->getOperator() == gpInverse)
          return rewrite(new BinaryGPExpression(right, gpDivision, unaryLeft->getExpression()));

        // x * inverse(y) = x / y
        if (unaryRight && unaryRight->getOperator() == gpInverse)
          return rewrite(new BinaryGPExpression(left, gpDivision, unaryRight->getExpression()));
      }

      if (op == gpAddition || op == gpMultiplication)
      {
        // x + y and x * y commutativity
        if (Variable(left).compare(right) > 0)
          return rewrite(new BinaryGPExpression(right, op, left));
      }
      
      return new BinaryGPExpression(left, op, right);
    }

    UnaryGPExpressionPtr unaryExpression = expression.dynamicCast<UnaryGPExpression>();
    if (unaryExpression)
    {
      GPExpressionPtr expr = rewrite(unaryExpression->getExpression());
      return new UnaryGPExpression(unaryExpression->getOperator(), expr);
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
    std::map<Variable, Variable>::const_iterator it = cache.find(input.toShortString());
    if (it == cache.end())
      return false;
    juce::atomicIncrement(cacheAccesses);
    output = it->second;
    return true;
  }

  void addResultInCache(ExecutionContext& context, const Variable& input, const Variable& output)
  {
    //context.informationCallback(T("cache ") + input.toShortString() + T(" (") + output.toShortString() + T(")"));
    ScopedWriteLock _(cacheLock); cache[input.toShortString()] = output;
  }
};

typedef ReferenceCountedObjectPtr<UnaryCacheFunction> UnaryCacheFunctionPtr;

/*
** SandBox
*/
class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() : numBandits(2), maxTimeStep(100), numTrainingProblems(100), numTestingProblems(1000), l0Weight(0.0) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    /*
    ** Make training and testing problems
    */
    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    SamplerPtr initialStateSampler = new DiscreteBanditInitialStateSampler(objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0)), numBandits);

    std::vector<DiscreteBanditStatePtr> testingStates(numTestingProblems);
    for (size_t i = 0; i < testingStates.size(); ++i)
      testingStates[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    std::vector<DiscreteBanditStatePtr> trainingStates(numTrainingProblems);
    for (size_t i = 0; i < trainingStates.size(); ++i)
      trainingStates[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();


    /*
    ** Compute a bunch of baseline policies
    */
    std::vector<DiscreteBanditPolicyPtr> policies;
    //policies.push_back(greedyDiscreteBanditPolicy());
    //policies.push_back(ucb1DiscreteBanditPolicy());
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    //policies.push_back(ucb1NormalDiscreteBanditPolicy());
    //policies.push_back(ucb2DiscreteBanditPolicy());
    //policies.push_back(ucbvDiscreteBanditPolicy());
    //policies.push_back(epsilonGreedyDiscreteBanditPolicy());
    
    //policies.push_back(gpExpressionDiscreteBanditPolicy(new VariableGPExpression(Variable(2, gpExpressionDiscreteBanditPolicyVariablesEnumeration))));

    //policies.push_back(new UCBVDiscreteBanditPolicy());
    //policies.push_back(new UCB1NormalDiscreteBanditPolicy());
    //policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.05, 0.1));

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), policies.size() * 2);
    for (size_t i = 0; i < policies.size(); ++i)
    {
      //policies[i]->saveToFile(context, context.getFile(T("Policies/") + policies[i]->toString() + T(".policy")));
      workUnit->setWorkUnit(i * 2, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), policies[i], true));
      workUnit->setWorkUnit(i * 2 + 1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), policies[i]->cloneAndCast<DiscreteBanditPolicy>(), true));
    }
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);

    ultimatePolicySearch(context, trainingStates, testingStates);
    return true;


    std::vector<std::pair<DiscreteBanditPolicyPtr, String> > policiesToOptimize;
    policiesToOptimize.push_back(std::make_pair(powerDiscreteBanditPolicy(2, false), T("powerFunction2-dense")));
    policiesToOptimize.push_back(std::make_pair(powerDiscreteBanditPolicy(2, true), T("powerFunction2-sparse")));
    //policiesToOptimize.push_back(std::make_pair(new GPExpressionDiscreteBanditPolicy(), T("ultimate")));

    
    //policiesToOptimize.push_back(std::make_pair(new PowerDiscreteBanditPolicy(2), T("powerFunction2")));
    /*policiesToOptimize.push_back(std::make_pair(new UCBVDiscreteBanditPolicy(), T("UCBv")));
    policiesToOptimize.push_back(std::make_pair(new UCB2DiscreteBanditPolicy(), T("UCB2")));
    policiesToOptimize.push_back(std::make_pair(new WeightedUCBBanditPolicy(), T("WeightedUCB1")));
    policiesToOptimize.push_back(std::make_pair(new EpsilonGreedyDiscreteBanditPolicy(0.1, 0.1), T("epsilonGreedy")));*/

    //policiesToOptimize.push_back(std::make_pair(new OldStyleParameterizedBanditPolicy(false), T("oldStyle")));

    //policiesToOptimize.push_back(std::make_pair(new GPExpressionDiscreteBanditPolicy(), T("ultimate")));
    //policiesToOptimize.push_back(std::make_pair(new OldStyleParameterizedBanditPolicy(true), T("oldStyle-sparse")));

    
    //policiesToOptimize.push_back(std::make_pair(new TimeDependentWeightedUCBBanditPolicy(), T("timeDependentWeighted")));

    //policiesToOptimize.push_back(new PerTimesWeightedUCBBanditPolicy(maxTimeStep));
    //policiesToOptimize.push_back(new WeightedUCBBanditPolicy());
    //policiesToOptimize.push_back(new OldStyleParameterizedBanditPolicy());
    
    
    //policiesToOptimize.push_back(new PerTimesWeightedUCBBanditPolicy(maxTimeStep));

    //for (size_t i = 1; i < 10; ++i)
    //  policiesToOptimize.push_back(new MomentsWeightedUCBBanditPolicy(i));

    for (size_t i = 0; i < policiesToOptimize.size(); ++i)
    {
      DiscreteBanditPolicyPtr policyToOptimize = policiesToOptimize[i].first;
      const String& policyName = policiesToOptimize[i].second;

      context.enterScope(T("Optimizing ") + policyName);
      Variable bestParameters = optimizePolicy(context, policyToOptimize, trainingStates, testingStates);
      DiscreteBanditPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policyToOptimize, bestParameters);
      //String outputFileName = T("Policies100iter/") + String((int)maxTimeStep) + T("/") + policyName + T(".policy");
      //context.informationCallback(T("Saving policy ") + outputFileName);
      //optimizedPolicy->saveToFile(context, context.getFile(outputFileName));

      // evaluate on train and on test
      workUnit = new CompositeWorkUnit(T("Evaluating optimized policy"), 2);
      // policy must be cloned since it may be used in multiple threads simultaneously
      workUnit->setWorkUnit(0, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setWorkUnit(1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setProgressionUnit(T("Problem Sets"));
      workUnit->setPushChildrenIntoStackFlag(true);
      context.run(workUnit);

      context.leaveScope(true);
    }
    return true;
  }

protected:
  friend class BanditsSandBoxClass;

  size_t numBandits;
  size_t maxTimeStep;

  size_t numTrainingProblems;
  size_t numTestingProblems;

  double l0Weight;

//  DiscreteBanditPolicyPtr policyToOptimize;

  WorkUnitPtr makeEvaluationWorkUnit(const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose) const
    {return new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, initialStatesDescription, policy, 100, verbose);}
  
  Variable optimizePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& trainingStates,
                          const std::vector<DiscreteBanditStatePtr>& testingStates, size_t numIterations = 100)
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
    size_t populationSize = numParameters * 8;
    size_t numBests = numParameters * 2;

    if (populationSize < 50)
      populationSize = 50;
    if (numBests < 10)
      numBests = 10;

    // optimizer state
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(Parameterized::get(policy)->createParametersSampler());

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, maxTimeStep, trainingStates, 100, l0Weight);

    objectiveFunction->initialize(context, parametersType);

    FunctionPtr validationFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, maxTimeStep, testingStates);
    validationFunction->initialize(context, parametersType);

    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objectiveFunction, validationFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests);
    //OptimizerPtr optimizer = asyncEDAOptimizer(numIterations*populationSize, populationSize, populationSize/numBests, 1, 10, populationSize);

    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    Variable bestParameters = optimizerState->getBestVariable();
    context.resultCallback(T("optimizedPolicy"), Parameterized::cloneWithNewParameters(policy, bestParameters));
    return bestParameters;
  }

  bool ultimatePolicySearch(ExecutionContext& context, const std::vector<DiscreteBanditStatePtr>& trainingStates, const std::vector<DiscreteBanditStatePtr>& testingStates)
  {
    FunctionPtr objective = new EvaluateOptimizedDiscreteBanditPolicyParameters(
      gpExpressionDiscreteBanditPolicy(), numBandits, maxTimeStep, trainingStates);
    
    UnaryCacheFunctionPtr cacheFunction = new UnaryCacheFunction(objective);
    objective = composeFunction(new MakeGPExpressionUnique(), cacheFunction);

    FunctionPtr validation = new EvaluateOptimizedDiscreteBanditPolicyParameters(
      gpExpressionDiscreteBanditPolicy(), numBandits, maxTimeStep, testingStates);
    if (!objective->initialize(context, gpExpressionClass) ||
        !validation->initialize(context, gpExpressionClass))
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
    for (size_t maxDepth = 1; maxDepth <= 5; ++maxDepth)
    {
      context.enterScope(T("maxDepth = ") + String((int)maxDepth));

      DecisionProblemStatePtr state = new CompactGPExpressionBuilderState(T("toto"), gpExpressionDiscreteBanditPolicyVariablesEnumeration, objective);
      double allTimesBestScore = DBL_MAX;
      double allTimesBestValidation = DBL_MAX;
      GPExpressionPtr allTimesBestExpression;

      for (size_t timeStep = 0; timeStep < 10; ++timeStep)
      {
        context.enterScope(T("TimeStep ") + String((int)timeStep));
        double bestScore = DBL_MAX;
        GPExpressionPtr bestExpression;
        Variable bestFirstAction;
        Variable currentFirstAction;
        recursiveExhaustiveSearch(context, state, validation, 0, maxDepth, bestScore, bestExpression, bestFirstAction, currentFirstAction);
        double bestValidation = validation->compute(context, bestExpression).toDouble();

        context.resultCallback(T("timeStep"), timeStep);
        context.resultCallback(T("bestScore"), bestScore);
        context.resultCallback(T("bestValidation"), bestValidation);
        context.resultCallback(T("bestExpression"), bestExpression);
        context.informationCallback(T("Best: ") + bestExpression->toShortString() + T(" -> ") + String(bestScore) + T(", ") + String(bestValidation));
        context.informationCallback(T("Best First Action: ") + bestFirstAction.toShortString());
        cacheFunction->displayAndFlushCacheUsage(context);

        if (bestScore < allTimesBestScore)
          allTimesBestScore = bestScore, allTimesBestValidation = bestValidation, allTimesBestExpression = bestExpression;

        double reward;
        state->performTransition(context, bestFirstAction, reward);

        context.leaveScope(new Pair(bestScore, bestValidation));
        if (state->isFinalState())
        {
          context.informationCallback(T("Reached Final State"));
          break;
        }
      }
      context.informationCallback(T("Best: ") + allTimesBestExpression->toShortString());

      context.resultCallback(T("bestScore"), allTimesBestScore);
      context.resultCallback(T("bestValidation"), allTimesBestValidation);
      context.resultCallback(T("bestExpression"), allTimesBestExpression);
      context.leaveScope(new Pair(allTimesBestScore, allTimesBestValidation));
    }
    return true;

  
    //return breadthFirstSearch(context, ultimatePolicyVariablesEnumeration, objective, validation);
  }

  void recursiveExhaustiveSearch(ExecutionContext& context, CompactGPExpressionBuilderStatePtr state, const FunctionPtr& validation,
                  size_t depth, size_t maxDepth, double& bestScore, GPExpressionPtr& bestExpression, Variable& bestFirstAction, Variable& currentFirstAction)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = actions->getElement(i);
      if (depth == 0)
        currentFirstAction = action;

      double reward;
      state->performTransition(context, action, reward);

      if (state->isFinalState())
      {
        double score = -reward;
        if (score < bestScore)
        {
          bestScore = score;
          bestExpression = state->getExpression()->cloneAndCast<GPExpression>();
          //context.informationCallback(bestExpression->toShortString() + T(" -> ") + String(bestScore));
          bestFirstAction = currentFirstAction;
        }
      }
      else
      {
        size_t nextDepth = depth + 1;
        if (nextDepth < maxDepth)
          recursiveExhaustiveSearch(context, state, validation, nextDepth, maxDepth, bestScore, bestExpression, bestFirstAction, currentFirstAction);
      }
      state->undoTransition(context, action);
    }
  } 


  bool breadthFirstSearch(ExecutionContext& context, EnumerationPtr inputVariables, const FunctionPtr& objective, const FunctionPtr& validation, size_t maxSearchNodes = 50)
  {
    DecisionProblemPtr problem = new GPExpressionBuilderProblem(inputVariables, objective);
    DecisionProblemStatePtr state = new CompactGPExpressionBuilderState(T("toto"), inputVariables, objective);

    for (size_t depth = 0; depth < 10; ++depth)
    {
      context.enterScope(T("Depth ") + String((int)depth + 1));

      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
      
      PolicyPtr searchPolicy = bestFirstSearchPolicy(new MinDepthSearchHeuristic());

      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);

      context.resultCallback(T("bestReturn"), searchTree->getBestReturn());
      bool isFinished = (searchTree->getBestReturn() <= 0.0);
      if (!isFinished)
      {
        context.resultCallback(T("bestAction"), searchTree->getBestAction());
        context.resultCallback(T("bestTrajectory"), searchTree->getBestNodeTrajectory());

        double transitionReward;
        state->performTransition(context, searchTree->getBestAction(), transitionReward);
        context.resultCallback(T("newState"), state->clone(context));
        context.resultCallback(T("transitionReward"), transitionReward);

        GPExpressionBuilderStatePtr expressionBuilderState = state.dynamicCast<GPExpressionBuilderState>();
        jassert(expressionBuilderState);
        GPExpressionPtr expression = expressionBuilderState->getExpression();
        double trainScore = expressionBuilderState->getScore();
        double validationScore = validation->compute(context, expression).toDouble();

        context.resultCallback(T("expression"), expression);
        context.resultCallback(T("score"), trainScore);
        context.resultCallback(T("validationScore"), validationScore);
        context.informationCallback(T("Best Formula: ") + expression->toShortString());

        context.leaveScope(new Pair(trainScore, validationScore));
      }
      else
      {
        context.leaveScope(state);
        break;
      }
    }
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

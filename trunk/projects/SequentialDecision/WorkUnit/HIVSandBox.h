/*-----------------------------------------.---------------------------------.
| Filename: HIVSandBox.h                   | HIV Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2011 13:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_

# include "../Core/SearchTree.h"
# include "../Core/SearchPolicy.h"
# include "../Core/SearchHeuristic.h"
# include "../Problem/DamienDecisionProblem.h"
# include "../Problem/LinearPointPhysicProblem.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class WeightedRewardsFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return searchTreeNodeClass();}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    for (size_t i = 0; i < 10; ++i)
      res->addElement(context, T("r.") + String((i + 1.0) / 10.0) + T("^d"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const SearchTreeNodePtr& node = inputs[0].getObjectAndCast<SearchTreeNode>();
    size_t depth = node->getDepth();
    double reward = node->getReward();
    
    for (size_t i = 0; i < 10; ++i)
    {
      double lambda = (i + 1.0) / 10.0;
      callback.sense(i, reward * pow(lambda, (double)depth));
    }
  }
};

class SimpleSearchNodeFeatureGenerator : public CompositeFunction
{
public:
  SimpleSearchNodeFeatureGenerator(bool applyLog = false, bool includeUnit = true)
    : applyLog(applyLog), includeUnit(includeUnit) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass(), T("node"));
    size_t state = builder.addFunction(getVariableFunction(T("state")), node);

    std::vector<size_t> features;
    size_t r = builder.addFunction(getVariableFunction(T("reward")), node, T("r"));
    if (applyLog)
      r = builder.addFunction(lbcppMemberUnaryFunction(SimpleSearchNodeFeatureGenerator, computeLogFunction, doubleType, doubleType), r, T("logr"));
    features.push_back(r);

    FunctionPtr convertFunction = lbcppMemberUnaryFunction(SimpleSearchNodeFeatureGenerator, convertToDoubleFunction, variableType, doubleType);
    size_t depth = builder.addFunction(getVariableFunction(T("depth")), node);
    features.push_back(builder.addFunction(convertFunction, depth, T("d")));

    if (includeUnit)
      features.push_back(builder.addConstant(Variable(1.0), T("unit")));

    size_t nodeFeatures = builder.addFunction(concatenateFeatureGenerator(true), features, T("node"));
    size_t stateFeatures = builder.addFunction(objectDoubleMembersFeatureGenerator(), state, T("s"));

    if (applyLog)
      stateFeatures = builder.addFunction(mapContainerFunction(lbcppMemberUnaryFunction(SimpleSearchNodeFeatureGenerator, computeLogFunction, doubleType, doubleType)), stateFeatures, T("logS"));

    builder.addFunction(cartesianProductFeatureGenerator(true), nodeFeatures, stateFeatures, T("prod"));
  }

  Variable convertToDoubleFunction(ExecutionContext& context, const Variable& input) const
    {return input.toDouble();}

  Variable computeLogFunction(ExecutionContext& context, const Variable& input) const
  {
    double v = input.getDouble();
    return v >= 1.0 ? log(v) : 0.0;
  }
  
protected:
  friend class SimpleSearchNodeFeatureGeneratorClass;

  bool applyLog;
  bool includeUnit;
};

///////////////////////////////////////////////////

// HIVState -> Features
class HIVStateFeatures : public CompositeFunction
{
public:
  Variable getEValueFromState(ExecutionContext& context, const Variable& state) const
    {return state.getObjectAndCast<HIVDecisionProblemState>()->getE();}
 
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t stateEValue = builder.addFunction(lbcppMemberUnaryFunction(HIVStateFeatures, getEValueFromState, decisionProblemStateClass, doubleType), state);
    builder.addFunction(softDiscretizedLogNumberFeatureGenerator(1, 6, 6), stateEValue, T("e"));
  }
};

class LPPStateFeatures : public CompositeFunction
{
public:
  Variable getPositionFromState(ExecutionContext& context, const Variable& state) const
    {return state.getObjectAndCast<LinearPointPhysicState>()->getPosition();}
 
    virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t state = builder.addInput(decisionProblemStateClass, T("state"));
    size_t position = builder.addFunction(lbcppMemberUnaryFunction(LPPStateFeatures, getPositionFromState, decisionProblemStateClass, doubleType), state);
    builder.addFunction(softDiscretizedNumberFeatureGenerator(-1.0, 1.0, 6), position, T("p"));
  }
};

class HIVSearchFeatures : public CompositeFunction
{
public:
  HIVSearchFeatures(double discount, double maxReward, double maxDepth, FunctionPtr stateFeatures = FunctionPtr())
    : discount(discount), maxReward(maxReward), maxDepth(maxDepth), stateFeatures(stateFeatures) {}
  HIVSearchFeatures() : discount(1.0), maxReward(1.0), maxDepth(1.0) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass(), T("node"));

    //size_t depth = builder.addFunction(getVariableFunction(T("depth")), node);
    //size_t reward = builder.addFunction(getVariableFunction(T("reward")), node);
    //size_t currentReturn = builder.addFunction(getVariableFunction(T("currentReturn")), node);

    builder.startSelection();

      builder.addFunction(greedySearchHeuristic(1.0, maxReward), node, T("maxReward"));
      builder.addFunction(greedySearchHeuristic(discount, maxReward), node, T("maxDiscountedReward"));
      builder.addFunction(maxReturnSearchHeuristic(maxReward / (1.0 - discount)), node, T("maxReturn"));
     // builder.addFunction(optimisticPlanningSearchHeuristic(discount), node, T("optimistic"));
      builder.addFunction(minDepthSearchHeuristic(maxDepth), node, T("minDepth"));

    size_t heuristics = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true), T("heuristics"));
    
    if (stateFeatures)
    {
      size_t state = builder.addFunction(getVariableFunction(T("state")), node);
      size_t stateFeatures = builder.addFunction(this->stateFeatures, state, T("stateFeatures"));
      builder.addFunction(cartesianProductFeatureGenerator(), heuristics, stateFeatures, T("sdh"));
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class HIVSearchFeaturesClass;

  double discount;
  double maxReward;
  double maxDepth;
  FunctionPtr stateFeatures;
};

class HIVSearchHeuristic : public LearnableSearchHeuristic
{
public:
  HIVSearchHeuristic(const FunctionPtr& featuresFunction, DenseDoubleVectorPtr parameters)
    : featuresFunction(featuresFunction), parameters(parameters) {}
  HIVSearchHeuristic() {}

  virtual FunctionPtr createPerceptionFunction() const
    {return featuresFunction;}

  virtual FunctionPtr createScoringFunction() const
  {
    NumericalLearnableFunctionPtr function = linearLearnableFunction();
    function->setParameters(parameters);
    return function;
  }

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    LearnableSearchHeuristic::buildFunction(builder);
    featuresEnumeration = DoubleVector::getElementsEnumeration(featuresFunction->getOutputType());
  }

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
  {
    if (!featuresFunction)
      return ObjectPtr();
    jassert(variableName == T("featuresEnumeration"));
    if (!featuresFunction->isInitialized() && !featuresFunction->initialize(context, searchTreeNodeClass()))
      return ObjectPtr();
    return DoubleVector::getElementsEnumeration(featuresFunction->getOutputType());
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class HIVSearchHeuristicClass;

  FunctionPtr featuresFunction;
  EnumerationPtr featuresEnumeration;
  DenseDoubleVectorPtr parameters;
};

// DenseDoubleVector -> Double
class EvaluateHIVSearchHeuristic : public SimpleUnaryFunction
{
public:
  EvaluateHIVSearchHeuristic(DecisionProblemPtr problem, const FunctionPtr& featuresFunction, size_t maxSearchNodes, size_t maxHorizon, double discount)
    : SimpleUnaryFunction(TypePtr(), doubleType), problem(problem), featuresFunction(featuresFunction),
      maxSearchNodes(maxSearchNodes), maxHorizon(maxHorizon), discount(discount)
  {
    bool ok = featuresFunction->initialize(defaultExecutionContext(), searchTreeNodeClass(problem->getStateClass(), problem->getActionType()));
    if (ok)
      inputType = denseDoubleVectorClass(DenseDoubleVector::getElementsEnumeration(featuresFunction->getOutputType()));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& parameters = input.getObjectAndCast<DenseDoubleVector>();
    FunctionPtr heuristic = new HIVSearchHeuristic(featuresFunction, parameters);
    PolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);

    DecisionProblemStatePtr state = problem->sampleInitialState(RandomGenerator::getInstance());

    double res = 0.0;
    for (size_t i = 0; i < maxHorizon; ++i)
    {
      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
      Variable action = searchTree->getBestAction();

      double reward = 0.0;
      state->performTransition(action, reward);
      res += reward * pow(discount, (double)i);
    }
    return -res;
  }

protected:
  DecisionProblemPtr problem;
  FunctionPtr featuresFunction;
  size_t maxSearchNodes;
  size_t maxHorizon;
  double discount;
};

extern ClassPtr independentDoubleVectorDistributionClass(TypePtr elementsEnumeration);

class IndependentDoubleVectorDistribution : public MultiVariateDistribution
{
public:
  IndependentDoubleVectorDistribution(EnumerationPtr elementsEnumeration)
    : MultiVariateDistribution(independentDoubleVectorDistributionClass(elementsEnumeration)),
      elementsEnumeration(elementsEnumeration), elementsType(denseDoubleVectorClass(elementsEnumeration))
  {
    subDistributions.resize(elementsEnumeration->getNumElements());
  }
  IndependentDoubleVectorDistribution() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual double computeEntropy() const
    {jassert(false); return 0.0;}

  virtual double computeProbability(const Variable& value) const
    {jassert(false); return 0.0;}

  virtual Variable sample(RandomGeneratorPtr random) const
  {
    // /!\ This distribution only outputs normalized double vectors
    DenseDoubleVectorPtr res(new DenseDoubleVector(elementsType, subDistributions.size()));
  //  double sumOfSquares = 0.0;
    for (size_t i = 0; i < subDistributions.size(); ++i)
    {
      double value = subDistributions[i]->sample(random).getDouble();
  //    sumOfSquares += value * value;
      res->setValue(i, value);
    }
 //   if (sumOfSquares)
 //     res->multiplyByScalar(1.0 / sqrt(sumOfSquares)); // normalize
    return res;
  }

  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {jassert(false); return Variable();}
  
  virtual DistributionBuilderPtr createBuilder() const;

  void setSubDistribution(size_t index, const DistributionPtr& distribution)
    {subDistributions[index] = distribution;}

  size_t getNumSubDistributions() const
    {return subDistributions.size();}

protected:
  friend class IndependentDoubleVectorDistributionClass;

  EnumerationPtr elementsEnumeration;
  TypePtr elementsType;
  std::vector<DistributionPtr> subDistributions;
};

typedef ReferenceCountedObjectPtr<IndependentDoubleVectorDistribution> IndependentDoubleVectorDistributionPtr;

class IndependentDoubleVectorDistributionBuilder : public DistributionBuilder
{
public:
  IndependentDoubleVectorDistributionBuilder(EnumerationPtr elementsEnumeration, const std::vector<DistributionBuilderPtr>& subBuilders)
    : elementsEnumeration(elementsEnumeration), subBuilders(subBuilders) {}
  IndependentDoubleVectorDistributionBuilder() {}

  virtual TypePtr getInputType() const
    {return denseDoubleVectorClass();}

  virtual void clear()
  {
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->clear();
  }

  virtual void addElement(const Variable& element, double weight = 1.0)
  {
    const DenseDoubleVectorPtr& doubleVector = element.getObjectAndCast<DenseDoubleVector>();
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->addElement(doubleVector->getValue(i), weight);
  }

  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0)
    {jassert(false);}

  virtual DistributionPtr build(ExecutionContext& context) const
  {
    IndependentDoubleVectorDistributionPtr res = new IndependentDoubleVectorDistribution(elementsEnumeration);
    jassert(res->getNumSubDistributions() == subBuilders.size());
    for (size_t i = 0; i < subBuilders.size(); ++i)
      res->setSubDistribution(i, subBuilders[i]->build(context));
    return res;
  }

private:
  EnumerationPtr elementsEnumeration;
  std::vector<DistributionBuilderPtr> subBuilders;
};

inline DistributionBuilderPtr IndependentDoubleVectorDistribution::createBuilder() const
{
  std::vector<DistributionBuilderPtr> subBuilders(subDistributions.size());
  for (size_t i = 0; i < subBuilders.size(); ++i)
    subBuilders[i] = subDistributions[i]->createBuilder();
  return new IndependentDoubleVectorDistributionBuilder(elementsEnumeration, subBuilders);
}

class HIVSandBox : public WorkUnit
{
public:
  // default values
  HIVSandBox() :  discount(0.98), minDepth(0), maxDepth(10), maxHorizon(300),
                  verboseTrajectories(false),
                  iterations(100), populationSize(100), numBests(10), reinjectBest(false),
                  baseHeuristics(false), optimisticHeuristics(false), learnedHeuristic(false)
  {
    problem = hivDecisionProblem(discount);
    featuresFunction = new SimpleSearchNodeFeatureGenerator();
  }

  virtual Variable run(ExecutionContext& context)
  {
    if (numBests >= populationSize)
    {
      context.errorCallback(T("Invalid parameters: numBests is greater than populationSize"));
      return false;
    }

    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }
    discount = problem->getDiscount();

    if (!featuresFunction)
    {
      context.errorCallback(T("No features function"));
      return false;
    }

    /* features parameters 
    double maxReward = normalizeHeuristics ? problem->getMaxReward() : 1.0;
    double depthNormalize = normalizeHeuristics ? (double)maxHorizon : 1.0;
    FunctionPtr stateFeatures;
    if (stateDependentHeuristic)
    {
      if (problem.isInstanceOf<HIVDecisionProblem>())
        stateFeatures = new HIVStateFeatures();
      else if (problem.isInstanceOf<LinearPointPhysicProblem>())
        stateFeatures = new LPPStateFeatures();
      else
      {
        context.errorCallback(T("No known state features for this kind of problems"));
        return false;
      }
    }*/

    //FunctionPtr featuresFunction = new HIVSearchFeatures(discount, maxReward, depthNormalize, stateFeatures);
    //FunctionPtr featuresFunction = new GenericSearchNodeFeatureGenerator(false, true, true, true, false, true);
    //FunctionPtr featuresFunction = new WeightedRewardsFeatureGenerator();

    // check depth parameters
    if (minDepth > maxDepth)
    {
      context.errorCallback(T("Min depth is higher than max detph"));
      return false;
    }

    size_t fixedNumberOfActions = problem->getFixedNumberOfActions();
    if (!fixedNumberOfActions)
    {
      context.errorCallback(T("This problem does not have a fixed number of actions"));
      return false;
    }

    // compute initial value of maxSearchNodes
    size_t maxSearchNodes = 1;
    for (size_t depth = 0; depth < minDepth; ++depth)
      maxSearchNodes = maxSearchNodes * fixedNumberOfActions + 1; // any general formula for that ?

    // iterator over all possible depth values
    for (size_t depth = minDepth; depth <= maxDepth; ++depth)
    {
      context.enterScope(T("D = ") + String((int)depth) + T(" N = ")+ String((int)maxSearchNodes));
      context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
      context.resultCallback(T("depth"), depth);

      if (baseHeuristics)
      {
        // BASELINES
        computeTrajectory(context, problem, greedySearchHeuristic(), T("maxReward"), maxSearchNodes);
        computeTrajectory(context, problem, greedySearchHeuristic(discount), T("maxDiscountedReward"), maxSearchNodes);
        computeTrajectory(context, problem, maxReturnSearchHeuristic(), T("maxReturn"), maxSearchNodes);
        computeTrajectory(context, problem, minDepthSearchHeuristic(), T("minDepth"), maxSearchNodes);
      }

      if (optimisticHeuristics)
      {
        if (problem.isInstanceOf<HIVDecisionProblem>())
        {
          for (int i = 3; i <= 15; i += 3)
            computeTrajectory(context, problem, optimisticPlanningSearchHeuristic(discount, pow(10.0, (double)i)), T("optimistic(10^") + String(i) + T(")"), maxSearchNodes);
        }
        else
          computeTrajectory(context, problem, optimisticPlanningSearchHeuristic(discount), T("optimistic"), maxSearchNodes);
      }

      if (learnedHeuristic)
      {
        // EDA
        FunctionPtr functionToOptimize = new EvaluateHIVSearchHeuristic(problem, featuresFunction, maxSearchNodes, maxHorizon, discount);
        
        EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(functionToOptimize->getRequiredInputType(0, 1));
        if (!featuresEnumeration)
        {
          context.errorCallback(T("Problem in feature function"));
          return false;
        }
        context.informationCallback(String((int)featuresEnumeration->getNumElements()) + T(" parameters"));
        ClassPtr parametersClass = denseDoubleVectorClass(featuresEnumeration, doubleType);

        if (!functionToOptimize->initialize(context, parametersClass))
          return false;

        IndependentDoubleVectorDistributionPtr initialDistribution = new IndependentDoubleVectorDistribution(featuresEnumeration);
        for (size_t i = 0; i < featuresEnumeration->getNumElements(); ++i)
          initialDistribution->setSubDistribution(i, new GaussianDistribution(0.0, 1.0));
      
        RandomGeneratorPtr random = RandomGenerator::getInstance();
        //double bestIndividualScore = evaluateEachFeature(context, functionToOptimize, featuresEnumeration);

        Variable bestParameters;
        double bestScore = performEDA(context, functionToOptimize, initialDistribution, bestParameters);

        // Evaluate EDA
        FunctionPtr bestHeuristic = new HIVSearchHeuristic(featuresFunction, bestParameters.getObjectAndCast<DenseDoubleVector>());
        context.resultCallback(T("model"), bestHeuristic);
        computeTrajectory(context, problem, bestHeuristic, T("learnedHeuristic"), maxSearchNodes);        
      }

      context.leaveScope();
      maxSearchNodes = maxSearchNodes * fixedNumberOfActions + 1;
    }

    return true;
  }

private:
  friend class HIVSandBoxClass;
 
  DecisionProblemPtr problem;
  FunctionPtr featuresFunction;
 
  double discount;
  size_t minDepth, maxDepth;
  size_t maxHorizon;
  bool verboseTrajectories;

  size_t iterations;
  size_t populationSize;
  size_t numBests;
  bool reinjectBest;

  bool breadthFirstHeuristic;
  bool baseHeuristics;
  bool optimisticHeuristics;
  bool learnedHeuristic;

  double performEDA(ExecutionContext& context, const FunctionPtr& functionToOptimize, const DistributionPtr& initialDistribution, Variable& bestParameters) const
  {
    double bestScore = DBL_MAX;

    DistributionPtr distribution = initialDistribution;
    context.enterScope(T("Optimizing"));
    for (size_t i = 0; i < iterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      Variable bestIterationParameters = bestParameters;
      double score = performEDAIteration(context, functionToOptimize, distribution, bestIterationParameters);
      context.resultCallback(T("bestParameters"), bestIterationParameters);

      //context.resultCallback(T("distribution"), distribution);
      context.leaveScope(score);
      if (score < bestScore)
      {
        bestScore = score;
        bestParameters = bestIterationParameters;
      }
      context.progressCallback(new ProgressionState(i + 1, iterations, T("Iterations")));
    }
    context.leaveScope(bestScore);
    return bestScore;
  }

  double performEDAIteration(ExecutionContext& context, const FunctionPtr& functionToMinimize, DistributionPtr& distribution, Variable& bestParameters) const
  {
    jassert(numBests < populationSize);
    //Object::displayObjectAllocationInfo(std::cout);

    RandomGeneratorPtr random = RandomGenerator::getInstance();
    
    // generate evaluation work units
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Evaluating ") + String((int)populationSize) + T(" parameters"), populationSize));
    std::vector<Variable> inputs(populationSize);
    std::vector<Variable> scores(populationSize);
    for (size_t i = 0; i < populationSize; ++i)
    {
      if (reinjectBest && i == 0 && bestParameters.exists())
        inputs[0] = bestParameters;
      else
        inputs[i] = distribution->sample(random);
      workUnit->setWorkUnit(i, new FunctionWorkUnit(functionToMinimize, inputs[i], String::empty, &scores[i]));
    }
    workUnit->setProgressionUnit(T("parameters"));
    workUnit->setPushChildrenIntoStackFlag(false);

    // run work units
    context.run(workUnit);

    // sort by scores
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < populationSize; ++i)
      sortedScores.insert(std::make_pair(scores[i].getDouble(), i));
    jassert(sortedScores.size() == populationSize);

    // build new distribution
    std::multimap<double, size_t>::const_iterator it = sortedScores.begin();
    DistributionBuilderPtr builder = distribution->createBuilder();
    for (size_t i = 0; i < numBests; ++i, ++it)
    {
      size_t index = it->second;
      //context.informationCallback(T("Best ") + String((int)i + 1) + T(": ") + inputs[index].toShortString() + T(" (") + scores[index].toShortString() + T(")"));
      builder->addElement(inputs[index]);
    }
    distribution = builder->build(context);

    // return best score
    bestParameters = inputs[sortedScores.begin()->second];
    return sortedScores.begin()->first;
  }

  double evaluateEachFeature(ExecutionContext& context, const FunctionPtr& functionToMinimize, EnumerationPtr featuresEnumeration) const
  {
    context.enterScope(T("Evaluating features"));
    size_t n = featuresEnumeration->getNumElements();
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Evaluating ") + String((int)n) + T(" features"), n));
    std::vector<Variable> scores(n);
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr doubleVector = new DenseDoubleVector(featuresEnumeration, doubleType, n);
      doubleVector->setValue(i, 1.0);
      workUnit->setWorkUnit(i, new FunctionWorkUnit(functionToMinimize, doubleVector, String::empty, &scores[i]));
    }
    workUnit->setProgressionUnit(T("features"));
    workUnit->setPushChildrenIntoStackFlag(false);
    context.run(workUnit);

    double bestScore = DBL_MAX;
    for (size_t i = 0; i < n; ++i)
    {
      double score = scores[i].getDouble();
      if (score < bestScore)
        bestScore = score;
      context.resultCallback(featuresEnumeration->getElementName(i), score);
    }
    context.leaveScope(bestScore);
    return bestScore;
  }

  double computeTrajectory(ExecutionContext& context, const DecisionProblemPtr& problem, const FunctionPtr& heuristic, const String& name, size_t maxSearchNodes) const
  {
    PolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);
    DecisionProblemStatePtr state = problem->sampleInitialState(RandomGenerator::getInstance());

    if (verboseTrajectories)
      context.enterScope(T("Trajectory ") + name);
    double res = 0.0;
    for (size_t i = 0; i < maxHorizon; ++i)
    {
      if (verboseTrajectories)
        context.enterScope(T("element ") + String((int)i));

      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
      Variable bestAction = searchTree->getBestAction();

      double reward = 0.0;
      state->performTransition(bestAction, reward);
      res += reward * pow(discount, (double)i);

      if (verboseTrajectories)
      {
        context.resultCallback(T("time"), i + 1);
        context.resultCallback(T("reward"), reward);
        
        HIVDecisionProblemStatePtr hivState = state.dynamicCast<HIVDecisionProblemState>();
        if (hivState)
        {
          DenseDoubleVectorPtr hivAction = bestAction.dynamicCast<DenseDoubleVector>();

          context.resultCallback(T("RTI"), hivAction->getValue(0) > 0.0);
          context.resultCallback(T("PI"), hivAction->getValue(1) > 0.0);

          for (size_t i = 0; i < 6; ++i)
          {
            String name = hivState->getVariableName(i);
            double value = hivState->getVariable(i).getDouble();
            context.resultCallback(T("log10(") + name + T(")"), log10(value));
          }
        }

        LinearPointPhysicStatePtr linearPointState = state.dynamicCast<LinearPointPhysicState>();
        if (linearPointState)
        {
          context.resultCallback(T("action"), bestAction);
          context.resultCallback(T("position"), linearPointState->getPosition());
          context.resultCallback(T("velocity"), linearPointState->getVelocity());
        }

        context.leaveScope(res);
      }
    }
    if (verboseTrajectories)
      context.leaveScope(res);
    context.resultCallback(name, res);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

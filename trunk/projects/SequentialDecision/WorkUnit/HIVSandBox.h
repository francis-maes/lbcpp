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

    size_t depth = builder.addFunction(getVariableFunction(T("depth")), node);
    features.push_back(builder.addFunction(convertToDoubleFunction(), depth, T("d")));

    if (includeUnit)
      features.push_back(builder.addConstant(Variable(1.0), T("unit")));

    size_t nodeFeatures = builder.addFunction(concatenateFeatureGenerator(true), features, T("node"));
    size_t stateFeatures = builder.addFunction(objectDoubleMembersFeatureGenerator(), state, T("s"));

    if (applyLog)
      stateFeatures = builder.addFunction(mapContainerFunction(lbcppMemberUnaryFunction(SimpleSearchNodeFeatureGenerator, computeLogFunction, doubleType, doubleType)), stateFeatures, T("logS"));

    builder.addFunction(cartesianProductFeatureGenerator(true), nodeFeatures, stateFeatures, T("prod"));
  }

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
  EvaluateHIVSearchHeuristic(DecisionProblemPtr problem, const FunctionPtr& featuresFunction, const ContainerPtr& initialStates, size_t maxSearchNodes, size_t maxHorizon, double discount)
    : SimpleUnaryFunction(TypePtr(), doubleType), problem(problem), featuresFunction(featuresFunction), initialStates(initialStates),
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
    SearchPolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);

    double res = 0.0;
    size_t n = initialStates->getNumElements();
    if (!n)
      return res;
    for (size_t i = 0; i < n; ++i)
    {
      DecisionProblemStatePtr state = initialStates->getElement(i).clone(context).getObjectAndCast<DecisionProblemState>();
      jassert(state);
      for (size_t t = 0; t < maxHorizon; ++t)
      {
        SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
        searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
        Variable action = searchTree->getBestAction();

        double reward = 0.0;
        state->performTransition(context, action, reward);
        res += reward * pow(discount, (double)t);
      }
    }
    return -res / (double)n;
  }

protected:
  DecisionProblemPtr problem;
  FunctionPtr featuresFunction;
  ContainerPtr initialStates;
  size_t maxSearchNodes;
  size_t maxHorizon;
  double discount;
};

class HIVSandBox : public WorkUnit
{
public:
  // default values
  HIVSandBox() :  discount(0.98), minDepth(0), maxDepth(10), depthIsBudget(false), maxHorizon(300), numInitialStates(1),
                  verboseTrajectories(false), computeGeneralization(false),
                  iterations(100), populationSize(100), numBests(10), reinjectBest(false),
                  baseHeuristics(false), optimisticHeuristics(false), learnedHeuristic(false)
  {
    problem = hivDecisionProblem();
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
    if (!featuresFunction->initialize(context, searchTreeNodeClass(problem->getStateClass(), problem->getActionType())))
      return false;
      

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
    if (!depthIsBudget)
    {
      for (size_t depth = 0; depth < minDepth; ++depth)
        maxSearchNodes = maxSearchNodes * fixedNumberOfActions + 1; // any general formula for that ?
    }

    // sample initial states
    ContainerPtr initialStates = problem->sampleInitialStates(context, numInitialStates);

    // load heuristics
    std::vector<FunctionPtr> loadedHeuristics;
    if (this->loadedHeuristics.existsAsFile() && !loadHeuristicsFromAlaRacheTextFile(context, this->loadedHeuristics, loadedHeuristics))
      return false;


    // iterator over all possible depth values
    for (size_t depth = minDepth; depth <= maxDepth; ++depth)
    {
      if (depthIsBudget)
      {
        maxSearchNodes = depth;
        if (depth >= 10)
          depth += 4; // go five by five
        context.enterScope(T("N = ")+ String((int)maxSearchNodes));
      }
      else
        context.enterScope(T("D = ") + String((int)depth) + T(" N = ")+ String((int)maxSearchNodes));

      context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
      if (maxSearchNodes)
      {
        if (!depthIsBudget)
          context.resultCallback(T("depth"), depth);

        if (baseHeuristics)
        {
          // BASELINES
          computeTrajectory(context, problem, initialStates, greedySearchHeuristic(), T("maxReward"), maxSearchNodes);
          computeTrajectory(context, problem, initialStates, greedySearchHeuristic(discount), T("maxDiscountedReward"), maxSearchNodes);
          computeTrajectory(context, problem, initialStates, maxReturnSearchHeuristic(), T("maxReturn"), maxSearchNodes);
          computeTrajectory(context, problem, initialStates, minDepthSearchHeuristic(), T("minDepth"), maxSearchNodes);
        }

        if (loadedHeuristics.size() && depth < loadedHeuristics.size())
        {
          // loaded heuristic
          computeTrajectory(context, problem, initialStates, loadedHeuristics[depth], T("loadedHeuristic"), maxSearchNodes);
        }

        if (optimisticHeuristics)
        {
          if (problem.isInstanceOf<HIVDecisionProblem>())
          {
            for (int i = 3; i <= 15; i += 3)
              computeTrajectory(context, problem, initialStates, optimisticPlanningSearchHeuristic(discount, pow(10.0, (double)i)), T("optimistic(10^") + String(i) + T(")"), maxSearchNodes);
          }
          else
            computeTrajectory(context, problem, initialStates, optimisticPlanningSearchHeuristic(discount), T("optimistic"), maxSearchNodes);
        }

        if (learnedHeuristic)
        {
          FunctionPtr optimizedHeuristic = optimizeLookAHeadTreePolicy(context, initialStates, maxSearchNodes);
          context.resultCallback(T("model"), optimizedHeuristic);
          computeTrajectory(context, problem, initialStates, optimizedHeuristic, T("optimized"), maxSearchNodes);        

          if (computeGeneralization)
            computeEDAGeneralization(context, initialStates, maxSearchNodes);
        }
      }
      else
        context.errorCallback(T("No max search nodes"));

      context.leaveScope();
      if (!depthIsBudget)
        maxSearchNodes = maxSearchNodes * fixedNumberOfActions + 1;
    }
    return true;
  }

  bool loadHeuristicsFromAlaRacheTextFile(ExecutionContext& context, const File& textFile, std::vector<FunctionPtr>& res)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(featuresFunction->getOutputType());
    if (!featuresEnumeration)
    {
      context.errorCallback(T("Problem in feature function"));
      return false;
    }

    InputStream* istr = textFile.createInputStream();
    if (!istr)
    {
      context.errorCallback(T("prout"));
      return false;
    }
    while (!istr->isExhausted())
    {
      String line = istr->readNextLine();
      StringArray tokens;
      tokens.addTokens(line, true);

      if (tokens.size() == 0)
        continue;

      DenseDoubleVectorPtr denseDoubleVector = new DenseDoubleVector(featuresEnumeration, doubleType, tokens.size());
      for (int i = 0; i < tokens.size(); ++i)
        denseDoubleVector->setValue(i, tokens[i].getDoubleValue());
      context.resultCallback(T("Loaded heuristic ") + String((int)res.size()), denseDoubleVector);
      res.push_back(new HIVSearchHeuristic(featuresFunction, denseDoubleVector));
    }
    delete istr;
    return true;
  }

private:
  friend class HIVSandBoxClass;
 
  DecisionProblemPtr problem;
  FunctionPtr featuresFunction;
 
  double discount;
  size_t minDepth, maxDepth;
  bool depthIsBudget;
  size_t maxHorizon;
  size_t numInitialStates;
  bool verboseTrajectories;
  bool computeGeneralization;

  size_t iterations;
  size_t populationSize;
  size_t numBests;
  bool reinjectBest;

  bool breadthFirstHeuristic;
  bool baseHeuristics;
  bool optimisticHeuristics;
  bool learnedHeuristic;
  File loadedHeuristics;

  void computeEDAGeneralization(ExecutionContext& context, const ContainerPtr& initialStates, size_t maxSearchNodes) const
  {
    context.enterScope(T("Generalization"));

    for (size_t numTrainingStates = 1; numTrainingStates <= 128; numTrainingStates *= 2)
    {
      context.enterScope(T("Num training states = ") + String((int)numTrainingStates));
      ScalarVariableStatisticsPtr statistics = new ScalarVariableStatistics(T("score"));
      for (size_t j = 0; j < 10; ++j)
      {
        context.enterScope(T("Trial ") + String((int)j));

        ContainerPtr trainingStates = problem->sampleInitialStates(context, numTrainingStates);
        FunctionPtr optimizedHeuristic = optimizeLookAHeadTreePolicy(context, trainingStates, maxSearchNodes);

        context.resultCallback(T("numTrainingStates"), numTrainingStates);
        double score = computeTrajectory(context, problem, initialStates, optimizedHeuristic, T("optimized"), maxSearchNodes);        
        context.resultCallback(T("score"), score);
        statistics->push(score);

        context.leaveScope(score);
      }
      context.resultCallback(T("numTrainingStates"), numTrainingStates);
      context.resultCallback(T("mean"), statistics->getMean());
      context.resultCallback(T("stddev"), statistics->getStandardDeviation());
      context.resultCallback(T("stats"), statistics);
      context.leaveScope(statistics->getMean());
    }

    context.leaveScope(true);
  }

  FunctionPtr optimizeLookAHeadTreePolicy(ExecutionContext& context, const ContainerPtr& trainingStates, size_t maxSearchNodes) const
  {
    // EDA
    FunctionPtr functionToOptimize = new EvaluateHIVSearchHeuristic(problem, featuresFunction, trainingStates, maxSearchNodes, maxHorizon, discount);
    
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(functionToOptimize->getRequiredInputType(0, 1));
    if (!featuresEnumeration)
    {
      context.errorCallback(T("Problem in feature function"));
      return FunctionPtr();
    }
    context.informationCallback(String((int)featuresEnumeration->getNumElements()) + T(" parameters"));
    ClassPtr parametersClass = denseDoubleVectorClass(featuresEnumeration, doubleType);

    if (!functionToOptimize->initialize(context, parametersClass))
      return FunctionPtr();

#if 0 // FIXME: reimplement with samplers
    IndependentDoubleVectorDistributionPtr initialDistribution = new IndependentDoubleVectorDistribution(featuresEnumeration);
    for (size_t i = 0; i < featuresEnumeration->getNumElements(); ++i)
      initialDistribution->setSubDistribution(i, new GaussianDistribution(0.0, 1.0));
  
    //double bestIndividualScore = evaluateEachFeature(context, functionToOptimize, featuresEnumeration);

    Variable bestParameters;
    /*double bestScore = */performEDA(context, functionToOptimize, initialDistribution, bestParameters);
    return new HIVSearchHeuristic(featuresFunction, bestParameters.getObjectAndCast<DenseDoubleVector>());
#endif // 0
    return FunctionPtr();
  }

#if 0 // FIXME: replace by default eda
  double performEDA(ExecutionContext& context, const FunctionPtr& functionToOptimize, const DistributionPtr& initialDistribution, Variable& bestParameters) const
  {
    double bestScore = DBL_MAX;

    DistributionPtr distribution = initialDistribution;
    context.enterScope(T("Optimizing"));
    for (size_t i = 0; i < iterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      Variable bestIterationSolution = bestParameters;
      double score = performEDAIteration(context, functionToOptimize, distribution, bestIterationSolution);
      context.resultCallback(T("bestParameters"), bestIterationSolution);

      //context.resultCallback(T("distribution"), distribution);
      context.leaveScope(score);
      if (score < bestScore)
      {
        bestScore = score;
        bestParameters = bestIterationSolution;
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

    RandomGeneratorPtr random = context.getRandomGenerator();
    
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
#endif // 0

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

#if 0
  struct UnderstandHIVBudget2Policy : public Policy
  {
    UnderstandHIVBudget2Policy(PolicyPtr pol)
      : pol(pol) {}
    PolicyPtr pol;
    
    virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions)
    {
      Variable res = pol->policyStart(context, state, actions);
      jassert(res.getInteger() == 0);
      return res;
    }

    size_t greedyAction;
    size_t secondAction;

    virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions)
    {
      SearchTreePtr tree = state.getObjectAndCast<SearchTree>();
      greedyAction = tree->getRootNode()->getBestChildNode()->getNodeIndex();
      jassert(greedyAction >= 1 && greedyAction <= 4);

      Variable res = pol->policyStep(context, reward, state, actions);
      secondAction = (size_t)res.getInteger();
      jassert(secondAction >= 1 && secondAction <= 4);
      //if (i == greedyAction)
      {
        probZeroG.push(greedyAction == 1 ? 1.0 : 0.0);
        probOneG.push(greedyAction == 2 ? 1.0 : 0.0);
        probTwoG.push(greedyAction == 3 ? 1.0 : 0.0);
        probThreeG.push(greedyAction == 4 ? 1.0 : 0.0);
      }
     // else
      {
        probZero.push(secondAction == 1 ? 1.0 : 0.0);
        probOne.push(secondAction == 2 ? 1.0 : 0.0);
        probTwo.push(secondAction == 3 ? 1.0 : 0.0);
        probThree.push(secondAction == 4 ? 1.0 : 0.0);
      }      
      probGreedy.push(secondAction == greedyAction ? 1.0 : 0.0);
      return res;
    }

    virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState)
    {
      SearchTreePtr searchTree = finalState.getObjectAndCast<SearchTree>();
      
      size_t finalAction = searchTree->getRootNode()->getBestChildNode()->getNodeIndex();
      probUsefull.push(finalAction != greedyAction ? 1.0 : 0.0);
      probSecondAction.push(finalAction == secondAction ? 1.0 : 0.0);

      pol->policyEnd(context, reward, finalState);
    }

    void showResults(ExecutionContext& context)
    {
      context.resultCallback(T("probGreedy"), probGreedy.getMean());
      context.resultCallback(T("probUsefull"), probUsefull.getMean());
      context.resultCallback(T("probZero"), probZero.getMean());
      context.resultCallback(T("probOne"), probOne.getMean());
      context.resultCallback(T("probTwo"), probTwo.getMean());
      context.resultCallback(T("probThree"), probThree.getMean());
      context.resultCallback(T("probZeroG"), probZeroG.getMean());
      context.resultCallback(T("probOneG"), probOneG.getMean());
      context.resultCallback(T("probTwoG"), probTwoG.getMean());
      context.resultCallback(T("probThreeG"), probThreeG.getMean());
      context.resultCallback(T("probSecondAction"), probSecondAction.getMean());
    }

    ScalarVariableMean probGreedy;
    ScalarVariableMean probUsefull;
    ScalarVariableMean probZero;
    ScalarVariableMean probOne;
    ScalarVariableMean probTwo;
    ScalarVariableMean probThree;
    ScalarVariableMean probZeroG;
    ScalarVariableMean probOneG;
    ScalarVariableMean probTwoG;
    ScalarVariableMean probThreeG;
    ScalarVariableMean probSecondAction;
  };
#endif // 0

  double computeTrajectory(ExecutionContext& context, const DecisionProblemPtr& problem, const ContainerPtr& initialStates, const FunctionPtr& heuristic, const String& name, size_t maxSearchNodes) const
  {
    SearchPolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);
    
    //ReferenceCountedObjectPtr<UnderstandHIVBudget2Policy> searchPolicy = new UnderstandHIVBudget2Policy(searchPolicy);

    double res = 0.0;
    size_t n = initialStates->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      DecisionProblemStatePtr state = initialStates->getElement(i).getObject()->cloneAndCast<DecisionProblemState>(context);

      if (verboseTrajectories)
        context.enterScope(T("Trajectory ") + name);
      double returnValue = 0.0;
      for (size_t t = 0; t < maxHorizon; ++t)
      {
        if (verboseTrajectories)
          context.enterScope(T("element ") + String((int)t));

        SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
        searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
        Variable bestAction = searchTree->getBestAction();

        double reward = 0.0;
        state->performTransition(context, bestAction, reward);
        returnValue += reward * pow(discount, (double)t);

        if (verboseTrajectories)
        {
          context.resultCallback(T("time"), t + 1);
          context.resultCallback(T("reward"), reward);
          
          HIVDecisionProblemStatePtr hivState = state.dynamicCast<HIVDecisionProblemState>();
          if (hivState)
          {
            DenseDoubleVectorPtr hivAction = bestAction.dynamicCast<DenseDoubleVector>();

            context.resultCallback(T("RTI"), hivAction->getValue(0) > 0.0);
            context.resultCallback(T("PI"), hivAction->getValue(1) > 0.0);

            for (size_t k = 1; k <= 6; ++k)
            {
              String name = hivState->getVariableName(k);
              double value = hivState->getVariable(k).getDouble();
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

          context.leaveScope(returnValue);
        }
      }
      if (verboseTrajectories)
        context.leaveScope(returnValue);

      res += returnValue;
    }
    res /= (double)n;
    context.resultCallback(name, res);

    //popo->showResults(context);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

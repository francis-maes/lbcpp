/*-----------------------------------------.---------------------------------.
| Filename: GoBoostingSandBox2.h           | Go Boosting Sand Box New version|
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2011 15:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_2_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_2_H_

# include "GoProblem.h"
# include <lbcpp/Learning/LossFunction.h>
# include "../Luape/LuapeGraph.h"
# include "../Luape/LuapeGraphBuilder.h"
# include "../Luape/LuapeProblem.h"
# include "../Luape/LuapeFunction.h"

# include "Perception/GoStatePerception.h"
# include "Perception/GoBoardPositionPerception.h"

# include <queue> // for priority queue in bandits pool

namespace lbcpp
{

class GoRankLuapeProblem : public LuapeProblem
{
public:
  GoRankLuapeProblem()
  {
    addInput(goBoardPositionPerceptionClass, "position");
    addFunction(getVariableFunction(0));
    addFunction(Function::create(getType("StumpFunction")));
    //addFunction( FunctionPtr())
  }
};
/*
class LuapeGraphBuilderBanditEnumerator
{
public:
  LuapeGraphBuilderBanditEnumerator(size_t maxDepth)
    : maxDepth(maxDepth) {}
  virtual ~LuapeGraphBuilderBanditEnumerator() {}

  void addNode(ExecutionContext& context, const LuapeGraphPtr& graph, size_t nodeIndex)
  {
    TypePtr nodeType = graph->getNodeType(nodeIndex);

    // accessor actions
    if (nodeType->inheritsFrom(objectClass))
    {
      std::vector<size_t> arguments(1, nodeIndex);
      size_t nv = nodeType->getNumMemberVariables();
      for (size_t j = 0; j < nv; ++j)
        res->append(new LuapeFunctionNode(getVariableFunction(j), arguments));
    }
    
    // function actions
    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      FunctionPtr function = problem->getFunction(i);
      std::vector<size_t> arguments;
      enumerateFunctionActionsRecursively(function, arguments, res);
    }

    // yield actions
    for (size_t i = 0; i < n; ++i)
      if (graph->getNodeType(i) == booleanType)
        res->append(new LuapeYieldNode(i));
    return res;
  }

protected:
  std::map<LuapeNodeKey, LuapeNodeCachePtr> cache;
};
*/
class LuapeGraphBuilderBanditPool : public Object
{
public:
  LuapeGraphBuilderBanditPool(size_t maxSize, size_t maxDepth)
    : maxSize(maxSize), maxDepth(maxDepth) {}

  size_t getNumArms() const
    {return arms.size();}

  void initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph)
  {
    LuapeRPNGraphBuilderStatePtr builder = new LuapeRPNGraphBuilderState(problem, graph, maxDepth);
    createNewArms(context, builder);
    createBanditsQueue();
  }

  void executeArm(ExecutionContext& context, size_t armIndex,  const LuapeProblemPtr& problem, const LuapeGraphPtr& graph)
  {
    // add nodes into graph
    size_t keyPosition = 0;
    graph->pushNodes(context, arms[armIndex].key, keyPosition, arms[armIndex].key.size());
    LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
    jassert(yieldNode);

    // update arms
    destroyArm(context, armIndex);

    LuapeRPNGraphBuilderStatePtr builder = new LuapeRPNGraphBuilderState(problem, graph, maxDepth);
    double reward;
    builder->performTransition(context, Variable(yieldNode->getArgument()), reward); // push last created node
    createNewArms(context, builder);
    createBanditsQueue();
  }

  // train the weak learner on 50% of data and evaluate on the other 50% of data
  double sampleReward(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals, size_t armIndex) const
  {
    static const double p = 0.5;
    RandomGeneratorPtr random = context.getRandomGenerator();

    ContainerPtr samples = arms[armIndex].cache->getTrainingSamples();
    DenseDoubleVectorPtr scalars = samples.dynamicCast<DenseDoubleVector>();
    if (scalars)
    {
      jassert(false); // not implemented yet: todo: random split or heuristic split
      return 0.0;
    }
    else
    {
      BooleanVectorPtr booleans = samples.dynamicCast<BooleanVector>();
      jassert(booleans);
      size_t n = booleans->getNumElements();
      jassert(n == pseudoResiduals->getNumValues());

      ScalarVariableMeanAndVariance trainPositive;
      ScalarVariableMeanAndVariance validationPositive;
      ScalarVariableMeanAndVariance trainNegative;
      ScalarVariableMeanAndVariance validationNegative;

      for (size_t i = 0; i < n; ++i)
      {
        bool isPositive = booleans->get(i);

        if (random->sampleBool(p))
          (isPositive ? trainPositive : trainNegative).push(pseudoResiduals->getValue(i));
        else
          (isPositive ? validationPositive : validationNegative).push(pseudoResiduals->getValue(i));
      }
      
      double meanSquareError = 0.0;
      if (validationPositive.getCount())
        meanSquareError += validationPositive.getCount() * (trainPositive.getSquaresMean() + validationPositive.getSquaresMean() 
                                                              - 2 * trainPositive.getMean() * validationPositive.getMean());
      if (validationNegative.getCount())
        meanSquareError += validationNegative.getCount() * (trainNegative.getSquaresMean() + validationNegative.getSquaresMean()
                                                              - 2 * trainNegative.getMean() * validationNegative.getMean()); 
      if (validationPositive.getCount() || validationNegative.getCount())
        meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
      
      return 1.0 - meanSquareError;
    }
  }

  void playArmWithHighestIndex(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals)
  {
    size_t armIndex = banditsQueue.top().first;
    banditsQueue.pop();
    
    Arm& arm = arms[armIndex];
    ++arm.playedCount;
    arm.rewardSum += sampleReward(context, pseudoResiduals, armIndex);

    banditsQueue.push(std::make_pair(armIndex, arm.getIndexScore()));
  }

  size_t getArmWithHighestReward() const
  {
    double bestReward = -DBL_MAX;
    size_t bestArm = (size_t)-1;
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double reward = arms[i].getMeanReward();
      if (reward > bestReward)
        bestReward = reward, bestArm = i;
    }
    return bestArm;
  }

  void displayInformation(ExecutionContext& context)
  {
    std::multimap<double, size_t> armsByMeanReward;
    for (size_t i = 0; i < arms.size(); ++i)
      armsByMeanReward.insert(std::make_pair(arms[i].getMeanReward(), i));

    size_t n = 10;
    size_t i = 1;
    for (std::multimap<double, size_t>::reverse_iterator it = armsByMeanReward.rbegin(); i < n && it != armsByMeanReward.rend(); ++it, ++i)
    {
      Arm& arm = arms[it->second];
      context.informationCallback(T("[") + String((int)i) + T("] r = ") + String(arm.getMeanReward())
        + T(" t = ") + String(arm.playedCount) + T(" -- ") + arm.description);
    }

    if (armsByMeanReward.size())
    {
      Arm& bestArm = arms[armsByMeanReward.rbegin()->second];
      context.resultCallback(T("bestArmReward"), bestArm.getMeanReward());
      context.resultCallback(T("bestArmPlayCount"), bestArm.playedCount);
    }
    context.resultCallback(T("banditPoolSize"), arms.size());
  }

  void clearSamples()
  {
    for (size_t i = 0; i < arms.size(); ++i)
      if (arms[i].cache)
        arms[i].cache->clearSamples();
  }

protected:
  size_t maxSize;
  size_t maxDepth;

  struct Arm
  {
    Arm(LuapeNodeCachePtr cache = LuapeNodeCachePtr())
      : playedCount(0), rewardSum(0.0), cache(cache) {}

    size_t playedCount;
    double rewardSum;
    LuapeNodeCachePtr cache;
    LuapeNodeKey key;
    String description;

    double getIndexScore() const
      {return playedCount ? rewardSum + 2.0 / (double)playedCount : DBL_MAX;}

    double getMeanReward() const
      {return playedCount ? rewardSum / (double)playedCount : 0.0;}
  };

  std::vector<Arm> arms;
  std::vector<size_t> destroyedArmIndices;

  typedef std::map<LuapeNodeKey, size_t> KeyToArmIndexMap;
  KeyToArmIndexMap keyToArmIndex;

  struct BanditScoresComparator
  {
    bool operator()(const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      if (left.second != right.second)
        return left.second < right.second;
      else
        return left.first < right.first;
    }
  };
  
  typedef std::priority_queue<std::pair<size_t, double>, std::vector<std::pair<size_t, double> >, BanditScoresComparator  > BanditsQueue;
  BanditsQueue banditsQueue;

  void createBanditsQueue()
  {
    banditsQueue = BanditsQueue();
    for (size_t i = 0; i < arms.size(); ++i)
      if (arms[i].cache)
        banditsQueue.push(std::make_pair(i, arms[i].getIndexScore()));
  }

  size_t createArm(ExecutionContext& context, const LuapeNodeKey& key, const LuapeNodeCachePtr& cache, const String& description)
  {
    jassert(cache);
    size_t index;
    if (destroyedArmIndices.size())
    {
      index = destroyedArmIndices.back();
      destroyedArmIndices.pop_back();
    }
    else
    {
      index = arms.size();
      arms.push_back(Arm());
    }
    Arm& arm = arms[index];
    arm.cache = cache;
    arm.key = key;
    arm.description = description;
    keyToArmIndex[key] = index;
    return index;
  }

  void destroyArm(ExecutionContext& context, size_t index)
  {
    arms[index] = Arm();
    destroyedArmIndices.push_back(index);
    banditsQueue = BanditsQueue();
  }

  void createNewArms(ExecutionContext& context, LuapeRPNGraphBuilderStatePtr state)
  {
    if (state->isFinalState())
    {
      LuapeGraphPtr graph = state->getGraph();
      LuapeYieldNodePtr yield = graph->getLastNode().dynamicCast<LuapeYieldNode>();
      if (yield)
      {
        LuapeNodeKey key;
        yield->fillKey(graph->getNodes(), key);
        if (keyToArmIndex.find(key) == keyToArmIndex.end())
          createArm(context, key, graph->getNode(yield->getArgument())->getCache(), yield->toShortString());
      }
    }
    else
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable stateBackup;
        double reward;
        state->performTransition(context, actions->getElement(i), reward, &stateBackup);
        context.enterScope(state->toShortString());
        createNewArms(context, state);
        context.leaveScope();
        state->undoTransition(context, stateBackup);
      }
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderBanditPool> LuapeGraphBuilderBanditPoolPtr;

class GoBoostingSandBox2 : public WorkUnit
{
public:
  GoBoostingSandBox2() : maxCount(0) {}

  LuapeRankerPtr createRanker(ExecutionContext& context, const LuapeProblemPtr& problem) const
  {
    LuapeRankerPtr ranker = new LuapeRanker();
    if (!ranker->initialize(context, objectVectorClass(goBoardPositionPerceptionClass), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)))
      return LuapeRankerPtr();

    // initialize graph
    LuapeGraphPtr graph = problem->createInitialGraph(context);
    graph->pushNode(context, new LuapeFunctionNode(getVariableFunction(0), 0)); // retrieve board from position
    graph->pushNode(context, new LuapeFunctionNode(getVariableFunction(0), 1)); // retrieve state from board
    ranker->setGraph(graph);    
   
    // initialize evaluator
    EvaluatorPtr evaluator = new GoActionScoringEvaluator();
    //evaluator->setUseMultiThreading(true);
    ranker->setEvaluator(evaluator);
    return ranker;
  }

  bool learnRanker(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeRankerPtr& ranker, const ContainerPtr& trainingGames, const ContainerPtr& testingGames) const
  {
    LuapeGraphBuilderBanditPoolPtr pool = new LuapeGraphBuilderBanditPool(100, 7);

    size_t n = trainingGames->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateAndTrajectory = trainingGames->getElement(i).getObjectAndCast<Pair>();
      jassert(stateAndTrajectory);
      GoStatePtr state = stateAndTrajectory->getFirst().getObjectAndCast<GoState>();
      ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();

      if (!doLearningEpisode(context, problem, ranker, state, trajectory, pool))
        return false;
    }
    return true;
  }

  void addRankingExampleToGraph(ExecutionContext& context, const LuapeGraphPtr& graph, const PairPtr& rankingExample, bool isTrainingExample) const
  {
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();
    size_t firstIndex = graph->getNumTrainingSamples();
    graph->resizeSamples(firstIndex + n, graph->getNumValidationSamples());
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<Variable> sample(1, alternatives->getElement(i));
      graph->setSample(true, firstIndex + i, sample);
    }
  }

  DenseDoubleVectorPtr computePseudoResiduals(ExecutionContext& context, const LuapeRankerPtr& ranker, const DenseDoubleVectorPtr& predictions, const std::vector<PairPtr>& rankingExamples) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(predictions->getNumValues(), 0.0);

    RankingLossFunctionPtr rankingLoss = allPairsRankingLossFunction(hingeDiscriminativeLossFunction());
    size_t index = 0;
    for (size_t i = 0; i < rankingExamples.size(); ++i)
    {
      size_t n = rankingExamples[i]->getFirst().getObjectAndCast<Container>()->getNumElements();
      DenseDoubleVectorPtr costs = rankingExamples[i]->getSecond().getObjectAndCast<DenseDoubleVector>();

      DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
      memcpy(scores->getValuePointer(0), predictions->getValuePointer(index), sizeof (double) * n);

      double lossValue = 0.0;
      DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(n, 0.0);
      rankingLoss->computeRankingLoss(scores, costs, &lossValue, &lossGradient, 1.0);
      jassert(lossGradient);
      
      memcpy(res->getValuePointer(index), lossGradient->getValuePointer(0), sizeof (double) * n);
      index += n;
    }

    res->multiplyByScalar(-1.0);
    return res;
  }

  bool doLearningEpisode(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeRankerPtr& ranker, const GoStatePtr& state, const ContainerPtr& trajectory, const LuapeGraphBuilderBanditPoolPtr& pool) const
  {
    LuapeGraphPtr graph = ranker->getGraph();
    graph->clearSamples();
    pool->clearSamples();
    
    // 1- fill graph
    size_t numSteps = trajectory->getNumElements();
    std::vector<PairPtr> rankingExamples;
    rankingExamples.resize(numSteps);
    context.enterScope("Filling graph");
    for (size_t i = 0; i < numSteps; ++i)
    {
      jassert(!state->isFinalState());
      Variable correctAction = trajectory->getElement(i);
      PairPtr rankingExample = makeRankingExample(context, state, correctAction);
      rankingExamples[i] = rankingExample;
      addRankingExampleToGraph(context, graph, rankingExample, true);
  
      double reward;
      state->performTransition(context, correctAction, reward);
      context.progressCallback(new ProgressionState(i+1, numSteps, T("Steps")));
    }
    context.leaveScope();

    // 2- compute graph outputs and compute loss derivatives
    context.enterScope(T("Computing predictions"));
    DenseDoubleVectorPtr predictions = ranker->computeSamplePredictions(context, true);
    context.leaveScope();
    context.enterScope(T("Computing pseudo-residuals"));
    DenseDoubleVectorPtr pseudoResiduals = computePseudoResiduals(context, ranker, predictions, rankingExamples);   
    context.leaveScope();

    // 3- play bandits
    if (pool->getNumArms() == 0)
    {
      context.enterScope(T("Create arms"));
      pool->initialize(context, problem, ranker->getGraph());
      context.leaveScope();
    }

    for (size_t t = 0; t < 10; ++t)
    {
      context.enterScope(T("Playing bandits iteration ") + String((int)t));
      for (size_t i = 0; i < pool->getNumArms(); ++i)
        pool->playArmWithHighestIndex(context, pseudoResiduals);
      pool->displayInformation(context);
      context.leaveScope();
    }

    // 4- select weak learner and update everything
    pool->executeArm(context, pool->getArmWithHighestReward(), problem, graph);

    // 5- optimize weight of new weak learner
    double weight = 0.0; // FIXME
    ranker->getVotes()->append(weight);
    return true;
  }

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Loading training games from ") + context.getFilePath(trainingFile));
    ContainerPtr trainingGames = loadSGFTrajectories(context, trainingFile, maxCount);
    context.leaveScope((trainingGames ? trainingGames->getNumElements() : 0));
    
    context.enterScope(T("Loading testing games from ") + context.getFilePath(testingFile));
    ContainerPtr testingGames = loadSGFTrajectories(context, testingFile, maxCount);
    context.leaveScope((testingGames ? testingGames->getNumElements() : 0));

    if (!printGamesInfo(context, trainingGames, T("training")) ||
        !printGamesInfo(context, testingGames, T("testing")))
      return false;

    LuapeProblemPtr problem = new GoRankLuapeProblem();

    LuapeRankerPtr ranker = createRanker(context, problem);
    if (!ranker)
      return false;

    if (!learnRanker(context, problem, ranker, trainingGames, testingGames))
      return false;

#if 0

    // iterate over sgf files
    //   open and play file
    //     for each step: do 100 bandit steps
    //   add best weak learner
    //   evaluate on validation data

    context.enterScope(T("Loading examples"));
    ContainerPtr examples = loadExamples(context, examplesDirectory, maxCount);
    context.leaveScope(examples ? examples->getNumElements() : 0);
    if (!examples)
      return false;
    
    // initialize graph
    LuapeProblemPtr problem = new GoRankLuapeProblem();
    LuapeGraphPtr graph = problem->createInitialGraph(context);

    // put data inside the graph
    PairPtr example = examples->getElement(0).getObjectAndCast<Pair>();

    GoStatePtr state = example->getFirst().getObjectAndCast<GoState>();
    GoStatePerceptionPtr statePerception = new GoStatePerception(state);
    GoBoardPerceptionPtr boardPerception = statePerception->getBoard();

    ContainerPtr availableActions = state->getAvailableActions();
    size_t n = availableActions->getNumElements();
    graph->resizeSamples(n, 0);
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = availableActions->getElement(i);
      GoState::Position position = action.getObjectAndCast<PositiveIntegerPair>()->getValue();
      std::vector<Variable> example(1, boardPerception->getPosition(position));
      graph->setSample(true, i, example);
    }

    // initial processing
    graph->pushNode(context, new LuapeFunctionNode(getVariableFunction(0), 0)); // retrieve board from position
    graph->pushNode(context, new LuapeFunctionNode(getVariableFunction(0), 1)); // retrieve state from board



    //OptimizerPtr optimizer = new NestedMonteCarloOptimizer(2, 1);

  //  OptimizerPtr optimizer = new SinglePlayerMCTSOptimizer(budgetPerIteration);
  
//    LuapeWeakLearnerPtr weakLearner = combinedStumpWeakLearner();
    //productWeakLearner(singleStumpWeakLearner(), 2);
      //luapeGraphBuilderWeakLearner(optimizer, maxSteps);
    //classifier->setBatchLearner(adaBoostMHLuapeLearner(problem, weakLearner, maxIterations));
    ranker->setEvaluator(defaultSupervisedEvaluator());

    //classifier->train(context, trainData, testData, T("Training"), true);
    //classifier->evaluate(context, trainData, EvaluatorPtr(), T("Evaluating on training data"));
    //classifier->evaluate(context, testData, EvaluatorPtr(), T("Evaluating on testing data"));


//    PairPtr rankingExample = makeRankingExample(context, example->getFirst().getObjectAndCast<GoState>(), example->getSecond());

    LuapeRPNGraphBuilderStatePtr builder = new LuapeRPNGraphBuilderState(problem, graph, 8);

    std::map<LuapeNodeKey, LuapeGraphPtr> finalStates;
    enumerateFinalStates(context, builder, finalStates);
    context.enterScope(T("Final states"));

    for (std::map<LuapeNodeKey, LuapeGraphPtr>::const_iterator it = finalStates.begin(); it != finalStates.end(); ++it)
    {
      LuapeGraphPtr graph = it->second;
      context.informationCallback(graph->getLastNode()->toShortString());
    }

    context.leaveScope(finalStates.size());
#endif // 0
    return true;

  /*
    ContainerPtr rankingExamples = makeRankingExamples(context, examples);

    // make train/test split
    ContainerPtr trainingExamples = rankingExamples->invFold(0, 10);
    ContainerPtr testingExamples = rankingExamples->fold(0, 10);

    // learn model with rankboost
    BoostingStrongModelPtr model = new RankingBoostingStrongModel();
    model->setBatchLearner(new RankBoostLearner(20));
    model->setEvaluator(new GoActionScoringEvaluator());
    model->train(context, trainingExamples, testingExamples, "Boosting", true);
    return true;*/
  }

private:
  friend class GoBoostingSandBox2Class;

  File trainingFile;
  File testingFile;
  size_t maxCount;

  ContainerPtr loadExamples(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    StreamPtr fileStream = directoryFileStream(context, directory);
    return fileStream->load(maxCount)->apply(context, loadFromFileFunction(pairClass(goStateClass, positiveIntegerPairClass)), Container::parallelApply);
  }

  ContainerPtr makeRankingExamples(ExecutionContext& context, const ContainerPtr& stateActionPairs) const
  {
    GoActionsPerceptionPtr perception = new GoActionsPerception(19);
    if (!perception->initialize(context, goStateClass, containerClass(positiveIntegerPairClass)))
      return false;

    context.enterScope("Make ranking examples");
    size_t n = stateActionPairs->getNumElements();
    ObjectVectorPtr rankingExamples = new ObjectVector(pairClass(doubleVectorClass(), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)), n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateActionPair = stateActionPairs->getElement(i).getObjectAndCast<Pair>();
      GoStatePtr state = stateActionPair->getFirst().getObjectAndCast<GoState>();
      Variable correctAction = stateActionPair->getSecond();
      rankingExamples->set(i, makeRankingExample(context, state, correctAction));

      context.progressCallback(new ProgressionState(i+1, n, "Examples"));
    }
    context.leaveScope();
    return rankingExamples;
  }

  PairPtr makeRankingExample(ExecutionContext& context, const GoStatePtr& state, const Variable& correctAction) const
  {
    GoStatePerceptionPtr statePerception = new GoStatePerception(state);
    GoBoardPerceptionPtr boardPerception = statePerception->getBoard();

    ContainerPtr availableActions = state->getAvailableActions();
    size_t n = availableActions->getNumElements();

    ObjectVectorPtr alternatives = new ObjectVector(goBoardPositionPerceptionClass, n);
    DenseDoubleVectorPtr costs(new DenseDoubleVector(n, 0.0));

    bool actionFound = false;
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = availableActions->getElement(i);
      GoState::Position position = action.getObjectAndCast<PositiveIntegerPair>()->getValue();

      alternatives->set(i, boardPerception->getPosition(position));
      if (!actionFound && action == correctAction)
      {
        costs->setValue(i, -1);
        actionFound = true;
      }
    }
    if (!actionFound)
      context.warningCallback("Could not find correct action in available actions");
  
    return new Pair(alternatives, costs);
  }

  bool printGamesInfo(ExecutionContext& context, const ContainerPtr& games, const String& name) const
  {
    if (!games || !games->getNumElements())
    {
      context.errorCallback(T("No ") + name + T(" games"));
      return false;
    }

    size_t n = games->getNumElements();
    size_t moves = 0;
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateAndTrajectory = games->getElement(i).getObjectAndCast<Pair>();
      jassert(stateAndTrajectory);
      moves += stateAndTrajectory->getSecond().getObjectAndCast<Container>()->getNumElements();
    }
    context.informationCallback(String((int)n) + T(" ") + name + T(" games and ") + String((int)moves) + T(" ") + name + T(" moves"));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_2_H_

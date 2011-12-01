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
# include "../Luape/BanditPoolWeakLearner.h"
# include "../Luape/PolicyBasedWeakLearner.h"
# include "../Luape/GradientBoostingLearner.h"

# include "Perception/GoStatePerception.h"
# include "Perception/GoBoardPositionPerception.h"

namespace lbcpp
{

class GoBoostingSandBox2 : public WorkUnit
{
public:
  GoBoostingSandBox2() : maxCount(0), maxDepth(6), budget(1000), numIterations(100) {}

  virtual Variable run(ExecutionContext& context)
  {
    static const double learningRate = 1.0;


    // loading games
    context.enterScope(T("Loading training games from ") + context.getFilePath(trainingFile));
    ContainerPtr trainingGames = loadSGFTrajectories(context, trainingFile, maxCount);
    context.leaveScope((trainingGames ? trainingGames->getNumElements() : 0));
    context.enterScope(T("Loading testing games from ") + context.getFilePath(testingFile));
    ContainerPtr testingGames = loadSGFTrajectories(context, testingFile, maxCount);
    context.leaveScope((testingGames ? testingGames->getNumElements() : 0));

    if (!printGamesInfo(context, trainingGames, T("training")) ||
        !printGamesInfo(context, testingGames, T("testing")))
      return false;

    // create problem and ranker
    LuapeInferencePtr learningMachine = createLearningMachine(context);
    if (!learningMachine)
      return false;

    // configure gradient boosting
    jassert(false);
    //LuapeLearnerPtr learner = new LuapeBanditPoolGBLearner(1.0, 1000, maxDepth);
//    PolicyPtr policy = new TreeBasedRandomPolicy();
    //PolicyPtr policy = new RandomPolicy();
    //PolicyPtr policy = new LuapeRewardStorageBasedPolicy();
    
    LuapeLearnerPtr learner; // FIXME
    //LuapeLearnerPtr learner = l2BoostingLearner(policyBasedWeakLearner(policy, budget, maxDepth), learningRate);
    if (!learner->initialize(context, learningMachine))
      return false;
    
    // learn
    if (!learn(context, learner, trainingGames, testingGames))
      return false;

    return true;
  }

  LuapeInferencePtr createLearningMachine(ExecutionContext& context) const
  {
    //LuapeInferencePtr res = new LuapeRanker();
    //if (!res->initialize(context, objectVectorClass(goBoardPositionPerceptionClass), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)))
    //  return LuapeInferencePtr();

    LuapeInferencePtr res = new LuapeRegressor();

    res->addInput(goBoardPositionPerceptionClass, "position");
    res->addFunction(getVariableLuapeFunction());
    res->addFunction(andBooleanLuapeFunction());
    res->addFunction(equalsConstantEnumLuapeFunction());
    res->addFunction(new GoBoardPositionRelationLuapeFunction());

    if (!res->initialize(context, goBoardPositionPerceptionClass, doubleType))
      return LuapeInferencePtr();

    /* initialize graph
    LuapeGraphPtr graph = problem->createInitialGraph(context);
    LuapeNodePtr positionNode = graph->getNode(0);
    //LuapeNodePtr boardNode = graph->pushFunctionNode(context, getVariableFunction(0), positionNode); // retrieve board from position
    //LuapeNodePtr stateNode = graph->pushFunctionNode(context, getVariableFunction(0), boardNode); // retrieve state from board
    res->setGraph(graph);    */
   
    // initialize evaluator
    EvaluatorPtr evaluator = new GoActionScoringEvaluator();
    //evaluator->setUseMultiThreading(true);
    res->setEvaluator(evaluator);
    return res;
  }

  bool learn(ExecutionContext& context, const LuapeLearnerPtr& learner, const ContainerPtr& trainingGames, const ContainerPtr& testingGames) const
  {
/*    size_t n = trainingGames->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      context.enterScope("Iteration " + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
      std::vector<PairPtr> rankingExamples;
      makeRankingExamples(context, trainingGames->getElement(i), rankingExamples);
      bool ok = true;
      if (!rankingExamples.size())
        context.warningCallback(T("No ranking examples"));
      else
        ok = learner->doLearningEpisode(context, *(std::vector<ObjectPtr>* )&rankingExamples);
      context.leaveScope(ok);
      if (!ok)
        break;
    }*/
    std::vector<PairPtr> rankingExamples;
    size_t n = trainingGames->getNumElements();
    for (size_t i = 0; i < n; ++i)
      makeRankingExamples(context, trainingGames->getElement(i), rankingExamples);

    learner->setExamples(context, true, *(std::vector<ObjectPtr>* )&rankingExamples);
    context.enterScope(T("Gradient Boosting"));
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope("Iteration " + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
      bool ok = learner->doLearningIteration(context);
      context.leaveScope(ok);
      if (!ok)
        break;
    }
    context.leaveScope();
    return true;
  }

private:
  friend class GoBoostingSandBox2Class;

  File trainingFile;
  File testingFile;
  size_t maxCount;
  size_t maxDepth;
  size_t budget;
  size_t numIterations;

  void makeRankingExamples(ExecutionContext& context, const Variable& stateAndTrajectory, std::vector<PairPtr>& res) const
  {
    PairPtr stateAndTrajectoryPair = stateAndTrajectory.getObjectAndCast<Pair>();
    if (!stateAndTrajectoryPair)
      return;

    GoStatePtr state = stateAndTrajectoryPair->getFirst().getObjectAndCast<GoState>();
    ContainerPtr trajectory = stateAndTrajectoryPair->getSecond().getObjectAndCast<Container>();
    
    size_t numSteps = trajectory->getNumElements();
    res.reserve(res.size() + numSteps);
    context.enterScope("Making trajectory");
    for (size_t i = 0; i < numSteps; ++i)
    {
      jassert(!state->isFinalState());
      Variable correctAction = trajectory->getElement(i);
      PairPtr rankingExample = makeRankingExample(context, state, correctAction);
      
      // res.push_back(rankingExample);

      // regression data
      ContainerPtr alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
      DenseDoubleVectorPtr costs = rankingExample->getSecond().getObjectAndCast<DenseDoubleVector>();
      for (size_t j = 0; j < alternatives->getNumElements(); ++j)
      {
        if (costs->getValue(j) == -1.0)
          res.push_back(new Pair(alternatives->getElement(j), 1.0));
        else if (context.getRandomGenerator()->sampleBool(0.01))
          res.push_back(new Pair(alternatives->getElement(j), -1.0));
      }
  
      double reward;
      state->performTransition(context, correctAction, reward);
      context.progressCallback(new ProgressionState(i+1, numSteps, T("Steps")));
    }
    context.leaveScope();
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

      GoBoardPositionPerceptionPtr positionPerception = boardPerception->getPosition(position)->cloneAndCast<GoBoardPositionPerception>();
      positionPerception->setPrevious(statePerception->getLastAction());
      
      alternatives->set(i, positionPerception);
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

  ContainerPtr loadExamples(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    StreamPtr fileStream = directoryFileStream(context, directory);
    return fileStream->load(maxCount)->apply(context, loadFromFileFunction(pairClass(goStateClass, positiveIntegerPairClass)), Container::parallelApply);
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
      if (stateAndTrajectory)
        moves += stateAndTrajectory->getSecond().getObjectAndCast<Container>()->getNumElements();
    }
    context.informationCallback(String((int)n) + T(" ") + name + T(" games and ") + String((int)moves) + T(" ") + name + T(" moves"));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_2_H_
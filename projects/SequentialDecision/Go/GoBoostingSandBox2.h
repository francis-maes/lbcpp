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
# include "../Luape/LuapeGradientBoosting.h"

# include "Perception/GoStatePerception.h"
# include "Perception/GoBoardPositionPerception.h"

namespace lbcpp
{

class GoRankLuapeProblem : public LuapeProblem
{
public:
  GoRankLuapeProblem()
  {
    addInput(goBoardPositionPerceptionClass, "position");
    addFunction(getVariableFunction(0));
    //addFunction(Function::create(getType("StumpFunction")));
    //addFunction( FunctionPtr())
  }
};

class GoBoostingSandBox2 : public WorkUnit
{
public:
  GoBoostingSandBox2() : maxCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
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
    LuapeProblemPtr problem = new GoRankLuapeProblem();
    LuapeRankerPtr ranker = createRanker(context, problem);
    if (!ranker)
      return false;

    // configure gradient boosting
    LuapeGradientBoostingLossPtr gbmLoss = new RankingGradientBoostingLoss(allPairsRankingLossFunction(hingeDiscriminativeLossFunction()));
    LuapeGradientBoostingLearnerPtr learner = new LuapeGradientBoostingLearner(gbmLoss, 0.1, 1000, 6);
    if (!learner->initialize(context, problem, ranker))
      return false;
    
    // learn
    if (!learnRanker(context, learner, trainingGames, testingGames))
      return false;

    return true;
  }

  LuapeRankerPtr createRanker(ExecutionContext& context, const LuapeProblemPtr& problem) const
  {
    LuapeRankerPtr ranker = new LuapeRanker();
    if (!ranker->initialize(context, objectVectorClass(goBoardPositionPerceptionClass), denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)))
      return LuapeRankerPtr();

    // initialize graph
    LuapeGraphPtr graph = problem->createInitialGraph(context);
    LuapeNodePtr positionNode = graph->getNode(0);
    LuapeNodePtr boardNode = graph->pushFunctionNode(context, getVariableFunction(0), positionNode); // retrieve board from position
    LuapeNodePtr stateNode = graph->pushFunctionNode(context, getVariableFunction(0), boardNode); // retrieve state from board
    ranker->setGraph(graph);    
    ranker->setVotes(ranker->createVoteVector(0));
   
    // initialize evaluator
    EvaluatorPtr evaluator = new GoActionScoringEvaluator();
    //evaluator->setUseMultiThreading(true);
    ranker->setEvaluator(evaluator);
    return ranker;
  }

  bool learnRanker(ExecutionContext& context, const LuapeGradientBoostingLearnerPtr& learner, const ContainerPtr& trainingGames, const ContainerPtr& testingGames) const
  {
    size_t n = trainingGames->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<PairPtr> rankingExamples;
      makeRankingExamples(context, trainingGames->getElement(i), rankingExamples);
      context.enterScope("Iteration " + String((int)i + 1));
      bool ok = learner->doLearningEpisode(context, *(std::vector<ObjectPtr>* )&rankingExamples);
      context.leaveScope(ok);
      if (!ok)
        return false;
    }
    return true;
  }

private:
  friend class GoBoostingSandBox2Class;

  File trainingFile;
  File testingFile;
  size_t maxCount;

  void makeRankingExamples(ExecutionContext& context, const Variable& stateAndTrajectory, std::vector<PairPtr>& res) const
  {
    PairPtr stateAndTrajectoryPair = stateAndTrajectory.getObjectAndCast<Pair>();
    jassert(stateAndTrajectoryPair);
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
      res.push_back(rankingExample);

      res[i] = rankingExample;
  
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
      jassert(stateAndTrajectory);
      moves += stateAndTrajectory->getSecond().getObjectAndCast<Container>()->getNumElements();
    }
    context.informationCallback(String((int)n) + T(" ") + name + T(" games and ") + String((int)moves) + T(" ") + name + T(" moves"));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_2_H_

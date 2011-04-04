/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "GoProblem.h"
# include "LoadSGFFileFunction.h"
# include "GoActionsPerception.h"
# include "GoSupervisedEpisode.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class GoActionScoringScoreObject : public ScoreObject
{
public:
  GoActionScoringScoreObject() 
    : predictionRate(new ScalarVariableMean(T("predictionRate"))), 
      rankOfAction(new ScalarVariableStatistics(T("rankOfAction"))),
      unsupervisedRate(new ScalarVariableMean(T("unsupervisedRate"))) {}

  static int getRank(const std::multimap<double, size_t>& sortedScores, size_t index)
  {
    int res = 0;
    for (std::multimap<double, size_t>::const_iterator it = sortedScores.begin(); it != sortedScores.end(); ++it, ++res)
      if (it->second == index)
        return res;
    return -1;
  }

  bool add(ExecutionContext& context, const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs)
  {
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < scores->getNumElements(); ++i)
      sortedScores.insert(std::make_pair(-(scores->getValue(i)), i));
    
    if (sortedScores.empty())
    {
      context.errorCallback(T("No scores"));
      return false;
    }

    // prediction rate
    size_t selectedAction = sortedScores.begin()->second;
    predictionRate->push(costs->getValue(selectedAction) < 0 ? 1.0 : 0.0);

    // rank of selected action
    size_t index = costs->getIndexOfMinimumValue();
    if (index >= 0 && costs->getValue(index) < 0)
    {
      int rank = getRank(sortedScores, index);
      if (rank >= 0)
      {
        rankOfAction->push((double)rank);
        unsupervisedRate->push(0.0);
      }
      else
        unsupervisedRate->push(1.0);
    }
    else
      unsupervisedRate->push(1.0);
    return true;
  }

  virtual double getScoreToMinimize() const
    //{return 1.0 - predictionRate->getMean();} // prediction error
    {return rankOfAction->getMean();} // mean rank of best action

private:
  friend class GoActionScoringScoreObjectClass;

  ScalarVariableMeanPtr predictionRate;
  ScalarVariableStatisticsPtr rankOfAction;
  ScalarVariableMeanPtr unsupervisedRate;
};

class GoActionScoringEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual TypePtr getRequiredSupervisionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new GoActionScoringScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {result.staticCast<GoActionScoringScoreObject>()->add(context, prediction.getObjectAndCast<DenseDoubleVector>(), supervision.getObjectAndCast<DenseDoubleVector>());}
};

class GoSupervisedEpisodeEvaluator : public CallbackBasedEvaluator
{
public:
  GoSupervisedEpisodeEvaluator() : CallbackBasedEvaluator(new GoActionScoringEvaluator()) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const
  {
    const DecisionProblemSupervisedEpisodePtr& episodeFunction = evaluatedFunction.staticCast<DecisionProblemSupervisedEpisode>();
    return episodeFunction->getSupervisedDecisionMaker().staticCast<SupervisedLinearRankingBasedDecisionMaker>()->getRankingMachine();
  }
};

GoStateComponent::GoStateComponent(GoStatePtr state, const String& name)
  : MatrixComponent(state->getBoard()), state(state), actionsPerceptionFunction(new GoActionsPerception())
{
  availableActions = state->getAvailableActions();
}

class GoSandBox : public WorkUnit
{
public:
  GoSandBox() : maxCount(0), numFolds(7), learningParameters(new StochasticGDParameters(constantIterationFunction(1.0))), testFeatures(false)
  {
  }

  ContainerPtr loadGames(ExecutionContext& context, const File& directoryOrFile, size_t maxCount)
  {
    if (!directoryOrFile.exists())
    {
      context.errorCallback(T("File ") + directoryOrFile.getFullPathName() + T(" does not exists"));
      return ContainerPtr();
    }
    if (directoryOrFile.isDirectory())
      return directoryFileStream(context, directoryOrFile, T("*.sgf"))->load(maxCount, false)
        ->apply(context, new LoadSGFFileFunction(), Container::parallelApply);
    else
    {
      FunctionPtr loader = new LoadCompositeSGFFileFunction(maxCount);
      return loader->compute(context, directoryOrFile).getObjectAndCast<Container>();
    }
  }

  virtual Variable run(ExecutionContext& context)
  {
    double startTime = Time::getMillisecondCounterHiRes();

    // create problem
    DecisionProblemPtr problem = new GoProblem(0);

    // load games
    context.enterScope(T("Loading training games from ") + context.getFilePath(trainingFile));
    ContainerPtr trainingGames = loadGames(context, trainingFile, maxCount);
    context.leaveScope((trainingGames ? trainingGames->getNumElements() : 0));
    
    context.enterScope(T("Loading testing games from ") + context.getFilePath(testingFile));
    ContainerPtr testingGames = loadGames(context, testingFile, maxCount);
    context.leaveScope((testingGames ? testingGames->getNumElements() : 0));

    ContainerPtr validationGames;
    if (numFolds)
    {
      validationGames = trainingGames->fold(0, numFolds);
      trainingGames = trainingGames->invFold(0, numFolds);
    }
    else
      validationGames = testingGames; // tmp ...
    if (!printGamesInfo(context, trainingGames, T("training")) ||
        (validationGames && !printGamesInfo(context, validationGames, T("validation"))) ||
        !printGamesInfo(context, testingGames, T("testing")))
      return false;

    if (interactiveFile.existsAsFile() && !processInteractiveFile(context, interactiveFile))
      return false;
  
    // test features
    if (testFeatures)
    {
      PairPtr pair = trainingGames->getElement(0).getObjectAndCast<Pair>();
      DecisionProblemStatePtr state = pair->getFirst().getObjectAndCast<DecisionProblemState>();
      ContainerPtr trajectory  = pair->getSecond().getObjectAndCast<Container>();
      for (size_t i = 0; i < 150; ++i)
      {
        double r;
        state->performTransition(trajectory->getElement(i), r);
      }
      context.resultCallback(T("state"), state);
      return true;
    }


    // create ranking machine
    if (!learningParameters)
    {
      context.errorCallback(T("No learning parameters"));
      return false;
    }
    StochasticGDParametersPtr sgdParameters = learningParameters.dynamicCast<StochasticGDParameters>();
    if (!sgdParameters)
    {
      context.errorCallback(T("Learning parameters type not supported"));
      return false;
    }
    FunctionPtr goDecisionMaker = new SupervisedLinearRankingBasedDecisionMaker(new GoActionsPerception(), sgdParameters);

    FunctionPtr episode = new DecisionProblemSupervisedEpisode(goDecisionMaker);
    if (!episode->initialize(context, goStateClass, containerClass(positiveIntegerPairClass)))
      return false;

    EvaluatorPtr evaluator = new GoSupervisedEpisodeEvaluator();
    evaluator->setUseMultiThreading(true);
    episode->setEvaluator(evaluator);
    episode->setBatchLearner(learningParameters->createBatchLearner(context));
    episode->setOnlineLearner(
      compositeOnlineLearner(evaluatorOnlineLearner(false, true), stoppingCriterionOnlineLearner(sgdParameters->getStoppingCriterion()), restoreBestParametersOnlineLearner()));
    
    episode->train(context, trainingGames, validationGames, T("Training"), true);

    //goEpisodeFunction->evaluate(context, trainingGames, EvaluatorPtr(), T("Evaluating on training examples"));
    //goEpisodeFunction->evaluate(context, validationGames, EvaluatorPtr(), T("Evaluating on validation examples"));


    return Variable((Time::getMillisecondCounterHiRes() - startTime) / 1000.0, timeType);

    /*
    // check validity
    context.enterScope(T("Check validity"));
    bool ok = true;
    for (size_t i = 0; i < games->getNumElements(); ++i)
    {
      context.progressCallback(new ProgressionState(i, games->getNumElements(), T("Games")));
      PairPtr stateAndTrajectory = games->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
      {
        DecisionProblemStatePtr state = stateAndTrajectory->getFirst().getObject()->cloneAndCast<DecisionProblemState>();
        ok &= state->checkTrajectoryValidity(context, stateAndTrajectory->getSecond().getObjectAndCast<Container>());
      }
      
    }
    context.leaveScope(ok);
    return true;
    */
  }

  bool processInteractiveFile(ExecutionContext& context, const File& file) const
  {
    XmlElementPtr xml = SGFFileParser(context, file).next().dynamicCast<XmlElement>();
    if (!xml)
      return false;

    FunctionPtr convertFunction = new ConvertSGFXmlToStateAndTrajectory();
    PairPtr stateAndTrajectory = convertFunction->compute(context, xml).getObjectAndCast<Pair>();
    if (!stateAndTrajectory)
      return false;

    GoStatePtr state = stateAndTrajectory->getFirst().getObjectAndCast<GoState>();
    ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();

    double sumOfRewards = 0.0;
    state->performTrajectory(trajectory, sumOfRewards);
    context.resultCallback(T("state"), state);
    context.resultCallback(T("sumOfRewards"), sumOfRewards);
    return true;
  }

private:
  friend class GoSandBoxClass;

  File trainingFile;
  File testingFile;

  File interactiveFile;

  size_t maxCount;
  size_t numFolds;
  LearnerParametersPtr learningParameters;
  bool testFeatures;

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

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

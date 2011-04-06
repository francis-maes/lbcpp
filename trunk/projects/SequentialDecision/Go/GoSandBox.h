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
# include "GoActionScoringEvaluator.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

GoStateComponent::GoStateComponent(GoStatePtr state, const String& name)
  : MatrixComponent(state->getBoard()), state(state), actionsPerceptionFunction(new GoActionsPerception())
{
  availableActions = state->getAvailableActions();
}

GoStateAndScoresComponent::GoStateAndScoresComponent(PairPtr stateAndScores, const String& name)
  : GoStateComponent(stateAndScores->getFirst().getObjectAndCast<GoState>(), name), scores(stateAndScores->getSecond().getObjectAndCast<Matrix>())
{
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
    
    CompositeFunctionPtr goDecisionMaker = new SupervisedLinearRankingBasedDecisionMaker(new GoActionsPerception(), sgdParameters);
 /*
    if (outputFile != File::nonexistent)
    {
      goDecisionMaker = CompositeFunction::createFromFile(context, outputFile);
      context.resultCallback(T("loadedGoDecisionMaker"), goDecisionMaker);
      return true;
    }*/
 
    //goDecisionMaker->initialize(context, goStateClass, positiveIntegerPairClass);
   //if (interactiveFile.existsAsFile() && !GoPredictWorkUnit::processSgfFile(context, interactiveFile, goDecisionMaker))
   //   return false;

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

    // train
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
 
    if (outputFile != File::nonexistent)
    {
      context.resultCallback(T("savedGoDecisionMaker"), goDecisionMaker);
      goDecisionMaker->saveToFile(context, outputFile);
    }
    if (interactiveFile.existsAsFile() && !GoPredict::processSgfFile(context, interactiveFile, goDecisionMaker))
      return false;

    /* tmp
    if (outputFile != File::nonexistent)
    {
      goDecisionMaker = CompositeFunction::createFromFile(context, outputFile);
      context.resultCallback(T("loadedGoDecisionMaker"), goDecisionMaker);
      context.enterScope(T("Loading test"));
      bool ok = !interactiveFile.existsAsFile() || GoPredict::processSgfFile(context, interactiveFile, goDecisionMaker); 
      goDecisionMaker->saveToFile(context, File(outputFile.getFullPathName() + T(".test")));
      context.leaveScope(ok);
      return true;
    }
    */

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

private:
  friend class GoSandBoxClass;

  File trainingFile;
  File testingFile;

  File interactiveFile;

  size_t maxCount;
  size_t numFolds;
  LearnerParametersPtr learningParameters;
  bool testFeatures;
  
  File outputFile;

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

/*-----------------------------------------.---------------------------------.
| Filename: GoSGFToGTPWorkUnit.h           | Go SGF -> GTP Work Unit         |
| Author  : Francis Maes                   |                                 |
| Started : 06/04/2011 12:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SGF_TO_GTP_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SGF_TO_GTP_SAND_BOX_H_

# include "GoProblem.h"
# include "GoActionScoringEvaluator.h"
# include "LoadSGFFileFunction.h"
# include <lbcpp/Execution/WorkUnit.h>
#include "../../juce/ConsoleProcess.h"

namespace lbcpp
{

using juce::ConsoleProcess;

class GoSGFToGTP : public WorkUnit
{
public:
  GoSGFToGTP()
  {
    executable = T("mogo");
    parameters = T("--19 --nbTotalSimulations 63 --earlyCut 0");
    // commandLine == T("gnugo --mode gtp");
  }

  virtual Variable run(ExecutionContext& context)
    {return processSGFFile(context, sgfFile);}

  bool processSGFFile(ExecutionContext& context, const File& sgfFile) const
  {
    PairPtr stateAndTrajectory = LoadSGFFileFunction::loadStateAndTrajectory(context, sgfFile);
    if (!stateAndTrajectory)
      return false;

    ConsoleProcess* process = ConsoleProcess::create(executable, parameters); // workingDirectory
    if (!process)
    {
      context.errorCallback(T("Could not create ConsoleProcess ") + executable + T(" ") + parameters);
      return false;
    }
    ScoreObjectPtr res = processStateAndTrajectory(context, *process, stateAndTrajectory->getFirst().getObjectAndCast<GoState>(), stateAndTrajectory->getSecond().getObjectAndCast<Container>());
    delete process;
    context.resultCallback(T("scores"), res);
    return true;
  }

  ScoreObjectPtr processStateAndTrajectory(ExecutionContext& context, ConsoleProcess& process, const GoStatePtr& s, const ContainerPtr& trajectory) const
  {
    GoStatePtr state = s->cloneAndCast<GoState>(context);
    size_t boardSize = state->getBoardSize();

    String initCommands;
    initCommands += T("boardsize ") + String((int)boardSize) + T("\n");
    initCommands += T("clear_board\n");
    if (!process.writeStandardInput(initCommands))
      return ScoreObjectPtr();

    SupervisedEvaluatorPtr evaluator = new GoActionScoringEvaluator();
    ScoreObjectPtr scores = evaluator->createEmptyScoreObject(context, FunctionPtr());

    size_t n = trajectory->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      String player = (state->getCurrentPlayer() == blackPlayer ? T("black") : T("white"));
      if (!process.writeStandardInput(T("genmove ") + player + T("\n")))
        return ScoreObjectPtr();
      if (!process.writeStandardInput(T("showboard\n")))
        return ScoreObjectPtr();
      
      ContainerPtr availableActions = state->getAvailableActions();
      Variable correctAction = trajectory->getElement(i);
      size_t numActions = availableActions->getNumElements();

      bool actionFound = false;
      DenseDoubleVectorPtr costs = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, numActions);
      for (size_t i = 0; i < numActions; ++i)
      {
        if (availableActions->getElement(i) == correctAction)
        {
          actionFound = true;
          costs->setValue(i, -1.0);
        }
      }
      if (!actionFound)
        context.warningCallback(T("Could not find action ") + correctAction.toShortString());

      PairPtr example = new Pair(Variable::missingValue(objectVectorClass(doubleVectorClass())), costs);
      DenseDoubleVectorPtr scoreVector = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, numActions);
      // todo: retrieve predicted scores

      if (!evaluator->updateScoreObject(context, scores, example, scoreVector))
        return false;

      double reward;
      state->performTransition(correctAction, reward);

      if (!process.writeStandardInput(T("undo\n")))
        return ScoreObjectPtr();
      if (!process.writeStandardInput(T("play ") + player + T(" ") + formatMove(correctAction.getObjectAndCast<PositiveIntegerPair>(), boardSize) + T("\n")))
        return ScoreObjectPtr();
    }

    evaluator->finalizeScoreObject(scores, FunctionPtr());
    if (!process.writeStandardInput(T("quit\n")))
      return ScoreObjectPtr();
    return scores;
  }

  static String formatMove(const PositiveIntegerPairPtr& pair, size_t boardSize)
  {
    int x = pair->getFirst();
    int y = pair->getSecond();
    char cx = 'A' + x;
    if (cx >= 'I')
      ++cx;
    String cy = String(boardSize - 1 - y);

    String res;
    res += cx;
    res += cy;
    return res;
  }

private:
  friend class GoSGFToGTPClass;

  String executable;
  String parameters;
  File sgfFile;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SGF_TO_GTP_SAND_BOX_H_

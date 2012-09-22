/*-----------------------------------------.---------------------------------.
| Filename: GoPredictWorkUnit.h            | Go Predict Work Unit            |
| Author  : Francis Maes                   |                                 |
| Started : 06/04/2011 00:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_PREDICT_WORK_UNIT_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_PREDICT_WORK_UNIT_H_

# include "GoProblem.h"
# include "LoadSGFFileFunction.h"
# include <lbcpp/Execution/ExecutionContext.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class GoPredict : public WorkUnit
{
public:
  GoPredict() : maxTurns(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!modelFile.existsAsFile() || !sgfFile.existsAsFile())
      return false;

    // load go decision maker
    FunctionPtr goDecisionMaker = CompositeFunction::createFromFile(context, modelFile);
    if (!goDecisionMaker)
    {
      context.errorCallback(T("Could not load ") + modelFile.getFileName());
      return false;
    }
    if (!goDecisionMaker->initialize(context, goStateClass, positiveIntegerPairClass))
      return false;

    // load sgf file
    context.enterScope(T("Predicting scores for final state of ") + sgfFile.getFileName());
    bool ok = processSgfFile(context, sgfFile, goDecisionMaker, maxTurns);
    context.leaveScope(ok);
    return ok;
  }

  static DenseDoubleVectorPtr computeScores(ExecutionContext& context, const DecisionProblemStatePtr& state, CompositeFunctionPtr goDecisionMaker, ContainerPtr& availableActions)
  {
    availableActions = state->getAvailableActions();

    // compute scores
    std::vector<Variable> decisionMakerInputs(2);
    decisionMakerInputs[0] = Variable(state, goStateClass);
    decisionMakerInputs[1] = Variable::missingValue(positiveIntegerPairClass);
    DenseDoubleVectorPtr scoreVector = goDecisionMaker->computeUntilStep(context, &decisionMakerInputs[0], goDecisionMaker->getNumSteps() - 2).getObjectAndCast<DenseDoubleVector>();
    jassert(!scoreVector || scoreVector->getNumElements() == availableActions->getNumElements());
    return scoreVector;
  }

  static bool processSgfFile(ExecutionContext& context, const File& file, CompositeFunctionPtr goDecisionMaker, size_t maxTurns = 0)
  {
    if (!goDecisionMaker->isInitialized() && !goDecisionMaker->initialize(context, goStateClass, positiveIntegerPairClass))
      return false;
    if (!file.exists())
    {
      context.errorCallback(T("File ") + file.getFileName() + T(" does not exists"));
      return false;
    }
      
    PairPtr stateAndTrajectory = LoadSGFFileFunction::loadStateAndTrajectory(context, file);
    if (!stateAndTrajectory)
      return false;

    GoStatePtr state = stateAndTrajectory->getFirst().clone(context).getObjectAndCast<GoState>();
    if (!state)
      return false;

    size_t size = state->getBoard()->getSize();
    ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();
    
    context.enterScope(T("Simulating game"));
    double sumOfRewards = 0.0;
    state->performTrajectory(trajectory, sumOfRewards, maxTurns);
    context.resultCallback(T("state"), state);
    context.leaveScope(true);
    state->saveToFile(context, context.getFile(T("state_test.xml")));

    context.enterScope(T("Computing scores"));
    ContainerPtr actions;
    DenseDoubleVectorPtr scoreVector = computeScores(context, state, goDecisionMaker, actions);
    size_t n = actions->getNumElements();
    if (!scoreVector)
    {
      context.warningCallback(T("No predicted scores, setting all scores to zero"));
      scoreVector = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, n, 0.0);
      context.leaveScope(false);
    }
    else
    {
      context.resultCallback(T("scoreVector"), scoreVector);
      context.leaveScope(true);
    }

    DoubleMatrixPtr scores = new DoubleMatrix(size, size);
    for (size_t i = 0; i < n; ++i)
    {
      PositiveIntegerPairPtr position = actions->getElement(i).getObjectAndCast<PositiveIntegerPair>();
      if (position->getFirst() == size && position->getSecond() == size)
        context.resultCallback(T("passScore"), scoreVector->getValue(i));
      else
        scores->setValue(position->getSecond(), position->getFirst(), scoreVector->getValue(i));
    }
    context.resultCallback(T("scores"), scores);
    context.resultCallback(T("stateAndScores"), Variable::pair(state, scores));
    return true;
  }

protected:
  friend class GoPredictClass;

  File modelFile;
  File sgfFile;
  size_t maxTurns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

/*-----------------------------------------.---------------------------------.
| Filename: GoBoostingSandBox.h            | Go Boosting Sand Box            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 11:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_

# include "GoProblem.h"
# include "LoadSGFFileFunction.h"
# include "GoActionsPerception.h"
# include "GoSupervisedEpisode.h"
# include "GoActionScoringEvaluator.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SGFToStateSamples : public WorkUnit
{
public:
  SGFToStateSamples() : maxCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("Loading input trajectories from ") + context.getFilePath(input));
    ContainerPtr trajectories = loadSGFTrajectories(context, input, maxCount);
    size_t n = trajectories ? trajectories->getNumElements() : 0;
    context.leaveScope(n);
    if (!n)
      return false;
    
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    context.enterScope(T("Converting"));
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr stateAndTrajectory = trajectories->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
      {
        PairPtr stateAndAction = sampleStateActionPairFromTrajectory(context, stateAndTrajectory->getFirst().getObjectAndCast<GoState>(), stateAndTrajectory->getSecond().getObjectAndCast<Container>());
        GoStatePtr state = stateAndAction->getFirst().getObjectAndCast<GoState>();
        stateAndAction->saveToFile(context, outputDirectory.getChildFile(state->getName() + ".stateActionPair"));
      }
      context.progressCallback(new ProgressionState(i+1, n, "games"));
    }
    context.leaveScope();
    return true;
  }

private:
  friend class SGFToStateSamplesClass;

  File input;
  size_t maxCount;
  File outputDirectory;

  PairPtr sampleStateActionPairFromTrajectory(ExecutionContext& context, GoStatePtr initialState, ContainerPtr actions) const
  {
    size_t trajectoryLength = actions->getNumElements();
    size_t l = context.getRandomGenerator()->sampleSize(0, trajectoryLength);

    GoStatePtr res = initialState->cloneAndCast<GoState>();
    for (size_t i = 0; i < l; ++i)
    {
      Variable action = actions->getElement(i);
      double reward;
      res->performTransition(context, action, reward);
    }
    return new Pair(res, actions->getElement(l));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_BOOSTING_SAND_BOX_H_

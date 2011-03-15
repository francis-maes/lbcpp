/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "../Problem/GoProblem.h"
# include "../Problem/LoadSGFFileFunction.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/UserInterface/MatrixComponent.h>

namespace lbcpp
{

class GoStateComponent : public MatrixComponent
{
public:
  GoStateComponent(GoStatePtr state, const String& name)
    : MatrixComponent(state->getBoard()) {}
 
  virtual juce::Colour selectColour(const Variable& element) const
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }
};

class GoSandBox : public WorkUnit
{
public:
  GoSandBox() : maxCount(0), numFolds(7)
  {
  }

  ContainerPtr loadGames(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    if (!gamesDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid games directory"));
      return ContainerPtr();
    }

    return directoryFileStream(context, directory, T("*.sgf"))->load(maxCount, false)->apply(context, new LoadSGFFileFunction(), Container::parallelApply);
  }

  virtual Variable run(ExecutionContext& context)
  {
    // create problem
    DecisionProblemPtr problem = new GoProblem(0);
    if (!problem->initialize(context))
      return false;

    // load games
    ContainerPtr games = loadGames(context, gamesDirectory, maxCount);
    if (!games)
      return false;

    // check validity
    context.enterScope(T("Check validity"));
    bool ok = true;
    for (size_t i = 0; i < games->getNumElements(); ++i)
    {
      context.progressCallback(new ProgressionState(i, games->getNumElements(), T("Games")));
      PairPtr stateAndTrajectory = games->getElement(i).getObjectAndCast<Pair>();
      if (stateAndTrajectory)
        ok &= problem->checkTrajectoryValidity(context, stateAndTrajectory->getFirst(), stateAndTrajectory->getSecond().getObjectAndCast<Container>());
    }
    context.leaveScope(ok);
    return true;

    /*
    ContainerPtr trainingGames = games->invFold(0, numFolds);
    ContainerPtr testingGames = games->fold(0, numFolds);
    context.informationCallback(String((int)trainingGames->getNumElements()) + T(" training games, ") + String((int)testingGames->getNumElements()) + T(" testing games"));*/

#if 0
    if (!fileToParse.existsAsFile())
    {
      context.errorCallback(T("File to parse does not exist"));
      return false;
    }

    XmlElementPtr xml = (new SGFFileParser(context, fileToParse))->next().dynamicCast<XmlElement>();
    if (!xml)
      return false;

    //context.resultCallback(T("XML"), xml);

    FunctionPtr convert = new ConvertSGFXmlToStateAndTrajectory();
    PairPtr stateAndTrajectory = convert->compute(context, xml).getObjectAndCast<Pair>();
    if (!stateAndTrajectory)
      return false;
  
    Variable initialState = stateAndTrajectory->getFirst();
    ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();

    context.resultCallback(T("Initial State"), initialState);
    context.resultCallback(T("Trajectory"), trajectory);

    DecisionProblemPtr problem = new GoProblem(0);
    if (!problem->initialize(context))
      return false;

    Variable finalState = problem->computeFinalState(context, initialState, trajectory);
    context.resultCallback(T("Final State"), finalState);
    return true;
#endif // 0
  }

private:
  friend class GoSandBoxClass;

  File gamesDirectory;
  size_t maxCount;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

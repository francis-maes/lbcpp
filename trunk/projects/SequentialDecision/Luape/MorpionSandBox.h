/*-----------------------------------------.---------------------------------.
| Filename: MorpionSandBox.h               | Morpion SandBox                 |
| Author  : Francis Maes                   |                                 |
| Started : 24/07/2012 18:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MORPION_SAND_BOX_H_
# define LBCPP_LUAPE_MORPION_SAND_BOX_H_

# include "MetaMCSandBox.h"
# include "../Problem/MorpionProblem.h"

namespace lbcpp
{

class PlayMorpionTrajectory : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (file == File::nonexistent)
    {
      context.errorCallback("No input file");
      return false;
    }
    juce::InputStream* istr = file.createInputStream();
    if (!istr)
    {
      context.errorCallback("Could not open file " + file.getFileName());
      return false;
    }
    MorpionStatePtr initialState;
    ObjectVectorPtr actions;
    if (!parseFile(context, *istr, initialState, actions))
      return false;
    context.resultCallback("initialState", initialState);
    context.resultCallback("actions", actions);

    MorpionStatePtr state = initialState->cloneAndCast<MorpionState>();
    for (size_t i = 0; i < actions->getNumElements(); ++i)
    {
      MorpionActionPtr action = actions->getAndCast<MorpionAction>(i);
      double reward;
      state->performTransition(context, action, reward);
      context.enterScope("Step " + String((int)i+1));
      context.resultCallback("step", i+1);
      context.resultCallback("action", action);
      context.resultCallback("state", state->cloneAndCast<MorpionState>());
      ContainerPtr actions = state->getAvailableActions()->cloneAndCast<Container>();
      context.resultCallback("availableActions", actions);
      context.resultCallback("numAvailableActions", actions->getNumElements());
      context.leaveScope();
    }
    context.resultCallback("finalState", state);
    return true;
  }

  MorpionPoint parsePosition(const String& position) const
  {
    jassert(position.isNotEmpty() && position[0] == '(' && position[position.length() - 1] == ')');
    int i = position.indexOfChar(',');
    jassert(i >= 0);
    return MorpionPoint(position.substring(1, i).getIntValue(), position.substring(i + 1, position.length() - 1).getIntValue());
  }

  MorpionDirection parseDirection(const juce::tchar direction) const
  {
    if (direction == '-')
      return MorpionDirection::E;
    else if (direction == '|')
      return MorpionDirection::S;
    else if (direction == '/')
      return MorpionDirection::NE;
    else if (direction == '\\')
      return MorpionDirection::SE;
    else
      return MorpionDirection::none;
  }

  MorpionActionPtr parseAction(const String& line, size_t crossLength, const MorpionPoint& referencePosition) const
  {
    int i = line.indexOfChar(')');
    if (i < 0)
      return MorpionActionPtr();
    ++i;
    MorpionPoint point = parsePosition(line.substring(0, i));
    int delta = crossLength - 2; // the reference of Pentasol is shifted compared to ours
    point = MorpionPoint(point.getX() - referencePosition.getX() + delta, point.getY() - referencePosition.getY() + delta);
    if (line[i] != ' ')
      return MorpionActionPtr();
    ++i;
    MorpionDirection direction = parseDirection(line[i]);
    if (direction == MorpionDirection::none)
      return MorpionActionPtr();
    ++i;
    if (line[i] != ' ')
      return MorpionActionPtr();
    ++i;
    if (line[i] == '+')
      ++i;
    int indexInLine = -line.substring(i).getIntValue() + (int)crossLength / 2;
    if (indexInLine < 0 || indexInLine >= (int)crossLength)
      return MorpionActionPtr();
    return new MorpionAction(point, direction, (size_t)indexInLine, (size_t)indexInLine);
  }

  bool parseFile(ExecutionContext& context, InputStream& istr, MorpionStatePtr& initialState, ObjectVectorPtr& actions)
  {
    MorpionPoint referencePosition(0, 0);
    size_t crossLength = 0;

    initialState = MorpionStatePtr();
    actions = new ObjectVector(morpionActionClass, 0);
    while (!istr.isExhausted())
    {
      String line = istr.readNextLine().trim();
      if (line.isEmpty() || line[0] == '#')
        continue;
      if (line.startsWith(T("GameType")))
      {
        String gameType = line.substring(9);
        jassert(isdigit(gameType[0]) && gameType.length() >= 2);
        crossLength = gameType[0] - '0';
        bool isDisjoint = (gameType[1] == 'D');
        initialState = new MorpionState(crossLength, isDisjoint);
        referencePosition = parsePosition(istr.readNextLine().trim());
        continue;
      }
      else
      {
        if (!initialState || !crossLength)
        {
          context.errorCallback("parse error");
          return false;
        }
        MorpionActionPtr action = parseAction(line, crossLength, referencePosition);
        if (!action)
        {
          context.errorCallback("could not parse action");
          return false;
        }
        actions->append(action);
      }
    }
    return true;
  }

protected:
  friend class PlayMorpionTrajectoryClass;

  File file;
};

class RunNRPAWorkUnit : public RunMCAlgorithmWorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
      return false;

    RandomGeneratorPtr random = context.getRandomGenerator();

    CompositeWorkUnitPtr wu(new CompositeWorkUnit("NRPA", numRuns));
    for (size_t run = 0; run < numRuns; ++run)
    {
      /*static const size_t numIterationss[] = {5, 10, 20, 50, 100, 200, 500, 1000};
      static const double learningRates[] = {0.001, 0.01, 0.1, 1.0, 10.0};
    
      size_t numIterations = numIterationss[random->sampleSize(8)];
      double learningRate = learningRates[random->sampleSize(5)];*/
      MCAlgorithmPtr algorithm = new NRPAMCAlgorithm(6, 0, 0.0);
      wu->setWorkUnit(run, new RunAlgorithmWorkUnit(problem, algorithm, budget, run, algorithm->toShortString()));
    }
    wu->setProgressionUnit("Runs");
    wu->setPushChildrenIntoStackFlag(true);

    ContainerPtr results = context.run(wu).getObjectAndCast<Container>();

    ScalarVariableStatisticsPtr statistics(new ScalarVariableStatistics("scores"));
    for (size_t i = 0; i < results->getNumElements(); ++i)
      statistics->push(results->getElement(i).toDouble());
    context.informationCallback("Results: " + statistics->toShortString());
    context.resultCallback("statistics", statistics);
    return true;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MORPION_SAND_BOX_H_

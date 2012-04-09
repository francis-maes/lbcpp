/*-----------------------------------------.---------------------------------.
| Filename: MCBanditPool.cpp               | Monte Carlo Bandit Pool         |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2012 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "MCBanditPool.h"
#include <lbcpp/Core/Function.h>
#include <lbcpp/Execution/WorkUnit.h>
using namespace lbcpp;

void MCBanditPool::play(ExecutionContext& context, size_t numTimeSteps, bool showProgression)
{
  for (size_t i = 0; i < numTimeSteps; ++i)
  {
    if (showProgression)
      context.progressCallback(new ProgressionState(i + 1, numTimeSteps, T("Steps")));

    selectAndPlayArm(context);

    if (useMultiThreading && context.isMultiThread())
    {
      context.flushCallbacks();
      //context.informationCallback(T("Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
      
      while (getNumCurrentlyPlayedArms() >= 10)
      {
        //context.informationCallback(T("Waiting - Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
        Thread::sleep(10);
        context.flushCallbacks();
      }
    }
  }

  if (useMultiThreading && context.isMultiThread())
    while (getNumCurrentlyPlayedArms() > 0)
    {
      Thread::sleep(10);
      context.flushCallbacks();
    }
}

void MCBanditPool::playIterations(ExecutionContext& context, size_t numIterations, size_t stepsPerIteration)
{
  for (size_t i = 0; i < numIterations; ++i)
  {
    context.enterScope(T("Iteration ") + String((int)i+1));
    context.resultCallback(T("iteration"), i+1);
    play(context, stepsPerIteration);
    displayInformation(context, 25);
    context.leaveScope();
  }
}

const Variable& MCBanditPool::getArmParameter(size_t index) const
  {jassert(index < arms.size()); return arms[index].parameter;}

void MCBanditPool::setArmParameter(size_t index, const Variable& parameter)
  {jassert(index < arms.size()); arms[index].parameter = parameter;}

size_t MCBanditPool::selectAndPlayArm(ExecutionContext& context)
{
  int index = popArmFromQueue();
  if (index < 0)
    return (size_t)-1;

  Arm& arm = arms[index];
  std::vector<Variable> variables;
  variables.push_back(arm.parameter);
  if (objective->getNumRequiredInputs() == 2)
    variables.push_back(arm.playedCount);
  WorkUnitPtr workUnit = functionWorkUnit(objective, variables);
  if (useMultiThreading && context.isMultiThread())
  {
    jassert(currentlyPlayedArms.find(arm.parameter) == currentlyPlayedArms.end());
    currentlyPlayedArms[arm.parameter] = (size_t)index;
    context.pushWorkUnit(workUnit, this, false);
  }
  else
  {
    double reward = context.run(workUnit, false).toDouble();
    observeReward(index, reward);
  }
  return (size_t)index;
}

void MCBanditPool::workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace)
{
  FunctionWorkUnitPtr wu = workUnit.staticCast<FunctionWorkUnit>();
  
  Variable parameter = wu->getInputs()[0];
  std::map<Variable, size_t>::iterator it = currentlyPlayedArms.find(parameter);
  jassert(it != currentlyPlayedArms.end());
  size_t index = it->second;
  currentlyPlayedArms.erase(it);

  observeReward(index, result.toDouble());
}

void MCBanditPool::observeReward(size_t index, double reward)
{
  Arm& arm = arms[index];
  arm.observe(reward);
  pushArmIntoQueue(index, getIndexScore(arm));
}

void MCBanditPool::displayInformation(ExecutionContext& context, size_t numBestArms)
{
  std::multimap<double, size_t> armsByMeanReward;
  for (size_t i = 0; i < arms.size(); ++i)
    if (arms[i].parameter.exists())
      armsByMeanReward.insert(std::make_pair(arms[i].getMeanReward(), i));

  size_t i = 1;
  for (std::multimap<double, size_t>::reverse_iterator it = armsByMeanReward.rbegin(); i < numBestArms && it != armsByMeanReward.rend(); ++it, ++i)
  {
    Arm& arm = arms[it->second];
    context.informationCallback(T("[") + String((int)i) + T("] r = ") + String(arm.getMeanReward())
      + T(" t = ") + String((int)arm.playedCount) + T(" -- ") + arm.parameter.toShortString());
  }

  if (armsByMeanReward.size())
  {
    Arm& bestArm = arms[armsByMeanReward.rbegin()->second];
    context.resultCallback(T("bestArmReward"), bestArm.getMeanReward());
    context.resultCallback(T("bestArmPlayCount"), bestArm.playedCount);
  }
  context.resultCallback(T("numArms"), arms.size());
}

size_t MCBanditPool::sampleArmWithHighestReward(ExecutionContext& context) const
{
  double bestReward = -DBL_MAX;
  std::vector<size_t> bests;
  for (size_t i = 0; i < arms.size(); ++i)
  {
    double reward = arms[i].getMeanReward();
    if (reward >= bestReward)
    {
      if (reward > bestReward)
      {
        bestReward = reward;
        bests.clear();
      }
      bests.push_back(i);
    }
  }
  RandomGeneratorPtr random = context.getRandomGenerator();
  if (bests.empty())
    return random->sampleSize(arms.size());
  else
    return bests[random->sampleSize(bests.size())];
}

void MCBanditPool::reserveArms(size_t count)
  {arms.reserve(count);}

size_t MCBanditPool::createArm(const Variable& parameter)
{
  jassert(parameter.exists());
  size_t index = arms.size();
  arms.push_back(Arm());
  Arm& arm = arms[index];
  arm.parameter = parameter;
  pushArmIntoQueue(index, DBL_MAX);
  return index;
}

void MCBanditPool::destroyArm(size_t index)
{
  arms[index] = Arm();
  // FIXME: check that it is not beeing played
  jassert(false); // FIXME: remove from queue
}

double MCBanditPool::getIndexScore(Arm& arm) const
  {return arm.playedCount ? (arm.rewardSum + explorationCoefficient) / (double)arm.playedCount : DBL_MAX;}

void MCBanditPool::pushArmIntoQueue(size_t index, double score)
  {queue.push(std::make_pair(index, score));}

int MCBanditPool::popArmFromQueue()
{
  if (queue.empty())
    return -1;
  size_t res = queue.top().first;
  queue.pop();
  return (int)res;
}

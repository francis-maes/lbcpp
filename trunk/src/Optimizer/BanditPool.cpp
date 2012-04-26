/*-----------------------------------------.---------------------------------.
| Filename: BanditPool.cpp                 | Monte Carlo Bandit Pool         |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2012 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Optimizer/BanditPool.h>
#include <lbcpp/Core/Function.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <algorithm>
using namespace lbcpp;

void BanditPool::play(ExecutionContext& context, size_t numTimeSteps, bool showProgression)
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

void BanditPool::playIterations(ExecutionContext& context, size_t numIterations, size_t stepsPerIteration)
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

const Variable& BanditPool::getArmParameter(size_t index) const
  {jassert(index < arms.size()); return arms[index].parameter;}

void BanditPool::setArmParameter(size_t index, const Variable& parameter)
  {jassert(index < arms.size()); arms[index].parameter = parameter;}

struct PlayArmWorkUnit : public WorkUnit
{
  PlayArmWorkUnit(BanditPoolObjectivePtr objective, const Variable& parameter, size_t instanceIndex, size_t armInedx)
    : objective(objective), parameter(parameter), instanceIndex(instanceIndex), armIndex(armIndex) {}

  BanditPoolObjectivePtr objective;
  Variable parameter;
  size_t instanceIndex;
  size_t armIndex;

  virtual Variable run(ExecutionContext& context)
    {return objective->computeObjective(context, parameter, instanceIndex);}
};

size_t BanditPool::selectAndPlayArm(ExecutionContext& context)
{
  int index = popArmFromQueue();
  if (index < 0)
    return (size_t)-1;

  Arm& arm = arms[index];
  WorkUnitPtr workUnit = new PlayArmWorkUnit(objective, arm.parameter, arm.playedCount, index);
  if (useMultiThreading && context.isMultiThread())
    context.pushWorkUnit(workUnit, this, false);
  else
  {
    double reward = context.run(workUnit, false).getDouble();
    observeReward(index, reward);
  }
  return (size_t)index;
}

void BanditPool::workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace)
  {observeReward(workUnit.staticCast<PlayArmWorkUnit>()->armIndex, result.toDouble());}

void BanditPool::observeReward(size_t index, double objectiveValue)
{
  Arm& arm = arms[index];
  double worst, best;
  objective->getObjectiveRange(worst, best);
  arm.observe(objectiveValue, (objectiveValue - worst) / (best - worst));
  
  if (arm.playedCount == 1)
    arm.objectiveValueBest = objectiveValue;
  else
  {
    if (best > worst && objectiveValue > arm.objectiveValueBest)
      arm.objectiveValueBest = objectiveValue;
    else if (best < worst && objectiveValue < arm.objectiveValueBest)
      arm.objectiveValueBest = objectiveValue;
  }

  pushArmIntoQueue(index, getIndexScore(arm));
}

void BanditPool::getArmsOrder(std::vector< std::pair<size_t, double> >& res) const
{
  res.resize(arms.size());
  for (size_t i = 0; i < arms.size(); ++i)
  {
    double value = optimizeMax ? arms[i].rewardMax : arms[i].getMeanReward();
    res[i] = std::make_pair(i, -value);
  }
  std::sort(res.begin(), res.end(), ArmScoreComparator());
}

void BanditPool::displayInformation(ExecutionContext& context, size_t numBestArms)
{
  std::vector< std::pair<size_t, double> > order;
  getArmsOrder(order);
  if (order.size())
  {
    size_t count = order.size();
    if (numBestArms < count)
      count = numBestArms;
    for (size_t i = 0; i < count; ++i)
    {
      Arm& arm = arms[order[i].first];
      double objectiveValue = optimizeMax ? arm.objectiveValueBest : arm.getMeanObjectiveValue();
      context.informationCallback(T("[") + String((int)i) + T("] ") + 
        arm.parameter.toShortString() + T(" -> ") + String(objectiveValue) +
          T(" (played ") + String((int)arm.playedCount) + T(" times)"));
    }

    Arm& bestArm = arms[order[0].first];
    context.resultCallback(T("bestArmPlayCount"), bestArm.playedCount);
    context.resultCallback(T("bestArmObjective"), bestArm.getMeanObjectiveValue());
    context.resultCallback(T("bestArmBestObjective"), bestArm.objectiveValueBest);
    context.resultCallback(T("bestArmReward"), bestArm.getMeanReward());
    context.resultCallback(T("bestArmMinReward"), bestArm.rewardMin);
    context.resultCallback(T("bestArmMaxReward"), bestArm.rewardMax);
  }
  context.resultCallback(T("numArms"), arms.size());
}

size_t BanditPool::sampleArmWithHighestReward(ExecutionContext& context) const
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

void BanditPool::reserveArms(size_t count)
  {arms.reserve(count);}

size_t BanditPool::createArm(const Variable& parameter)
{
  jassert(parameter.exists());
  size_t index = arms.size();
  arms.push_back(Arm());
  Arm& arm = arms[index];
  arm.parameter = parameter;
  pushArmIntoQueue(index, DBL_MAX);
  return index;
}

void BanditPool::destroyArm(size_t index)
{
  arms[index] = Arm();
  // FIXME: check that it is not beeing played
  jassert(false); // FIXME: remove from queue
}

double BanditPool::getIndexScore(Arm& arm) const
{
  if (!arm.playedCount)
    return DBL_MAX;
  size_t N = objective->getNumInstances();
  if (N && arm.playedCount == N)
    return -DBL_MAX;
  double score = optimizeMax ? arm.rewardMax : (arm.rewardSum / (double)arm.playedCount);
  return score + explorationCoefficient / (double)arm.playedCount;
}

void BanditPool::pushArmIntoQueue(size_t index, double score)
  {queue.push(std::make_pair(index, score));}

int BanditPool::popArmFromQueue()
{
  if (queue.empty())
    return -1;
  size_t res = queue.top().first;
  queue.pop();
  return (int)res;
}

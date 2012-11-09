/*-----------------------------------------.---------------------------------.
| Filename: BanditPool.cpp                 | Monte Carlo Bandit Pool         |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2012 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/BanditPool.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <algorithm>
using namespace lbcpp;

BanditPool::BanditPool(const StochasticObjectivePtr& objective, double explorationCoefficient, bool optimizeMax, bool useMultiThreading) 
  : objective(objective), explorationCoefficient(explorationCoefficient), optimizeMax(optimizeMax), useMultiThreading(useMultiThreading)
{
}

BanditPool::BanditPool() : explorationCoefficient(0.0)
{
}

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
      
      while (getNumCurrentlyPlayedArms() >= 25)
      {
        //context.informationCallback(T("Waiting - Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
        Thread::sleep(1);
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
    displayInformation(context, 20, 5);
    context.leaveScope();
  }
}

const ObjectPtr& BanditPool::getArmObject(size_t index) const
  {jassert(index < arms.size()); return arms[index].object;}

double BanditPool::getArmMeanObjective(size_t index) const
  {jassert(index < arms.size()); return arms[index].getMeanObjectiveValue();}

double BanditPool::getArmMeanReward(size_t index) const
  {jassert(index < arms.size()); return arms[index].getMeanReward();}

size_t BanditPool::getArmPlayedCount(size_t index) const
  {jassert(index < arms.size()); return arms[index].playedCount;}

void BanditPool::setArmObject(size_t index, const ObjectPtr& object)
  {jassert(index < arms.size()); arms[index].object = object;}

struct PlayArmWorkUnit : public WorkUnit
{
  PlayArmWorkUnit(StochasticObjectivePtr objective, const ObjectPtr& object, size_t instanceIndex, size_t armIndex)
    : objective(objective), object(object), instanceIndex(instanceIndex), armIndex(armIndex) {}

  StochasticObjectivePtr objective;
  ObjectPtr object;
  size_t instanceIndex;
  size_t armIndex;

  virtual ObjectPtr run(ExecutionContext& context)
   {return new NewDouble(objective->evaluate(context, object, instanceIndex));}
};

size_t BanditPool::selectAndPlayArm(ExecutionContext& context)
{
  int index = popArmFromQueue();
  if (index < 0)
    return (size_t)-1;

  Arm& arm = arms[index];
  WorkUnitPtr workUnit = new PlayArmWorkUnit(objective, arm.object, arm.playedCount, index);
  if (useMultiThreading && context.isMultiThread())
    context.pushWorkUnit(workUnit, this, false);
  else
  {
    double objective = NewDouble::get(context.run(workUnit, false));
    observeObjective(index, objective);
  }
  return (size_t)index;
}

void BanditPool::workUnitFinished(const WorkUnitPtr& workUnit, const ObjectPtr& result, const ExecutionTracePtr& trace)
  {observeObjective(workUnit.staticCast<PlayArmWorkUnit>()->armIndex, NewDouble::get(result));}

void BanditPool::observeObjective(size_t index, double objectiveValue)
{
  Arm& arm = arms[index];
  double worst, best;
  objective->getObjectiveRange(worst, best);
  if ((best > worst && objectiveValue == -DBL_MAX) ||
      (best < worst && objectiveValue == DBL_MAX))
    return; // arm is killed
  else
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

void BanditPool::displayAllArms(ExecutionContext& context)
{
  std::vector< std::pair<size_t, double> > order;
  getArmsOrder(order);
  for (size_t i = 0; i < order.size(); ++i)
  {
    Arm& arm = arms[order[i].first];
    context.enterScope(arm.object->toShortString());
    context.resultCallback("rank", i);
    context.resultCallback("object", arm.object->toShortString());
    context.resultCallback("playedCount", arm.playedCount);
    context.resultCallback("meanReward", arm.rewardSum / arm.playedCount);
    context.resultCallback("meanObjectiveValue", arm.objectiveValueSum / arm.playedCount);
    context.leaveScope();
  }
}

void BanditPool::displayArmInformation(ExecutionContext& context, size_t order, size_t armIndex) const
{
  const Arm& arm = arms[armIndex];
  double objectiveValue = optimizeMax ? arm.objectiveValueBest : arm.getMeanObjectiveValue();
  context.informationCallback(T("[") + String((int)order+1) + T("] ") + 
    arm.object->toShortString() + T(" -> ") + String(objectiveValue) +
      T(" (played ") + String((int)arm.playedCount) + T(" times)"));
}

void BanditPool::displayInformation(ExecutionContext& context, size_t numBestArms, size_t numWorstArms) const
{
  std::vector< std::pair<size_t, double> > order;
  getArmsOrder(order);
  if (order.size())
  {
    size_t count = order.size();
    if (numBestArms + numWorstArms >= count)
    {
      for (size_t i = 0; i < count; ++i)
        displayArmInformation(context, i, order[i].first);
    }
    else
    {
      for (size_t i = 0; i < numBestArms; ++i)
        displayArmInformation(context, i, order[i].first);
      context.informationCallback("...");
      for (size_t i = count - numWorstArms; i < count; ++i)
        displayArmInformation(context, i, order[i].first);
    }

    const Arm& bestArm = arms[order[0].first];
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

size_t BanditPool::createArm(const ObjectPtr& object)
{
  jassert(object);
  size_t index = arms.size();
  arms.push_back(Arm());
  Arm& arm = arms[index];
  arm.object = object;
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

/*-----------------------------------------.---------------------------------.
| Filename: BanditEDAOptimizer.h           | Bandit based EDA Optimizer      |
| Author  : Francis Maes, Julien Becker    |                                 |
| Started : 16/06/2011                     |                                 |
`------------------------------------------/                                 |
															 |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDIT_EDA_OPTIMIZER_H_
# define LBCPP_BANDIT_EDA_OPTIMIZER_H_

# include "PopulationBasedOptimizer.h"

namespace lbcpp
{

class BanditInfo : public Object
{
public:
  BanditInfo(const Variable& parameter, size_t identifier)
    : parameter(parameter), identifier(identifier),
      rewardSum(0.0), squaredRewardSum(0.0), objectiveValueSum(0.0), numSamples(0) {}

  double computeScoreToMinimize() const // this score should be minimized (opposite of the index function)
  {
    return -(numSamples ? getRewardMean() + 1 / sqrt((double)numSamples) : DBL_MAX);
    /*
    static const double C = 1.5f;
    return -(numSamples ? getRewardMean() + C / (double)numSamples : DBL_MAX);
    */
  }

  double getRewardMean() const
    {return numSamples ? rewardSum / numSamples : 0.f;}

  size_t getPlayedCount() const
    {return numSamples;}

  const Variable& getParameter() const
    {return parameter;}

  void receiveObjectiveValue(double value)
  {
    //static const double T = 1.0;
    //receiveReward(1 - exp(-value / T));
    jassert(value >= 0.f && value <= 1.f);
    receiveReward(value);
    objectiveValueSum += value;
  }

  void receiveReward(double reward)
  {
    rewardSum += reward;
    squaredRewardSum += reward * reward;
    ++numSamples;
  }

  double getObjectiveValueMean() const
    {return numSamples ? objectiveValueSum / numSamples : -DBL_MAX;}

  virtual String toString() const
    {return T("Bandit ") + String((int)identifier);}

protected:
  friend class BanditInfoClass;

  Variable parameter;
  size_t identifier;

  double rewardSum;
  double squaredRewardSum;
  double objectiveValueSum;
  size_t numSamples;

  BanditInfo() {}
};

typedef ReferenceCountedObjectPtr<BanditInfo> BanditInfoPtr;

class BanditEDAOptimizerState : public SamplerBasedOptimizerState, public ExecutionContextCallback
{
public:
  BanditEDAOptimizerState(const OptimizationProblemPtr& problem)
    : SamplerBasedOptimizerState(problem->getSampler()), problem(problem),
    numPlayed(0), numPlaySent(0), maxBudget(0), totalNumBandits(0), context(NULL)
  {}

  void setExecutionContext(ExecutionContext& context)
    {this->context = &context;}

  size_t getNumBandits() const
    {return banditsByScore.size();}

  void removeBanditsWithWorseRewardMean(ExecutionContext& context, size_t numBanditsToRemove)
  {
    typedef std::multimap<double, BanditInfoPtr> ScoresMap;

    if (getNumBandits() == 0)
      return;

    context.enterScope(T("Remove the ") + String((int)numBanditsToRemove) + (" worse bandits"));
    ScoresMap banditsByRewardMean;
    for (ScoresMap::iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      banditsByRewardMean.insert(std::make_pair(it->second->getRewardMean(), it->second));

    banditsByScore.clear();

    ScoresMap::const_iterator it = banditsByRewardMean.begin();
    for (size_t i = 0; i < numBanditsToRemove; ++i)
    {
      context.resultCallback(it->second->toString(), it->first);
      ++it;
    }

    for (; it != banditsByRewardMean.end(); ++it)
      banditsByScore.insert(std::make_pair(it->second->computeScoreToMinimize(), it->second));

    context.leaveScope(numBanditsToRemove);
  }

  void createBandit(ExecutionContext& context, const Variable& parameter)
  {
    BanditInfoPtr bandit(new BanditInfo(parameter, totalNumBandits));
    banditsByScore.insert(std::make_pair(bandit->computeScoreToMinimize(), bandit));
    context.resultCallback(bandit->toString(), parameter);
    ++totalNumBandits;
  }

  BanditInfoPtr getBestBandit() const
  {
    jassert(banditsByScore.size());
    // Stochastic sampling if there are many best bandits
    return banditsByScore.begin()->second;
  }

  BanditInfoPtr getRandomBandit(const RandomGeneratorPtr& rand) const
  {
    jassert(banditsByScore.size());
    std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin();
    const size_t n = rand->sampleSize(banditsByScore.size());
    for (size_t i = 0; i < n; ++i, ++it);
    return it->second;
  }

  void resetBudgetTo(size_t budget)
  {
    this->maxBudget = budget;
    this->numPlayed = 0;
    this->numPlaySent = 0;
  }

  size_t getNumPlayed() const
    {return numPlayed;}

  void playBandit(const BanditInfoPtr& bandit)
  {
    if (numPlaySent == maxBudget)
      return;
    WorkUnitPtr wu = new FunctionWorkUnit(problem->getObjective(), bandit->getParameter());
    banditsByWorkUnit[wu] = bandit;
    context->pushWorkUnit(wu, this, false);
    ++numPlaySent;
  }

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    ++numPlayed;
    jassert(banditsByWorkUnit.count(workUnit) == 1);
    processResult(banditsByWorkUnit[workUnit], result.getDouble());

    banditsByWorkUnit.erase(workUnit);

    if (numPlayed % 10 == 0)
    {
      context->enterScope(String((int)numPlayed));
      context->resultCallback(T("numPlayed"), numPlayed);
      for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
        context->resultCallback(it->second->toString(), it->second->getRewardMean());
      context->leaveScope(banditsByScore.begin()->second->toString());
    }
    playBandit(getBestBandit());
  }

  void processResult(const BanditInfoPtr& bandit, double score)
  {
    double oldScore = bandit->computeScoreToMinimize();
    bandit->receiveObjectiveValue(score);
    updateBanditIndex(bandit, oldScore);
  }

  void updateBanditIndex(BanditInfoPtr bandit, double previousScore)
  {
    std::multimap<double, BanditInfoPtr>::iterator it = banditsByScore.find(previousScore);
    while (it != banditsByScore.end() && it->first == previousScore && it->second != bandit)
      ++it;
    jassert(it != banditsByScore.end() && it->second == bandit);
    double newScore = bandit->computeScoreToMinimize();
    if (previousScore != newScore)
    {
      banditsByScore.erase(it);
      banditsByScore.insert(std::make_pair(newScore, bandit));
    }
  }
  
  void getParametersSortedByRewardMean(std::multimap<double, Variable>& res) const
  {
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      res.insert(std::make_pair(-it->second->getRewardMean(), it->second->getParameter()));
  }

  void getBandtisSortedByRewardMean(std::multimap<double, BanditInfoPtr>& res) const
  {
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      res.insert(std::make_pair(-it->second->getRewardMean(), it->second));
  }

protected:
  friend class BanditEDAOptimizerStateClass;
  
  OptimizationProblemPtr problem;
  size_t numPlayed;
  size_t numPlaySent;
  size_t maxBudget;
  size_t totalNumBandits;

  BanditEDAOptimizerState()
    : numPlayed(0), numPlaySent(0), maxBudget(0), totalNumBandits(0) {}

private:
  ExecutionContext* context;
  std::multimap<double, BanditInfoPtr> banditsByScore;
  std::map<WorkUnitPtr, BanditInfoPtr> banditsByWorkUnit;
};

typedef ReferenceCountedObjectPtr<BanditEDAOptimizerState> BanditEDAOptimizerStatePtr;

class SamplerBasedWorkUnit : public WorkUnit
{
public:
  SamplerBasedWorkUnit(const SamplerPtr& sampler)
    : sampler(sampler) {}

  virtual Variable run(ExecutionContext& context)
    {return sampler->sample(context, context.getRandomGenerator());}

protected:
  friend class SamplerBasedWorkUnitClass;

  SamplerPtr sampler;

  SamplerBasedWorkUnit() {}
};

class BanditEDAOptimizer : public PopulationBasedOptimizer
{
public:  
  BanditEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, double ratioOfBanditToKeep, size_t budget, size_t numSimultaneousPlays)
    : PopulationBasedOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0, false, false)
    , ratioOfBanditToKeep(ratioOfBanditToKeep), budget(budget), numSimultaneousPlays(numSimultaneousPlays) {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new BanditEDAOptimizerState(problem);}
  
  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {  
    BanditEDAOptimizerStatePtr state = optimizerState.dynamicCast<BanditEDAOptimizerState>();
    jassert(state);
    state->setExecutionContext(context);

    for (size_t i = state->getNumIterations(); i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      BanditInfoPtr bestInterationBandit = performIteration(context, state);
      context.leaveScope(bestInterationBandit ? bestInterationBandit->getObjectiveValueMean() : 0.f);

      if (!bestInterationBandit)
        return state;

      state->incrementNumIterations();
    }
    return state;
  }

protected:
  friend class BanditEDAOptimizerClass;

  double ratioOfBanditToKeep;
  size_t budget;
  size_t numSimultaneousPlays;

  BanditEDAOptimizer() {}
  
  BanditInfoPtr performIteration(ExecutionContext& context, const BanditEDAOptimizerStatePtr& state) const
  {
    /* Remove worse bandits */
    const size_t numBanditsToRemove = (size_t)((1 - ratioOfBanditToKeep) * state->getNumBandits());
    state->removeBanditsWithWorseRewardMean(context, numBanditsToRemove);
    
    /* Create new bandits (Parallel) */
    const size_t numBanditsToCreate = state->getNumBandits() != 0 ? numBanditsToRemove : populationSize;
    context.enterScope(T("Create ") + String((int)numBanditsToCreate) + (" new bandits"));
    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("Creating Bandits"), numBanditsToCreate);
    for (size_t i = 0; i < numBanditsToCreate; ++i)
      workUnits->setWorkUnit(i, new SamplerBasedWorkUnit(state->getSampler()));
    ContainerPtr sampledParameters = context.run(workUnits).getObjectAndCast<Container>();
    jassert(sampledParameters && sampledParameters->getNumElements() == numBanditsToCreate);
    size_t numCreatedBandits = 0;
    for (size_t i = 0; i < numBanditsToCreate; ++i)
    {
      const Variable v = sampledParameters->getElement(i);
      if (v.exists())
      {
        state->createBandit(context, v);
        ++numCreatedBandits;
      }
    }
    context.leaveScope(numCreatedBandits);

    if (numCreatedBandits == 0)
    {
      context.informationCallback(T("No new bandit !"));
      return BanditInfoPtr();
    }

    /* Play */
    context.informationCallback(T("Set budget to ") + String((int)budget) + T(" plays"));
    state->resetBudgetTo(budget);

    context.enterScope(T("Playing"));
    for (size_t i = 0; i < numSimultaneousPlays; ++i)
      state->playBandit(state->getRandomBandit(context.getRandomGenerator()));

    while (state->getNumPlayed() != budget)
    {
      context.progressCallback(new ProgressionState(state->getNumPlayed(), budget, T("Plays")));
      Thread::sleep(1000);
      context.flushCallbacks();
    }
    context.progressCallback(new ProgressionState(budget, budget, T("Plays")));
    context.leaveScope();

    /* Learn */
    std::multimap<double, Variable> sortedParameterScores;
    state->getParametersSortedByRewardMean(sortedParameterScores);

    context.enterScope(T("Learn distribution"));
    learnDistribution(context, state->getSampler(), state, sortedParameterScores);
    context.leaveScope();

    context.enterScope(T("Result"));
    std::multimap<double, BanditInfoPtr> sortedBanditScores;
    state->getBandtisSortedByRewardMean(sortedBanditScores);
    size_t i = 0;
    for (std::multimap<double, BanditInfoPtr>::iterator it = sortedBanditScores.begin(); it != sortedBanditScores.end(); ++it, ++i)
    {
      context.enterScope(T("Rank ") + String((int)i));
      context.resultCallback(T("Rank"), i);
      context.resultCallback(T("Name"), it->second->toString());
      context.resultCallback(T("RewardMean"), it->second->getRewardMean());
      context.resultCallback(T("Score"), it->second->computeScoreToMinimize());
      context.resultCallback(T("ObjectiveValueMean"), it->second->getObjectiveValueMean());
      context.leaveScope();
    }
    context.leaveScope();

    state->submitSolution(sortedBanditScores.begin()->second, -sortedBanditScores.begin()->first);
    return sortedBanditScores.begin()->second;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDIT_EDA_OPTIMIZER_H_

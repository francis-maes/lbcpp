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
    static const double C = 1.5f;
    return -(numSamples ? getRewardMean() + C / (double)numSamples : DBL_MAX);
  }

  double getRewardMean() const
    {return numSamples ? rewardSum / numSamples : -DBL_MAX;}

  size_t getPlayedCount() const
    {return numSamples;}

  const Variable& getParameter() const
    {return parameter;}

  void receiveObjectiveValue(double value)
  {
    static const double T = 1.0;
    receiveReward(1 - exp(-value / T));
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
    {return banditsByParameter.size();}

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
    banditsByParameter.clear();

    ScoresMap::const_iterator it = banditsByRewardMean.begin();
    for (size_t i = 0; i < numBanditsToRemove; ++i)
    {
      context.resultCallback(it->second->toString(), it->first);
      ++it;
    }

    for (; it != banditsByRewardMean.end(); ++it)
    {
      banditsByScore.insert(std::make_pair(it->second->computeScoreToMinimize(), it->second));
      banditsByParameter.insert(std::make_pair(it->second->getParameter(), it->second));
    }

    context.leaveScope(numBanditsToRemove);
  }

  void createBandit(ExecutionContext& context, const Variable& parameter)
  {
    BanditInfoPtr bandit(new BanditInfo(parameter, totalNumBandits));
    banditsByScore.insert(std::make_pair(bandit->computeScoreToMinimize(), bandit));
    banditsByParameter[parameter] = bandit;
    context.resultCallback(bandit->toString(), parameter);
    ++totalNumBandits;
    jassert(banditsByScore.size() == banditsByParameter.size());
  }

  BanditInfoPtr getBestBandit() const
  {
    jassert(banditsByScore.size());
    // Stochastic sampling if there are many best bandits
    return banditsByScore.begin()->second;
  }

  BanditInfoPtr getRandomBandit(const RandomGeneratorPtr& rand) const
  {
    jassert(banditsByParameter.size());
    std::map<Variable, BanditInfoPtr>::const_iterator it = banditsByParameter.begin();
    const size_t n = rand->sampleSize(banditsByParameter.size());
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
    context->pushWorkUnit(new FunctionWorkUnit(problem->getObjective(), bandit->getParameter()), this, false);
    ++numPlaySent;
  }

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    ScopedLock _(lock);
    ++numPlayed;
    Variable parameter = workUnit.staticCast<FunctionWorkUnit>()->getInputs()[0];
    processResult(parameter, result.getDouble());
    playBandit(getBestBandit());

    context->enterScope(String((int)numPlayed));
    context->resultCallback(T("numPlayed"), numPlayed);
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      context->resultCallback(it->second->toString(), it->second->getPlayedCount());
    context->leaveScope();
  }

  void processResult(const Variable& parameter, double score)
  {
    std::map<Variable, BanditInfoPtr>::iterator it = banditsByParameter.find(parameter);
    if (it != banditsByParameter.end())
    {
      const BanditInfoPtr& bandit = it->second;
      double oldScore = bandit->computeScoreToMinimize();
      it->second->receiveObjectiveValue(score);
      updateBanditIndex(bandit, oldScore);
    }
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
  CriticalSection lock;
  ExecutionContext* context;
  std::multimap<double, BanditInfoPtr> banditsByScore;
  std::map<Variable, BanditInfoPtr> banditsByParameter;
};

typedef ReferenceCountedObjectPtr<BanditEDAOptimizerState> BanditEDAOptimizerStatePtr;

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
      context.leaveScope(bestInterationBandit->getObjectiveValueMean());

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
    const size_t numBanditsToRemove = (size_t)((1 - ratioOfBanditToKeep) * populationSize);
    state->removeBanditsWithWorseRewardMean(context, numBanditsToRemove);
    
    const size_t numBanditsToCreate = state->getNumBandits() != 0 ? numBanditsToRemove : populationSize;
    context.enterScope(T("Create ") + String((int)numBanditsToCreate) + (" new bandits"));
    for (size_t i = 0; i < numBanditsToCreate; ++i)
    {
      context.progressCallback(new ProgressionState(i, numBanditsToCreate, T("Bandits")));
      state->createBandit(context, state->getSampler()->sample(context, context.getRandomGenerator()));
    }
    context.progressCallback(new ProgressionState(numBanditsToCreate, numBanditsToCreate, T("Bandits")));
    context.leaveScope(numBanditsToCreate);

    context.informationCallback(T("Set budget to ") + String((int)budget) + T(" plays"));
    state->resetBudgetTo(budget);

    context.enterScope(T("Playing"));
    for (size_t i = 0; i < numSimultaneousPlays; ++i)
      state->playBandit(state->getRandomBandit(context.getRandomGenerator()));

    //context.waitUntilAllWorkUnitsAreDone();
    while (state->getNumPlayed() != budget)
    {
      context.progressCallback(new ProgressionState(state->getNumPlayed(), budget, T("Plays")));
      Thread::sleep(1000);
      context.flushCallbacks();
    }
    context.progressCallback(new ProgressionState(budget, budget, T("Plays")));
    context.leaveScope();

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

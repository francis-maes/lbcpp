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
  BanditInfo(const Variable& parameter = Variable())
    : parameter(parameter), rewardSum(0.0), squaredRewardSum(0.0), objectiveValueSum(0.0), numSamples(0) {}

  double computeScore() const // this score should be minimized (opposite of the index function)
  {
    static const double C = 2.5f;
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
    receiveReward(exp(-value / T));
    objectiveValueSum += value;
  }

  void receiveReward(double reward)
  {
    rewardSum += reward;
    squaredRewardSum += reward * reward;
    ++numSamples;
  }

  double getObjectiveValueMean() const
    {return numSamples ? objectiveValueSum / numSamples : DBL_MAX;} // FIXME: -DBL_MAX, no ?

  Variable parameter;
  double rewardSum;
  double squaredRewardSum;
  double objectiveValueSum;
  size_t numSamples;
};

typedef ReferenceCountedObjectPtr<BanditInfo> BanditInfoPtr;

class BanditEDAOptimizerState : public SamplerBasedOptimizerState, public ExecutionContextCallback
{
public:
  BanditEDAOptimizerState(const OptimizationProblemPtr& problem)
    : SamplerBasedOptimizerState(problem->getSampler()), problem(problem), context(NULL)
  {}

  void setExecutionContext(ExecutionContext& context)
    {this->context = &context;}
  
  size_t getNumBandits() const
    {return banditsByParameter.size();}

  void removeBanditsWithWorseRewardMean(size_t numBandits)
  {
    typedef std::multimap<double, BanditInfoPtr>::iterator Iterator;
    std::multimap<double, Iterator> banditsByRewardMean;
    for (Iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      banditsByRewardMean.insert(std::make_pair(-it->second->getRewardMean(), it));

    for (std::multimap<double, Iterator>::const_iterator it = banditsByRewardMean.begin(); it != banditsByRewardMean.end() && numBandits != 0; ++it, --numBandits)
    {
      banditsByParameter.erase(it->second->second->getParameter());
      banditsByScore.erase(it->second);
    }
    jassert(banditsByScore.size() == banditsByParameter.size());
  }

  void createBandit(const Variable& parameter)
  {
    BanditInfoPtr bandit(new BanditInfo(parameter));
    banditsByScore.insert(std::make_pair(bandit->computeScore(), bandit));
    banditsByParameter[parameter] = bandit;

    jassert(banditsByScore.size() == banditsByParameter.size());
  }

  BanditInfoPtr getBestBandit() const
  {
    jassert(banditsByScore.size());
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

  void setBudget(size_t budget)
    {this->budget = budget;}

  size_t getBudget() const
    {return budget;}

  void playBandit(const BanditInfoPtr& bandit)
  {
    if (budget == 0)
      return;
    --budget;
    context->pushWorkUnit(new FunctionWorkUnit(problem->getObjective(), bandit->getParameter()), this);
  }

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    Variable parameter = workUnit.staticCast<FunctionWorkUnit>()->getInputs()[0];
    processResult(parameter, result.getDouble());
    playBandit(getBestBandit());

    context->enterScope(String((int)budget));
    context->resultCallback(T("budget"), 1000 - budget);
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      context->resultCallback(it->second->getParameter().toString(), it->second->getPlayedCount());
    context->leaveScope();
  }

  void processResult(const Variable& parameter, double score)
  {
    std::map<Variable, BanditInfoPtr>::iterator it = banditsByParameter.find(parameter);
    if (it != banditsByParameter.end())
    {
      const BanditInfoPtr& bandit = it->second;
      double oldScore = bandit->computeScore();
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
    double newScore = bandit->computeScore();
    if (previousScore != newScore)
    {
      banditsByScore.erase(it);
      banditsByScore.insert(std::make_pair(newScore, bandit));
    }
  }
  
  void getParametersSortedByRewardMean(std::multimap<double, Variable>& res) const
  {
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      res.insert(std::make_pair(it->second->getRewardMean(), it->second->getParameter()));
  }

protected:
  friend class BanditEDAOptimizerStateClass;

  OptimizationProblemPtr problem;
  size_t budget;

  BanditEDAOptimizerState() {}

private:
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

      Variable bestIterationSolution;
      double bestIterationScore;
      performIteration(context, state, bestIterationSolution, bestIterationScore);

      Variable res = state->finishIteration(context, problem, i, state->getBestScore(), state->getBestSolution());
      context.leaveScope(res);

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
  
  void performIteration(ExecutionContext& context, const BanditEDAOptimizerStatePtr& state, Variable& bestIterationSolution, double& bestIterationScore) const
  {
    const size_t numBanditsToRemove = (size_t)((1 - ratioOfBanditToKeep) * populationSize);
    state->removeBanditsWithWorseRewardMean(numBanditsToRemove);
    
    const size_t numBanditsToCreate = state->getNumBandits() != 0 ? numBanditsToRemove : populationSize;
    for (size_t i = 0; i < numBanditsToCreate; ++i)
      state->createBandit(state->getSampler()->sample(context, context.getRandomGenerator()));

    state->setBudget(budget);

    for (size_t i = 0; i < numSimultaneousPlays; ++i)
      state->playBandit(state->getRandomBandit(context.getRandomGenerator()));

    //context.waitUntilAllWorkUnitsAreDone();
    while (state->getBudget() != 0)
    {
      Thread::sleep(1000);
    }

    std::multimap<double, Variable> sortedBanditScores;
    state->getParametersSortedByRewardMean(sortedBanditScores);

    learnDistribution(context, state->getSampler(), state, sortedBanditScores);

    bestIterationSolution = sortedBanditScores.rbegin()->second;
    bestIterationScore = sortedBanditScores.rbegin()->first;
    state->submitSolution(bestIterationSolution, bestIterationScore);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDIT_EDA_OPTIMIZER_H_

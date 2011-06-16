/*-----------------------------------------.---------------------------------.
| Filename: BanditEDAOptimizer.h           | Bandit based EDA Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 16/06/2011                     |                                 |
`------------------------------------------/                                 |
															 |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDIT_EDA_OPTIMIZER_H_
# define LBCPP_BANDIT_EDA_OPTIMIZER_H_

# include "../../src/Optimizer/Optimizer/PopulationBasedOptimizer.h"

namespace lbcpp
{

class BanditEDAOptimizerState : public SamplerBasedOptimizerState
{
public:
  BanditEDAOptimizerState(const SamplerPtr& sampler, double autoSaveStateFrequency = 0)
    : SamplerBasedOptimizerState(sampler, autoSaveStateFrequency) {}
  BanditEDAOptimizerState() {}

  class BanditInfo : public Object
  {
  public:
    BanditInfo(const Variable& parameter = Variable())
      : parameter(parameter), rewardSum(0.0), squaredRewardSum(0.0), objectiveValueSum(0.0), numSamples(0) {}

    double computeScore() const // this score should be minimized (opposite of the index function)
    {
      static const double C = 0.1;
      return -(numSamples ? getRewardMean() + C / (double)numSamples : DBL_MAX);
    }

    double getRewardMean() const
      {jassert(numSamples); return rewardSum / numSamples;}

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
      {return numSamples ? objectiveValueSum / numSamples : DBL_MAX;}

    Variable parameter;
    double rewardSum;
    double squaredRewardSum;
    double objectiveValueSum;
    size_t numSamples;
  };

  typedef ReferenceCountedObjectPtr<BanditInfo> BanditInfoPtr;

  void createBandit(const Variable& parameter, size_t maxNumBandits = 0)
  {
    BanditInfoPtr bandit(new BanditInfo(parameter));
    banditsByScore.insert(std::make_pair(bandit->computeScore(), bandit));
    banditsByParameter[parameter] = bandit;
    
    jassert(banditsByScore.size() == banditsByParameter.size());

    if (maxNumBandits)
      while (banditsByScore.size() > maxNumBandits)
      {
        std::multimap<double, BanditInfoPtr>::iterator it = banditsByScore.end();
        --it;
        const Variable& parameter = it->second->getParameter();
        banditsByParameter.erase(parameter);
        banditsByScore.erase(it);
      }
  }

  void playBestBandit(const OptimizerContextPtr& optimizerContext)
  {
    if (banditsByScore.size())
    {
      Variable parameter = banditsByScore.begin()->second->getParameter();
      if (optimizerContext->evaluate(parameter))
        incTotalNumberOfRequests();
    }
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

  void flushProcessedRequests()
  {
    std::vector< std::pair<double, Variable> > requests;
    {
      ScopedLock _(lock);
      this->processedRequests.swap(requests);
    }
    for (size_t i = 0; i < requests.size(); ++i)
      processResult(requests[i].second, requests[i].first);
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

  void getBestParameters(std::multimap<double, Variable>& res)
  {
    for (std::multimap<double, BanditInfoPtr>::const_iterator it = banditsByScore.begin(); it != banditsByScore.end(); ++it)
      res.insert(std::make_pair(it->second->getObjectiveValueMean(), it->second->getParameter()));
  }

private:
  std::multimap<double, BanditInfoPtr> banditsByScore;
  std::map<Variable, BanditInfoPtr> banditsByParameter;
};

typedef ReferenceCountedObjectPtr<BanditEDAOptimizerState> BanditEDAOptimizerStatePtr;

class BanditEDAOptimizer : public PopulationBasedOptimizer
{
public:  
  BanditEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, size_t maxNumBandits, StoppingCriterionPtr stoppingCriterion, bool verbose = false)
    : PopulationBasedOptimizer(numIterations, populationSize, numBests, stoppingCriterion, 0.0, false, verbose), maxNumBandits(maxNumBandits) {}

  BanditEDAOptimizer() : PopulationBasedOptimizer() {}

  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& state) const
  {     
    BanditEDAOptimizerStatePtr optimizerState = state.staticCast<BanditEDAOptimizerState>();

    // useful to restart optimizer from optimizerState
    size_t i = (size_t) (optimizerState->getTotalNumberOfResults()/populationSize); // WARNING : integer division
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));

    if (stoppingCriterion)
      stoppingCriterion->reset();
    for ( ; i < numIterations; ++i)
    {
      Variable bestIterationParameters = optimizerState->getBestVariable();
      double bestIterationScore;
      double worstIterationScore;

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      bool ok = performEDAIteration(context, bestIterationParameters, bestIterationScore, worstIterationScore, optimizerContext, optimizerState);
      if (!ok)
        return false; // FIXME : handle this
        
      // display results & update optimizerState
      handleResultOfIteration(context, optimizerState, optimizerContext, bestIterationScore, bestIterationParameters);
      context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));

      jassert(bestIterationScore <= worstIterationScore);
      if (worstIterationScore - bestIterationScore < 1e-9) // all scores are nearly identical
      {
        context.informationCallback(T("EDA has converged"));
        break;
      }
      
      if (stoppingCriterion && stoppingCriterion->shouldStop(bestIterationScore))
      {
        context.informationCallback(T("Stopping criterion: ") + stoppingCriterion->toString());
        break;
      }
    }
    optimizerState->autoSaveToFile(context, true); // force to save at end of optimization
    return optimizerState->getBestVariable();
  }
  
protected:
  friend class BanditEDAOptimizerClass;
  
  size_t numberEvaluationsInProgress; /**< Number of evaluations in progress to maintain */ 
  size_t maxNumBandits;

  bool performEDAIteration(ExecutionContext& context, Variable& bestParameters, double& bestScore, double& worstScore, const OptimizerContextPtr& optimizerContext, const BanditEDAOptimizerStatePtr& state) const
  {    
    // generate evaluations requests
    size_t offset = state->getNumberOfProcessedRequests();   // always 0 except if optimizer has been restarted from optimizerState file !
    for (size_t i = offset; i < populationSize; i++)
    {
      Variable input = sampleCandidate(context, state);
      state->createBandit(input, maxNumBandits);
    }

    size_t numBanditPlays = maxNumBandits * 10;
    for (size_t i = 0; i < numBanditPlays; ++i)
    {
      state->playBestBandit(optimizerContext);
      if (optimizerContext->isSynchroneous())
        context.progressCallback(new ProgressionState(i + 1, numBanditPlays, T("Evaluations")));
      state->flushProcessedRequests();
    }

    if (!optimizerContext->isSynchroneous())
    {
      // wait (in case of async context) & update progression
      // (waitUntilAllRequestsAreProcessed is not used to enable doing progressCallback)
      while (!optimizerContext->areAllRequestsProcessed())
      {
        Thread::sleep(optimizerContext->getTimeToSleep());
        context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), numBanditPlays, T("Evaluations")));
        state->autoSaveToFile(context);
      }
      jassert(state->getNumberOfProcessedRequests() == numBanditPlays);
      context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), numBanditPlays, T("Evaluations"))); // needed to be sure to have 100% in Explorer
    }

    // sort results
    std::multimap<double, Variable> sortedScores;
    state->getBestParameters(sortedScores);
    
    // build new distribution & update OptimizerState
    learnDistribution(context, state, sortedScores);
    
    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    worstScore = sortedScores.rbegin()->first;
    return true;
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_BANDIT_EDA_OPTIMIZER_H_

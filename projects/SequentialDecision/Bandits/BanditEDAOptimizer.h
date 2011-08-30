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

  Variable playBestBandit()
  {
    if (banditsByScore.size())
      return banditsByScore.begin()->second->getParameter();
    return Variable();
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

  void getBestParameters(const ContainerPtr& results, const std::vector<Variable>& parameters, std::multimap<double, Variable>& res)
  {
    const size_t n = results->getNumElements();
    for (size_t i = 0; i < n; ++i)
      res.insert(std::make_pair(results->getElement(i).getDouble(), parameters[i].getObjectAndCast<BanditInfo>()));
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

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    const FunctionPtr& objectiveFunction = problem->getObjective();
    const FunctionPtr& validationFunction = problem->getValidation();
  
    SamplerBasedOptimizerStatePtr state = optimizerState.dynamicCast<SamplerBasedOptimizerState>();
    jassert(state);
    
    if (stoppingCriterion)
      stoppingCriterion->reset();
    
    for (size_t i = state->getNumIterations(); i < numIterations; ++i)
    {
      Variable bestIterationSolution = state->getBestSolution();
      double bestIterationScore;
      double worstIterationScore;
      
      context.enterScope(T("Iteration ") + String((int)i + 1));
      performEDAIteration(context, state, problem->getSampler(), objectiveFunction, bestIterationSolution, bestIterationScore, worstIterationScore);
      Variable res = state->finishIteration(context, problem, i+1, bestIterationScore, bestIterationSolution);
      context.leaveScope(res);
      
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
      
      state->incrementNumIterations();
      // TODO: save state
    }
    
    return state;
  }
  
protected:
  friend class BanditEDAOptimizerClass;
  
  size_t numberEvaluationsInProgress; /**< Number of evaluations in progress to maintain */ 
  size_t maxNumBandits;

  void performEDAIteration(ExecutionContext& context, const BanditEDAOptimizerStatePtr& state, const SamplerPtr& initialSampler, const FunctionPtr& objectiveFunction, Variable& bestParameters, double& bestScore, double& worstScore) const
  {    
    // generate evaluations requests
    for (size_t i = 0; i < populationSize; i++)
    {
      Variable input = sampleCandidate(context, state);
      state->createBandit(input, maxNumBandits);
    }

    const size_t numBanditPlays = maxNumBandits * 10;
    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("BanditEDAOptimizer - ") + String((int)state->getNumIterations()));
    std::vector<Variable> parameters;
    for (size_t i = 0; i < numBanditPlays; ++i)
    {
      Variable parameter = state->playBestBandit();
      if (parameter.exists())
      {
        parameters.push_back(parameter);
        workUnits->addWorkUnit(new FunctionWorkUnit(objectiveFunction, parameter));
      }
      else
        break;
    }

    ContainerPtr results = context.run(workUnits, false).getObjectAndCast<Container>();

    for (size_t i = 0; i < parameters.size(); ++i)
      state->processResult(parameters[i], results->getElement(i).getDouble());
    
    // sort results
    std::multimap<double, Variable> sortedScores;
    state->getBestParameters(results, parameters, sortedScores);

    // build new distribution & update OptimizerState
    learnDistribution(context, initialSampler, state, sortedScores);

    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    worstScore = sortedScores.rbegin()->first;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDIT_EDA_OPTIMIZER_H_

/*-----------------------------------------.---------------------------------.
| Filename: AsyncEDAOptimizer.h            | Asynchronous EDA based Optimizer|
| Author  : Arnaud Schoofs                 | (samples to evaluate are        |
| Started : 10/04/2011                     | genrated continuously)          |
`------------------------------------------/                                 |
															 |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ASYNC_EDA_OPTIMIZER_H_
# define LBCPP_ASYNC_EDA_OPTIMIZER_H_

# include "PopulationBasedOptimizer.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class AsyncSamplerBasedOptimizerState : public SamplerBasedOptimizerState, public ExecutionContextCallback
{
public:
  AsyncSamplerBasedOptimizerState(const SamplerPtr& sampler)
    : SamplerBasedOptimizerState(sampler) {}

  /* ExecutionContextCallback */
  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(workUnitParameters.count(workUnit) == 1);
    results.insert(std::make_pair(result.getDouble(), workUnitParameters[workUnit]));
    workUnitParameters.erase(workUnit);
  }

  void sampleWorkUnit(ExecutionContext& context, const FunctionPtr& objectiveFunction)
  {
    Variable parameter = getSampler()->sample(context, context.getRandomGenerator());
    WorkUnitPtr workUnit = new FunctionWorkUnit(objectiveFunction, parameter);
    context.pushWorkUnit(workUnit, this);
  }

  size_t getNumResults() const
  {
    ScopedLock _(lock);
    return results.size();
  }

  void getAndResetResult(std::multimap<double, Variable>& sortedScores)
  {
    ScopedLock _(lock);
    sortedScores = results;
    results.clear();
  }

protected:
  CriticalSection lock;

  std::map<WorkUnitPtr, Variable> workUnitParameters;
  std::multimap<double, Variable> results;
};

typedef ReferenceCountedObjectPtr<AsyncSamplerBasedOptimizerState> AsyncSamplerBasedOptimizerStatePtr;

class AsyncEDAOptimizer : public PopulationBasedOptimizer
{
public:  
  AsyncEDAOptimizer(const SamplerPtr& sampler, size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : PopulationBasedOptimizer(sampler, numIterations, populationSize, numBests, stoppingCriterion, slowingFactor, reinjectBest, verbose)
    {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const
    {return new AsyncSamplerBasedOptimizerState(sampler);}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction) const
  {
    AsyncSamplerBasedOptimizerStatePtr state = optimizerState.dynamicCast<AsyncSamplerBasedOptimizerState>();
    jassert(state);

    bool isInitialized = false;
    for (size_t i = state->getNumIterations(); i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      
      if (!isInitialized)
      {
        for (size_t j = 0; j < populationSize; ++j)
          state->sampleWorkUnit(context, objectiveFunction);
        isInitialized = true;
      }

      size_t numPreviousResults = 0;
      while (true) // waiting loop
      {
        size_t numCurrentResults = state->getNumResults();
        // There are enough result to learn distributions
        if (numCurrentResults >= populationSize)
        {
          std::multimap<double, Variable> sortedScores;
          state->getAndResetResult(sortedScores);
          numCurrentResults = sortedScores.size();
          
          learnDistribution(context, state, sortedScores);

          Variable bestIterationParameters = sortedScores.begin()->second;
          double bestIterationScore = sortedScores.begin()->first;
          handleResultOfIteration(context, state, validationFunction, bestIterationScore, bestIterationParameters);
          
          if (stoppingCriterion && stoppingCriterion->shouldStop(bestIterationScore))
          {
            context.informationCallback(T("Stopping criterion: ") + stoppingCriterion->toString());
            break;
          }

          state->incrementNumIterations();
        }
        // Send new work units
        if (i < numIterations - 1)
          for (size_t i = 0; i < numCurrentResults - numPreviousResults; ++i)
            state->sampleWorkUnit(context, objectiveFunction);
        numPreviousResults = numCurrentResults;
        
        // break the waiting loop if we have learn distribution
        if (numCurrentResults >= populationSize)
          break;

        juce::Thread::sleep(500);
      }
      
      
      state->incrementNumIterations();
    }

    return state;
  }
  
protected:
  friend class AsyncEDAOptimizerClass;

  AsyncEDAOptimizer() : PopulationBasedOptimizer() {}
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

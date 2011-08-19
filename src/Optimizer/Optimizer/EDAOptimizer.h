/*-----------------------------------------.---------------------------------.
| Filename: EDAOptimizer.h                 | Basic EDA Optimizer             |
| Author  : Arnaud Schoofs                 | (synchronous)                   |
| Started : 06/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EDA_OPTIMIZER_H_
# define LBCPP_EDA_OPTIMIZER_H_

# include "PopulationBasedOptimizer.h"

namespace lbcpp
{

class EDAOptimizer : public PopulationBasedOptimizer
{
public:
  EDAOptimizer(const SamplerPtr& sampler, size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : PopulationBasedOptimizer(sampler, numIterations, populationSize, numBests, stoppingCriterion, slowingFactor, reinjectBest, verbose)
    {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction) const
  {
    SamplerBasedOptimizerStatePtr state = optimizerState.dynamicCast<SamplerBasedOptimizerState>();
    jassert(state);

    if (stoppingCriterion)
      stoppingCriterion->reset();

    for (size_t i = state->getNumIterations(); i < numIterations; ++i)
    {
      Variable bestIterationParameters = state->getBestParameters();
      double bestIterationScore;
      double worstIterationScore;
      
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);

      performEDAIteration(context, state, objectiveFunction, bestIterationParameters, bestIterationScore, worstIterationScore);

      // display results & update optimizerState
      handleResultOfIteration(context, state, validationFunction, bestIterationScore, bestIterationParameters);

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
      
      state->incrementNumIterations();
      // TODO: save state
    }

    return state;
  }
  
protected:
  friend class EDAOptimizerClass;
  
  EDAOptimizer()
    : PopulationBasedOptimizer() {}

  void performEDAIteration(ExecutionContext& context, const SamplerBasedOptimizerStatePtr& state, const FunctionPtr& objectiveFunction, Variable& bestParameters, double& bestScore, double& worstScore) const
  {    
    // generate evaluations requests
    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("EDAOptimizer - Iteration ") + String((int)state->getNumIterations()), populationSize);
    std::vector<Variable> inputs(populationSize);
    for (size_t i = 0; i < populationSize; ++i)
    {
      Variable input;
      if (reinjectBest && i == 0 && bestParameters.exists())
        input = bestParameters;
      else
        input = sampleCandidate(context, state);
      inputs[i] = input;
      workUnits->setWorkUnit(i, new FunctionWorkUnit(objectiveFunction, input));
    }

    ContainerPtr results = context.run(workUnits, false).getObject();

    // sort results
    std::multimap<double, Variable> sortedScores;
    pushResultsSortedbyScore(context, results, inputs, sortedScores);

    // build new distribution & update OptimizerState
    learnDistribution(context, state, sortedScores);

    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    worstScore = sortedScores.rbegin()->first;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

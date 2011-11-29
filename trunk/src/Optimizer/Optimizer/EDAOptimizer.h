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
  EDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : PopulationBasedOptimizer(numIterations, populationSize, numBests, stoppingCriterion, slowingFactor, reinjectBest, verbose)
    {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    SamplerBasedOptimizerStatePtr state = optimizerState.staticCast<SamplerBasedOptimizerState>();
    jassert(state);
    if (!state->getSampler())
      state->setSampler(problem->getSampler());

    if (stoppingCriterion)
      stoppingCriterion->reset();

    for (size_t i = state->getNumIterations(); i < numIterations; ++i)
    {
      Variable bestIterationSolution = state->getBestSolution();
      double bestIterationScore;
      double worstIterationScore;
      
      context.enterScope(T("Iteration ") + String((int)i + 1));
      performEDAIteration(context, state, problem, bestIterationSolution, bestIterationScore, worstIterationScore);
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
  friend class EDAOptimizerClass;
  
  EDAOptimizer()
    : PopulationBasedOptimizer() {}

  void performEDAIteration(ExecutionContext& context, const SamplerBasedOptimizerStatePtr& state, const OptimizationProblemPtr& problem, Variable& bestParameters, double& bestScore, double& worstScore) const
  {    
    const FunctionPtr& objectiveFunction = problem->getObjective();
    const SamplerPtr& initialSampler = problem->getSampler();

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
    pushResultsSortedbyScore(context, results, inputs, problem->isMaximisationProblem(), sortedScores);

    // build new distribution & update OptimizerState
    learnDistribution(context, initialSampler, state, sortedScores);

    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    worstScore = sortedScores.rbegin()->first;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

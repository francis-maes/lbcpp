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
  EDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : PopulationBasedOptimizer(numIterations, populationSize, numBests, slowingFactor, reinjectBest, verbose) {}
  
  EDAOptimizer() : PopulationBasedOptimizer() {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {     
    // useful to restart optimizer from optimizerState
    size_t i = (size_t) (optimizerState->getTotalNumberOfResults()/populationSize); // WARNING : integer division
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));

    for ( ; i < numIterations; ++i)
    {
      Variable bestIterationParameters = optimizerState->getBestVariable();
      double bestIterationScore;

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      bool ok = performEDAIteration(context, bestIterationParameters, bestIterationScore, optimizerContext, optimizerState);
      if (!ok)
        return false; // FIXME : handle this
      
      // display results & update optimizerState
      handleResultOfIteration(context, optimizerState, optimizerContext, bestIterationScore, bestIterationParameters);
      context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));

    }
    optimizerState->autoSaveToFile(context, true); // force to save at end of optimization
    return optimizerState->getBestVariable();
  }
  
protected:
  friend class EDAOptimizerClass;

  bool performEDAIteration(ExecutionContext& context, Variable& bestParameters, double& bestScore, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {    
    // generate evaluations requests
    size_t offset = optimizerState->getNumberOfProcessedRequests();   // always 0 except if optimizer has been restarted from optimizerState file !
    for (size_t i = offset; i < populationSize; i++) {
      Variable input;
      if (reinjectBest && i == 0 && bestParameters.exists())
        input = bestParameters;
      else
        input = sampleCandidate(context, optimizerState);
      
      if (!optimizerContext->evaluate(input))
        return false; // FIXME : handle this ?
      optimizerState->incTotalNumberOfRequests();
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
    }
    
    // wait (in case of async context) & update progression
    // (waitUntilAllRequestsAreProcessed is not used to enable doing progressCallback)
    while (!optimizerContext->areAllRequestsProcessed())
    {
      Thread::sleep(optimizerContext->getTimeToSleep());
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
      optimizerState->autoSaveToFile(context);
    }
    jassert(optimizerState->getNumberOfProcessedRequests() == populationSize);
    context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations"))); // needed to be sure to have 100% in Explorer

    // sort results
    std::multimap<double, Variable> sortedScores;
    pushResultsSortedbyScore(context, optimizerState, sortedScores);
    
    // build new distribution & update OptimizerState
    learnDistribution(context, optimizerState, sortedScores);
    
    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

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

namespace lbcpp
{

class AsyncEDAOptimizer : public PopulationBasedOptimizer
{
public:  
  AsyncEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, size_t numberEvaluationsInProgress, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : PopulationBasedOptimizer(numIterations, populationSize, numBests, slowingFactor, reinjectBest, verbose),  numberEvaluationsInProgress(numberEvaluationsInProgress)  {}

  AsyncEDAOptimizer() : PopulationBasedOptimizer() {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {   
    
    // useful to restart optimizer from state
    size_t i = (size_t) (optimizerState->getTotalNumberOfResults()/populationSize);	// interger division	
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));
    
    bool doReinjectBest = i ? true : false;  // used to know when to reinject best parameter
    
    context.enterScope(T("Iteration ") + String((int)i + 1));
    context.resultCallback(T("iteration"), i + 1);
    
    size_t totalNumberEvaluationsRequested = numIterations * populationSize;
    while (optimizerState->getTotalNumberOfResults() < totalNumberEvaluationsRequested) 
    {      
      // Send evaluation requests
      // TODO arnaud : if useless
      if (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < populationSize) 
      {
        while (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < populationSize) 
        {
          
          Variable input;
          if (reinjectBest && doReinjectBest && optimizerState->getBestVariable().exists())
          {
            input = optimizerState->getBestVariable();
            doReinjectBest = false;
          }
          else 
            input = sampleCandidate(context, optimizerState, random);
          
          if (optimizerContext->evaluate(input))
          {
            optimizerState->incTotalNumberOfRequests();
            context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
          } else
            break;  // TODO arnaud
        }
      }
      
      // don't do busy waiting
      juce::Thread::sleep(optimizerContext->getTimeToSleep());
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
      optimizerState->autoSaveToFile(context);
      
      // enough WUs evaluated -> update distribution (with best results)
      if (optimizerState->getNumberOfProcessedRequests() >= populationSize) 
      {   
        context.progressCallback(new ProgressionState(populationSize, populationSize, T("Evaluations"))); // WARNING : force display of 100% in Explorer
        
        // sort results
        std::multimap<double, Variable> sortedScores;
        pushResultsSortedbyScore(context, optimizerState, sortedScores);
        
        // build new distribution & update OptimizerState
        learnDistribution(context, optimizerState, sortedScores);

        // display results & update state
        handleResultOfIteration(context, optimizerState, optimizerContext, sortedScores.begin()->first, sortedScores.begin()->second);
        context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));
        
        i++;
        if (i < numIterations) {
          context.enterScope(T("Iteration ") + String((int)i + 1));
          context.resultCallback(T("iteration"), i + 1);
          doReinjectBest = true;
        }
      }
    }
    optimizerState->autoSaveToFile(context, true); // force to save at end of optimization
    return optimizerState->getBestVariable();
  }
  
protected:
  friend class AsyncEDAOptimizerClass;
  
  size_t numberEvaluationsInProgress;              // number of evaluations in progress to maintain  
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

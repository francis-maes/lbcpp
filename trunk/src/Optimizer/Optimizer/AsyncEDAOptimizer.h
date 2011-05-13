/*-----------------------------------------.---------------------------------.
| Filename: AsyncEDAOptimizer.h            | Asynchronous EDA based Optimizer|
| Author  : Arnaud Schoofs                 | (samples to evaluate are        |
| Started : 10/04/2011                     | genrated continuously)          |
`------------------------------------------/                                 |
															 |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ASYNC_EDA_OPTIMIZER_H_
# define LBCPP_ASYNC_EDA_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{

class AsyncEDAOptimizer : public Optimizer
{
public:  
  AsyncEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, size_t numberEvaluationsInProgress, size_t updateFactor, bool verbose = false) // TODO arnaud : rename update factor and use real instead of size_t
  : numIterations(numIterations), populationSize(populationSize), numBests(numBests), numberEvaluationsInProgress(numberEvaluationsInProgress), updateFactor(updateFactor), verbose(verbose), random(new RandomGenerator()) {}

  AsyncEDAOptimizer() : random(new RandomGenerator()) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {  
    DistributionBasedOptimizerStatePtr state = optimizerState.dynamicCast<DistributionBasedOptimizerState>();
    jassert(state);  // TODO arnaud : message erreur
    
    jassert(numBests < populationSize);
    
    size_t i = (size_t) (state->getTotalNumberOfEvaluations()/populationSize);	// interger division	
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));
    
    context.enterScope(T("Iteration ") + String((int)i + 1));
    context.resultCallback(T("iteration"), i + 1);
    
    size_t totalNumberEvaluationsRequested = numIterations * populationSize;
    while (state->getTotalNumberOfEvaluations() < totalNumberEvaluationsRequested) 
    {      
      // Send WU's on network
      if (state->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && state->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && state->getNumberOfProcessedRequests() < populationSize) 
      {
        while (state->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && state->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && state->getNumberOfProcessedRequests() < populationSize) 
        {
          Variable input = state->getDistribution()->sample(random);
          if (optimizerContext->evaluate(input))
          {
            state->incTotalNumberOfRequests();
            context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
          }          
        }
      }
      
      
      // don't do busy waiting
      juce::Thread::sleep(optimizerContext->getTimeToSleep());
      context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
      saveState(context, state);
      
      // enough WUs evaluated -> update distribution (with best results)
      if (state->getNumberOfProcessedRequests() >= populationSize) 
      {   
        context.progressCallback(new ProgressionState(populationSize, populationSize, T("Evaluations"))); // TODO arnaud : forced
        // sort results
        std::multimap<double, Variable> sortedScores;
        {
          ScopedLock _(state->getLock());
          std::vector< std::pair<double, Variable> >::const_iterator it;
          size_t i = 1;
          for (it = state->getProcessedRequests().begin(); it < state->getProcessedRequests().begin() + populationSize; it++) {  // use only populationSize results
            sortedScores.insert(*it);
            
            if (verbose) 
            {
              context.enterScope(T("Request ") + String((int) i));
              context.resultCallback(T("requestNumber"), i);
              context.resultCallback(T("parameter"), it->second);      
              context.leaveScope(it->first);
            }
            i++;  // outside if to avoid a warning for unused variable
          }
          state->flushFirstProcessedRequests(populationSize);  // TODO arnaud : maybe do that after building new distri
        }
        
        // build new distribution
        DistributionBuilderPtr distributionsBuilder = state->getDistribution()->createBuilder();
        size_t nb = 0;
        std::multimap<double, Variable>::iterator mapIt;
        // best results : use them then delete
        for (mapIt = sortedScores.begin(); mapIt != sortedScores.end() && nb < numBests; mapIt++)
        {
          distributionsBuilder->addElement(mapIt->second);  // TODO arnaud : maybe use all results and use weight
          nb++;
        }
        
        DistributionPtr newDistri = distributionsBuilder->build(context);
        distributionsBuilder->clear();
        distributionsBuilder->addDistribution(state->getDistribution());  // old distri
        for (size_t x = 0; x < updateFactor; ++x)
          distributionsBuilder->addDistribution(newDistri);
        newDistri = distributionsBuilder->build(context);
        state->setDistribution(newDistri);        
        context.resultCallback(T("distribution"), newDistri);
        
        if (sortedScores.begin()->first < state->getBestScore()) 
        {
          ScopedLock _(state->getLock());
          state->setBestScore(sortedScores.begin()->first);
          state->setBestVariable(sortedScores.begin()->second);
        }
        context.resultCallback(T("bestScore"), state->getBestScore());
        context.resultCallback(T("bestParameters"), state->getBestVariable());
        context.leaveScope(sortedScores.begin()->first); // may be diffrent from state->getBestScore(), this is the return value of performEDAIteration, not the best score of all time !!!
        context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));
        
        i++;
        if (i < numIterations) {
          context.enterScope(T("Iteration ") + String((int)i + 1));
          context.resultCallback(T("iteration"), i + 1);
        }
        
        saveState(context, state);
      }
    }
    
    return state->getBestVariable();
  }
  
protected:
  friend class AsyncEDAOptimizerClass;
  
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
  size_t numberEvaluationsInProgress;              // number of evaluations in progress to maintain
  size_t updateFactor;                    // preponderance of new distri vs old distri (low value avoid too quick convergence)
  bool verbose;
  RandomGeneratorPtr random;
  
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

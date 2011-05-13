/*-----------------------------------------.---------------------------------.
| Filename: EDAOptimizer.h                 | Basic EDA Optimizer             |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 06/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EDA_OPTIMIZER_H_
# define LBCPP_EDA_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{

class EDAOptimizer : public Optimizer
{
public:
  EDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, bool reinjectBest = false, bool verbose = false)
    : numIterations(numIterations), populationSize(populationSize), numBests(numBests), reinjectBest(reinjectBest), verbose(verbose), random(new RandomGenerator()) {}
  
  EDAOptimizer()
    : numIterations(0), populationSize(0), numBests(0), reinjectBest(false), verbose(false), random(new RandomGenerator()) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {     
    size_t i = (size_t) (optimizerState->getTotalNumberOfEvaluations()/populationSize);	// integer division
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));

    for ( ; i < numIterations; ++i)
    {
      Variable bestIterationParameters = optimizerState->getBestVariable();
      double bestIterationScore;

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      bool ok = performEDAIteration(context, bestIterationParameters, optimizerContext, optimizerState, bestIterationScore, verbose);
      if (!ok)
        return false;
      
      // update OptimizerState if necessary
      {
        if (bestIterationScore < optimizerState->getBestScore())
        {
          ScopedLock _(optimizerState->getLock());
          optimizerState->setBestScore(bestIterationScore);
          optimizerState->setBestVariable(bestIterationParameters);
        }
      }
      
      context.resultCallback(T("bestScore"), optimizerState->getBestScore());
      context.resultCallback(T("bestParameters"), optimizerState->getBestVariable());
      context.leaveScope(bestIterationScore); // may be diffrent from optimizerState->getBestScore(), this is the return value of performEDAIteration, not the best score of all time !!!
      context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));
      
      saveState(context, optimizerState);
    }
    return optimizerState->getBestVariable();
  }
  
protected:
  friend class EDAOptimizerClass;
  
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
  bool reinjectBest;
  bool verbose;
  RandomGeneratorPtr random;
  
  bool performEDAIteration(ExecutionContext& context, Variable& bestParameters, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& state, double& bestScore, bool verbose = false) const
  {
    DistributionBasedOptimizerStatePtr optimizerState = state.dynamicCast<DistributionBasedOptimizerState>();
    jassert(optimizerState);  // TODO arnaud : message erreur
    
    jassert(numBests < populationSize);
    
    // generate evaluations requests
    size_t offset = optimizerState->getNumberOfProcessedRequests();   // always 0 except if optimizer has been restarted from file !
    for (size_t i = offset; i < populationSize; i++) {  // init condition used to restart from file
      Variable input;
      if (reinjectBest && i == 0 && bestParameters.exists())
        input = bestParameters;
      else 
        input = optimizerState->getDistribution()->sample(random);
      
      if (!optimizerContext->evaluate(input))
        return false; // TODO arnaud : handle this
      optimizerState->incTotalNumberOfRequests();
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
    }
    
    // wait (in case of async context) & update progression
    while (!optimizerContext->areAllRequestsProcessed())
    {
      Thread::sleep(optimizerContext->getTimeToSleep());
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
      saveState(context, optimizerState);
    }
    jassert(optimizerState->getNumberOfProcessedRequests() == populationSize);
    context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations"))); // needed to be sure to have 100% in Explorer

    // sort results
    std::multimap<double, Variable> sortedScores;
    {
      ScopedLock _(optimizerState->getLock());
      std::vector< std::pair<double, Variable> >::const_iterator it;
      size_t i = 1;
      for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().end(); it++)
      {
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
      optimizerState->flushProcessedRequests();  // TODO arnaud : maybe do that after building new distri
    }
    
    // build new distribution & update OptimizerState
    std::multimap<double, Variable>::const_iterator it2 = sortedScores.begin();
    DistributionBuilderPtr builder = optimizerState->getDistribution()->createBuilder();
    for (size_t i = 0; i < numBests && it2 != sortedScores.end(); ++i, ++it2)
      builder->addElement(it2->second);
    DistributionPtr newDistribution = builder->build(context);
    optimizerState->setDistribution(newDistribution);
    context.resultCallback(T("distribution"), newDistribution);
    
    // return best score and best parameter of this iteration
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    return true;
  }
};

typedef ReferenceCountedObjectPtr<EDAOptimizer> EDAOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

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

// TODO arnaud : add progression callback, enter/leavescope callback
class AsyncEDAOptimizer : public Optimizer
{
public:
  AsyncEDAOptimizer(size_t totalNumberEvaluationsRequested, size_t numberEvaluationsToUpdate, size_t ratioUsedForUpdate, size_t timeToSleep, size_t updateFactor, size_t numberEvaluationsInProgress, bool verbose = false)
    : totalNumberEvaluationsRequested(totalNumberEvaluationsRequested), numberEvaluationsToUpdate(numberEvaluationsToUpdate),
      ratioUsedForUpdate(ratioUsedForUpdate), timeToSleep(timeToSleep), updateFactor(updateFactor), numberEvaluationsInProgress(numberEvaluationsInProgress), verbose(false) 
    {random = RandomGenerator::getInstance();}
  
  AsyncEDAOptimizer() : verbose(false) 
    {random = RandomGenerator::getInstance();}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {  
    // TODO arnaud : save initial state
    size_t numIterations = (size_t) ceil((double)totalNumberEvaluationsRequested/(double)numberEvaluationsToUpdate);
    size_t i = 0;  // TODO arnaud : rename
    context.enterScope(T("Iteration ") + String((int)i + 1));
    context.resultCallback(T("iteration"), i + 1);
    
    while (optimizerState->getTotalNumberOfEvaluations() < totalNumberEvaluationsRequested) 
    {      
      // Send WU's on network
      if (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < numberEvaluationsToUpdate) 
      {
        size_t nb = 0;
        while (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < numberEvaluationsToUpdate) 
        {
          Variable input = optimizerState->getDistribution()->sample(random);
          if (optimizerContext->evaluate(context, input))
          {
            optimizerState->incTotalNumberOfRequests();
            nb++;
            context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), numberEvaluationsToUpdate, T("Evaluations")));
          }          
        }
        
        if (nb > 0) 
        {
          // TODO arnaud : save state
        }
      }
      
      
      // don't do busy waiting
      juce::Thread::sleep(timeToSleep*10);
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), numberEvaluationsToUpdate, T("Evaluations")));
      
      // enough WUs evaluated -> update distribution (with best results)
      if (optimizerState->getNumberOfProcessedRequests() >= numberEvaluationsToUpdate/* || (optimizerState->getTotalNumberOfRequests() == totalNumberEvaluationsRequested && optimizerState->getNumberOfInProgressEvaluations() == 0)*/) 
      {   
        //std::cout << "HERE:" << optimizerState->getNumberOfProcessedRequests() << std::endl;
        context.progressCallback(new ProgressionState(numberEvaluationsToUpdate, numberEvaluationsToUpdate, T("Evaluations"))); // TODO arnaud : forced
        // sort results
        std::multimap<double, Variable> sortedScores;
        {
          ScopedLock _(optimizerState->getLock());
          std::vector< std::pair<double, Variable> >::const_iterator it;
          size_t i = 1;
          for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().begin() + numberEvaluationsToUpdate; it++) {  // use only numberEvaluationsToUpdate results
            //std::cout << "HERE 2 : " << i << std::endl;
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
          optimizerState->flushFirstProcessedRequests(numberEvaluationsToUpdate);  // TODO arnaud : maybe do that after building new distri
        }
        
        // build new distribution
        DistributionBuilderPtr distributionsBuilder = optimizerState->getDistribution()->createBuilder();
        size_t nb = 0;
        std::multimap<double, Variable>::iterator mapIt;
        // best results : use them then delete
        for (mapIt = sortedScores.begin(); mapIt != sortedScores.end() && nb < sortedScores.size()/ratioUsedForUpdate; mapIt++)   // TODO arnaud: au lieu d'un ratio faire comme dans EDAOptimizer
        {
          distributionsBuilder->addElement(mapIt->second);  // TODO arnaud : maybe use all results and use weight
          nb++;
        }
        
        DistributionPtr newDistri = distributionsBuilder->build(context);
        distributionsBuilder->clear();
        distributionsBuilder->addDistribution(optimizerState->getDistribution());  // old distri
        for (size_t x = 0; x < updateFactor; ++x)
          distributionsBuilder->addDistribution(newDistri);
        newDistri = distributionsBuilder->build(context);
        optimizerState->setDistribution(newDistri);        
        context.resultCallback(T("distribution"), newDistri);
        
        if (sortedScores.begin()->first < optimizerState->getBestScore()) 
        {
          ScopedLock _(optimizerState->getLock());
          optimizerState->setBestScore(sortedScores.begin()->first);
          optimizerState->setBestVariable(sortedScores.begin()->second);
        }
        context.resultCallback(T("bestScore"), optimizerState->getBestScore());
        context.resultCallback(T("bestParameters"), optimizerState->getBestVariable());
        context.leaveScope(sortedScores.begin()->first); // may be diffrent from optimizerState->getBestScore(), this is the return value of performEDAIteration, not the best score of all time !!!
        context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));
        
        i++;
        if (i < numIterations) {
          context.enterScope(T("Iteration ") + String((int)i + 1));
          context.resultCallback(T("iteration"), i + 1);
        }
        
                
        // TODO arnaud : save state
      }
    }
    
    return optimizerState->getBestVariable();
  }
  
protected:
  friend class AsyncEDAOptimizerClass;
  
  RandomGeneratorPtr random;
  
  size_t totalNumberEvaluationsRequested; // total number of Variable to evaluate
  size_t numberEvaluationsToUpdate;       // min number of results needed to update distribution
  size_t ratioUsedForUpdate;              // number of results used to calculate new distribution is numberWuToUpdate/ratioUsedForUpdate 
  size_t timeToSleep;                     // time to sleep to avoid busy waiting
  size_t updateFactor;                    // preponderance of new distri vs old distri (low value avoid too quick convergence)
  size_t numberEvaluationsInProgress;              // number of evaluations in progress to maintain
  
  bool verbose;
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

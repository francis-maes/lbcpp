/*
 *  AsyncEDAOptimizer.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 10/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_ASYNC_EDA_OPTIMIZER_H_
# define LBCPP_ASYNC_EDA_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{
  
class AsyncEDAOptimizer : public Optimizer
{
public:
  AsyncEDAOptimizer(size_t totalNumberEvaluationsRequested, size_t numberEvaluationsToUpdate, size_t ratioUsedForUpdate, size_t timeToSleep, size_t updateFactor, size_t numberEvaluationsInProgress)
    : totalNumberEvaluationsRequested(totalNumberEvaluationsRequested), numberEvaluationsToUpdate(numberEvaluationsToUpdate),
    ratioUsedForUpdate(ratioUsedForUpdate), timeToSleep(timeToSleep), updateFactor(updateFactor), numberEvaluationsInProgress(numberEvaluationsInProgress) 
    {random = RandomGenerator::getInstance();}
  
  AsyncEDAOptimizer() 
    {random = RandomGenerator::getInstance();}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {  
    // TODO arnaud : save initial state
    
    while (optimizerState->getTotalNumberOfEvaluations() < totalNumberEvaluationsRequested) 
    {      
      // Send WU's on network
      if (optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && (optimizerState->getNumberOfUnprocessedEvaluations() < numberEvaluationsToUpdate) && (optimizerState->getTotalNumberOfRequests() - optimizerState->getTotalNumberOfEvaluations()) < numberEvaluationsInProgress) 
      {
        context.informationCallback(T("Asking evaluations ..."));
        size_t nb = 0;
        while (optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && (optimizerState->getNumberOfUnprocessedEvaluations() < numberEvaluationsToUpdate) && (optimizerState->getTotalNumberOfRequests() - optimizerState->getTotalNumberOfEvaluations()) < numberEvaluationsInProgress) 
        {
          Variable input = optimizerState->getDistribution()->sample(random);
          if (!optimizerContext->evaluate(input))
            break;
          optimizerState->incTotalNumberOfRequests();
          nb++;
        }
        context.informationCallback(T("Asking evaluations ... : ") + String((int) nb) + T(" evaluations requested!"));
        
        
        if (nb > 0) 
        {
          // TODO arnaud : save state
        }
      }
      
      
      // don't do busy waiting
      juce::Thread::sleep(timeToSleep*1000);
      std::cout << "HERE" << std::endl;
      
      
      // enough WUs evaluated -> update distribution (with best results)
      if (optimizerState->getNumberOfUnprocessedEvaluations() >= numberEvaluationsToUpdate || (optimizerState->getTotalNumberOfRequests() == totalNumberEvaluationsRequested && (optimizerState->getTotalNumberOfRequests() - optimizerState->getTotalNumberOfEvaluations()) == 0)) 
      {
        context.informationCallback(T("Updating state ..."));
        
        // sort results
        std::multimap<double, Variable> sortedScores;
        {
          ScopedLock _(optimizerState->getLock());
          std::vector< std::pair<double, Variable> >::const_iterator it;
          for (it = optimizerState->getUnprocessedEvaluations().begin(); it < optimizerState->getUnprocessedEvaluations().end(); it++)
            sortedScores.insert(*it);
          optimizerState->clearUnprocessedEvaluations();  // TODO arnaud : maybe do that after building new distri
        }
        
        
        DistributionBuilderPtr distributionsBuilder = optimizerState->getDistribution()->createBuilder();
        size_t nb = 0;
        std::multimap<double, Variable>::iterator mapIt;
        // best results : use them then delete
        for (mapIt = sortedScores.begin(); mapIt != sortedScores.end() && nb < sortedScores.size()/ratioUsedForUpdate; mapIt++)
        {
          distributionsBuilder->addElement(mapIt->second);  // TODO arnaud : maybe use all results and use weight
          nb++;
        }
        
        DistributionPtr newDistri = distributionsBuilder->build(context);
        distributionsBuilder->clear();
        distributionsBuilder->addDistribution(optimizerState->getDistribution());  // old distri
        for (size_t i = 0; i < updateFactor; ++i)
          distributionsBuilder->addDistribution(newDistri);
        optimizerState->setDistribution(distributionsBuilder->build(context));        
        
        if (sortedScores.begin()->first < optimizerState->getBestScore()) {
          optimizerState->setBestScore(sortedScores.begin()->first);
          optimizerState->setBestVariable(sortedScores.begin()->second);
          context.informationCallback(T("New best variable found : ") + optimizerState->getBestVariable().toString() + T(" ( ") + String(optimizerState->getBestScore()) + T(" )"));
        }
        
        context.informationCallback(T("State updated"));
        
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
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

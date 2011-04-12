/*
 *  EDAOptimizer.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 6/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef LBCPP_EDA_OPTIMIZER_H_
# define LBCPP_EDA_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{

class EDAOptimizer : public Optimizer
{
public:
  EDAOptimizer(size_t nbIterations, size_t populationSize, size_t numBests)
    : nbIterations(nbIterations), populationSize(populationSize), numBests(numBests)
    {random = RandomGenerator::getInstance();}
  
  EDAOptimizer() 
    {random = RandomGenerator::getInstance();}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {  
    for (size_t i = 0; i < nbIterations; ++i)
    {

      Variable bestIterationParameters = optimizerState->getBestVariable();
      double score = performEDAIteration(context, bestIterationParameters, optimizerContext, optimizerState);
      if (score < optimizerState->getBestScore())
        optimizerState->setBestRequest(score, bestIterationParameters);
      context.progressCallback(new ProgressionState(i + 1, nbIterations, T("Iterations")));
    }
    return optimizerState->getBestScore();
  }
  
protected:
  friend class EDAOptimizerClass;
  
private:
  RandomGeneratorPtr random;
  size_t nbIterations;
  size_t populationSize;
  size_t numBests;
  
  
  double performEDAIteration(ExecutionContext& context, Variable& bestParameters, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {
    jassert(numBests < populationSize);
    
    // generate evaluations
    for (size_t i = 0; i < populationSize; ++i)
    {
      Variable input;
      if (i == 0 && bestParameters.exists())
        input = bestParameters;
      else 
      {
        input = optimizerState->getDistribution()->sample(random);
        
      }
      if (!optimizerContext->evaluate(context, input))
        i--;
      else
        optimizerState->incTotalNumberOfRequests();
    }
    
    // wait and sort results
    optimizerContext->waitUntilAllRequestsAreProcessed();
    std::multimap<double, Variable> sortedScores;
    {
      ScopedLock _(optimizerState->getLock());
      
      // TODO arnaud : uncomment AND debug !
      // jassert(optimizerState->getNumberOfProcessedRequests() == populationSize);
      
      /*while (optimizerState->getNumberOfProcessedRequests() != populationSize)
      {
        std::cout << optimizerState->getNumberOfProcessedRequests() << " VS " << populationSize << std::endl;
        Thread::sleep(1000);
      }*/
      
      // sort results
      std::vector< std::pair<double, Variable> >::const_iterator it;
      for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().end(); it++)
        sortedScores.insert(*it);
      optimizerState->flushProcessedRequests();  // TODO arnaud : maybe do that after building new distri
    }
    
    // build new distribution
    std::multimap<double, Variable>::const_iterator it2 = sortedScores.begin();
    DistributionBuilderPtr builder = optimizerState->getDistribution()->createBuilder();
    for (size_t i = 0; i < numBests; ++i, ++it2)
      builder->addElement(it2->second);
    optimizerState->setDistribution(context, builder->build(context));
    
    // return best score
    bestParameters = sortedScores.begin()->second;
    double bestScore = sortedScores.begin()->first;
    return bestScore;
  }
};

typedef ReferenceCountedObjectPtr<EDAOptimizer> EDAOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

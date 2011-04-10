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
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Distribution/DistributionBuilder.h>


// TODO arnaud : modified to use new Function interface but not tested yet

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
    //std::cout << "HERE" << std::endl;
    //optimizerContext->evaluate(4.0, optimizerState);
    context.enterScope(T("Optimizing"));
    std::cout << optimizerState->distribution->toString() << std::endl;
    for (size_t i = 0; i < nbIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      Variable bestIterationParameters = optimizerState->bestVariable;
      double score = performEDAIteration(context, bestIterationParameters, optimizerContext, optimizerState);
      context.resultCallback(T("bestParameters"), bestIterationParameters);
      std::cout << optimizerState->distribution->toString() << std::endl;
      
      context.leaveScope(score);
      if (score < optimizerState->bestScore)
      {
        optimizerState->bestScore = score;
        optimizerState->bestVariable = bestIterationParameters;
      }
      context.progressCallback(new ProgressionState(i + 1, nbIterations, T("Iterations")));
    }
    context.resultCallback(T("bestParameters"), optimizerState->bestVariable);
    context.leaveScope(optimizerState->bestScore);
    return optimizerState->bestScore;
    return Variable();
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
        input = optimizerState->distribution->sample(random);
        
      }
      optimizerContext->evaluate(input);
      optimizerState->totalNumberGeneratedWUs++;
    }
    
    // wait and sort results
    optimizerContext->waitAllEvaluationsFinished();
    
    std::multimap<double, Variable> sortedScores;
    {
      ScopedLock _(optimizerState->lock);
      jassert(optimizerState->currentEvaluatedWUs.size() == populationSize);
      
      // sort results
      std::vector< std::pair<double, Variable> >::iterator it;
      for (it = optimizerState->currentEvaluatedWUs.begin(); it < optimizerState->currentEvaluatedWUs.end(); it++)
        sortedScores.insert(*it);
      optimizerState->currentEvaluatedWUs.clear();
    }
    
    // build new distribution
    std::multimap<double, Variable>::const_iterator it2 = sortedScores.begin();
    DistributionBuilderPtr builder = optimizerState->distribution->createBuilder();
    for (size_t i = 0; i < numBests; ++i, ++it2)
      builder->addElement(it2->second);
    optimizerState->distribution = builder->build(context);
    
    // return best score
    bestParameters = sortedScores.begin()->second;
    double bestScore = sortedScores.begin()->first;
    return bestScore;
  }
};

typedef ReferenceCountedObjectPtr<EDAOptimizer> EDAOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

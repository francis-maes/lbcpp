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
# include <lbcpp/Distribution/DistributionBuilder.h> // old
# include <lbcpp/Sampler/Sampler.h> // new

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
    jassert(numBests < populationSize);
    
    size_t i = (size_t) (optimizerState->getTotalNumberOfEvaluations()/populationSize);	// interger division	
    context.progressCallback(new ProgressionState(i, numIterations, T("Iterations")));
    
    context.enterScope(T("Iteration ") + String((int)i + 1));
    context.resultCallback(T("iteration"), i + 1);
    
    size_t totalNumberEvaluationsRequested = numIterations * populationSize;
    while (optimizerState->getTotalNumberOfEvaluations() < totalNumberEvaluationsRequested) 
    {      
      // Send WU's on network
      if (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < populationSize) 
      {
        while (optimizerState->getNumberOfInProgressEvaluations() < numberEvaluationsInProgress && optimizerState->getTotalNumberOfRequests() < totalNumberEvaluationsRequested && optimizerState->getNumberOfProcessedRequests() < populationSize) 
        {
          Variable input = sampleCandidate(context, optimizerState, random);
          if (optimizerContext->evaluate(input))
          {
            optimizerState->incTotalNumberOfRequests();
            context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
          }          
        }
      }
      
      
      // don't do busy waiting
      juce::Thread::sleep(optimizerContext->getTimeToSleep());
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), populationSize, T("Evaluations")));
      optimizerState->autoSaveToFile(context);
      
      // enough WUs evaluated -> update distribution (with best results)
      if (optimizerState->getNumberOfProcessedRequests() >= populationSize) 
      {   
        context.progressCallback(new ProgressionState(populationSize, populationSize, T("Evaluations"))); // TODO arnaud : forced
        // sort results
        std::multimap<double, Variable> sortedScores;
        {
          ScopedLock _(optimizerState->getLock());
          std::vector< std::pair<double, Variable> >::const_iterator it;
          size_t i = 1;
          for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().begin() + populationSize; it++) {  // use only populationSize results
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
          optimizerState->flushFirstProcessedRequests(populationSize);  // TODO arnaud : maybe do that after building new distri
        }
        
        // build new distribution & update OptimizerState
        learnDistribution(context, optimizerState, sortedScores, updateFactor);

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
        
        optimizerState->autoSaveToFile(context);
      }
    }
    optimizerState->autoSaveToFile(context, true); // force to save at end of optimization
    return optimizerState->getBestVariable();
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
  
  Variable sampleCandidate(ExecutionContext& context, const OptimizerStatePtr& state, const RandomGeneratorPtr& random) const
  {
    // TODO: replace by state.staticCast<SamplerBasedOptimizerState>()->getSampler()->sample(context, random);
    
    // new
    SamplerBasedOptimizerStatePtr samplerBasedState = state.dynamicCast<SamplerBasedOptimizerState>();
    if (samplerBasedState)
      return samplerBasedState->getSampler()->sample(context, random);
    
    // old
    DistributionBasedOptimizerStatePtr distributionBasedState = state.dynamicCast<DistributionBasedOptimizerState>();
    if (distributionBasedState)
      return distributionBasedState->getDistribution()->sample(random);
    
    jassert(false);
    return Variable();
  }  
  
  void learnDistribution(ExecutionContext& context, const OptimizerStatePtr& state, const std::multimap<double, Variable>& sortedScores, size_t updateFactor) const
  {
    // new
    SamplerBasedOptimizerStatePtr samplerBasedState = state.dynamicCast<SamplerBasedOptimizerState>();
    if (samplerBasedState)
    {
      SamplerPtr oldSampler = samplerBasedState->getSampler();
      SamplerPtr newSampler = oldSampler->cloneAndCast<Sampler>();
      
      VectorPtr bestVariables = vector(sortedScores.begin()->second.getType());
      bestVariables->reserve(numBests);
      
      std::multimap<double, Variable>::const_iterator it = sortedScores.begin();
      for (size_t i = 0; i < numBests && it != sortedScores.end(); ++i, ++it)
        bestVariables->append(it->second);
      newSampler->learn(context, ContainerPtr(), bestVariables);
      
      DenseDoubleVectorPtr probas = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, 2);
      probas->setValue(0, 1.0/(1.0+updateFactor));
      probas->setValue(1, updateFactor/(1.0+updateFactor));
      std::vector<SamplerPtr> vec;
      vec.reserve(2);
      vec.push_back(oldSampler);
      vec.push_back(newSampler);
      samplerBasedState->setSampler(mixtureSampler(probas, vec));
      
      context.resultCallback(T("sampler"), samplerBasedState->getSampler());
      return;
    }
    
    // old
    DistributionBasedOptimizerStatePtr distributionBasedState = state.dynamicCast<DistributionBasedOptimizerState>();
    if (distributionBasedState)
    {
      // build new distribution & update OptimizerState
      DistributionBuilderPtr builder = distributionBasedState->getDistribution()->createBuilder();
      std::multimap<double, Variable>::const_iterator it = sortedScores.begin();
      for (size_t i = 0; i < numBests && it != sortedScores.end(); ++i, ++it)
        builder->addElement(it->second);
      DistributionPtr newDistribution = builder->build(context);
      
      builder->clear();
      builder->addDistribution(distributionBasedState->getDistribution());  // old distri
      for (size_t x = 0; x < updateFactor; ++x)
        builder->addDistribution(newDistribution);
      
      distributionBasedState->setDistribution(builder->build(context));
      context.resultCallback(T("distribution"), newDistribution);      
      return;
    }
    
    jassert(false);
  }
  
};

typedef ReferenceCountedObjectPtr<AsyncEDAOptimizer> AsyncEDAOptimizerPtr;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_ASYNC_EDA_OPTIMIZER_H_

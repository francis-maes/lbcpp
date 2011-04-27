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
  EDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, bool reinjectBest = false)
    : numIterations(numIterations), populationSize(populationSize), numBests(numBests), reinjectBest(reinjectBest), random(new RandomGenerator()) {}
  
  EDAOptimizer()
    : numIterations(0), populationSize(0), numBests(0) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {  
    for (size_t i = 0; i < numIterations; ++i)
    {
      Variable bestIterationParameters = optimizerState->getBestVariable();
      double bestScore;

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      bool ok = performEDAIteration(context, bestIterationParameters, optimizerContext, optimizerState, bestScore);
      if (!ok)
        return false;
      
      if (bestScore < optimizerState->getBestScore())
      {
        ScopedLock _(optimizerState->getLock());  // TODO arnaud : tt block scoped ?
        optimizerState->setBestScore(bestScore);
        optimizerState->setBestVariable(bestIterationParameters);
      }
      
      context.resultCallback(T("bestScore"), optimizerState->getBestScore());
      context.resultCallback(T("bestParameters"), optimizerState->getBestVariable());
      context.leaveScope(optimizerState->getBestScore());
      context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iterations")));
    }
    return optimizerState->getBestScore();
  }
  
protected:
  friend class EDAOptimizerClass;
  
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
  bool reinjectBest;
  RandomGeneratorPtr random;
  
  bool performEDAIteration(ExecutionContext& context, Variable& bestParameters, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState, double& bestScore) const
  {
    jassert(numBests < populationSize);
    
    // generate evaluations
    std::vector<Variable> parametersVector(populationSize);
    for (size_t i = 0; i < populationSize; ++i)
    {
      Variable input;
      if (reinjectBest && i == 0 && bestParameters.exists())
        input = bestParameters;
      else 
        input = optimizerState->getDistribution()->sample(random);

      parametersVector[i] = input;
      optimizerState->incTotalNumberOfRequests();
    }

    if (!optimizerContext->evaluate(context, parametersVector))
      return false;
    
    // wait and sort results
    optimizerContext->waitUntilAllRequestsAreProcessed(context);
    
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
    for (size_t i = 0; i < numBests && it2 != sortedScores.end(); ++i, ++it2)
      builder->addElement(it2->second);
    optimizerState->setDistribution(context, builder->build(context));
    
    // return best score
    bestParameters = sortedScores.begin()->second;
    bestScore = sortedScores.begin()->first;
    return true;
  }
};

typedef ReferenceCountedObjectPtr<EDAOptimizer> EDAOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_EDA_OPTIMIZER_H_

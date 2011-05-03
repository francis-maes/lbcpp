/*-----------------------------------------.---------------------------------.
| Filename: UniformSampleAndPickBestOpti..h| Uniform sample and pick best    |
| Author  : Francis Maes, Arnaud Schoofs   |  optimizer                      |
| Started : 21/12/2010 23:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_
# define LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/ContinuousDistribution.h> // TODO arnaud : should no be here !!!

namespace lbcpp
{

  
// TODO arnaud : add progression callback, enter/leavescope callback
class UniformSampleAndPickBestOptimizer : public Optimizer
{
public:
  UniformSampleAndPickBestOptimizer(size_t numSamples = 0, bool verbose = false)
    : numSamples(numSamples), verbose(verbose) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {   
    std::vector<double> values;
    ContinuousDistributionPtr apriori = optimizerState->getDistribution().dynamicCast<ContinuousDistribution>();
    apriori->sampleUniformly(numSamples, values);
    
    for (size_t i = 0; i < numSamples; ++i) 
    {
      if (!optimizerContext->evaluate(context, values[i]))
        i--;
      else 
      {
        optimizerState->incTotalNumberOfRequests();
        context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), numSamples, T("Evaluations")));
      }
    }
    
    // wait (in case of async context) & update progression
    while (!optimizerContext->areAllRequestsProcessed()) {
      Thread::sleep(60000);
      context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), numSamples, T("Evaluations")));
    }
    jassert(optimizerState->getNumberOfProcessedRequests() == numSamples);
    context.progressCallback(new ProgressionState(optimizerState->getNumberOfProcessedRequests(), numSamples, T("Evaluations"))); // needed to be sure to have 100% in Explorer
    
    std::vector< std::pair<double, Variable> >::const_iterator it;
    {
      ScopedLock _(optimizerState->getLock());
      size_t i = 1;
      for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().end(); it++)
      {
        if (it->first < optimizerState->getBestScore())
        {
          optimizerState->setBestScore(it->first);
          optimizerState->setBestVariable(it->second);
        }        
        if (verbose) 
        {
          context.enterScope(T("Request ") + String((int) i));
          context.resultCallback(T("requestNumber"), i);
          context.resultCallback(T("parameter"), it->second);      
          context.leaveScope(it->first);
        }
        i++;  // outside if to avoid a warning for unused variable
      }
      optimizerState->flushProcessedRequests();
    }
    return optimizerState->getBestScore();
 }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
  bool verbose;
};

typedef ReferenceCountedObjectPtr<UniformSampleAndPickBestOptimizer> UniformSampleAndPickBestOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

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
  UniformSampleAndPickBestOptimizer(size_t numSamples = 0)
    : numSamples(numSamples) {}
  
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
        optimizerState->incTotalNumberOfRequests();
    }
    
    optimizerContext->waitUntilAllRequestsAreProcessed(context);
    
    std::vector< std::pair<double, Variable> >::const_iterator it;
    {
      ScopedLock _(optimizerState->getLock());
      jassert(optimizerState->getNumberOfProcessedRequests() == numSamples);
      for (it = optimizerState->getProcessedRequests().begin(); it < optimizerState->getProcessedRequests().end(); it++)
      {
        if (it->first < optimizerState->getBestScore())
        {
          ScopedLock _(optimizerState->getLock());  // TODO arnaud : tt block scoped ?
          optimizerState->setBestScore(it->first);
          optimizerState->setBestVariable(it->second);
        }
      }
      optimizerState->flushProcessedRequests();
    }
    return optimizerState->getBestScore();
 }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
};

typedef ReferenceCountedObjectPtr<UniformSampleAndPickBestOptimizer> UniformSampleAndPickBestOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

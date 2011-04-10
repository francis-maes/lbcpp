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
# include <lbcpp/Distribution/ContinuousDistribution.h>


// TODO arnaud : modified to use new Function interface but not tested yet

namespace lbcpp
{

class UniformSampleAndPickBestOptimizer : public Optimizer
{
public:
  UniformSampleAndPickBestOptimizer(size_t numSamples = 0)
    : numSamples(numSamples) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {   
    std::vector<double> values;
    ContinuousDistributionPtr apriori = optimizerState->distribution.dynamicCast<ContinuousDistribution>();
    apriori->sampleUniformly(numSamples, values);
    
    for (size_t i = 0; i < numSamples; ++i) 
    {
      optimizerContext->evaluate(values[i]);
      optimizerState->totalNumberGeneratedWUs++;
    }
    
    optimizerContext->waitAllEvaluationsFinished();
    
    // TODO arnaud : check results.size() == requests.size()    
    {
      ScopedLock _(optimizerState->lock);
      std::vector< std::pair<double, Variable> >::iterator it;
      for (it = optimizerState->currentEvaluatedWUs.begin(); it < optimizerState->currentEvaluatedWUs.end(); it++) {
        if (it->first < optimizerState->bestScore) {
          optimizerState->bestScore = it->first;
          optimizerState->bestVariable = it->second;
        }
      }
    }
    
    std::cout << "Best Score: " << optimizerState->bestScore << " (" << optimizerState->bestVariable << ")" << std::endl;
    return optimizerState->bestScore;
 }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
};

typedef ReferenceCountedObjectPtr<UniformSampleAndPickBestOptimizer> UniformSampleAndPickBestOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

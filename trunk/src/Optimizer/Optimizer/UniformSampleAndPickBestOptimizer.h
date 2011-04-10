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
    ContinuousDistributionPtr apriori = optimizerState->getDistribution().dynamicCast<ContinuousDistribution>();
    apriori->sampleUniformly(numSamples, values);
    
    for (size_t i = 0; i < numSamples; ++i) 
    {
      optimizerContext->evaluate(values[i]);
      optimizerState->incTotalNumberOfRequests();
    }
    
    optimizerContext->waitAllEvaluationsFinished();
    
    std::vector< std::pair<double, Variable> >::const_iterator it;
    {
      ScopedLock _(optimizerState->getLock());
      jassert(optimizerState->getNumberOfUnprocessedEvaluations() == numSamples);
      for (it = optimizerState->getUnprocessedEvaluations().begin(); it < optimizerState->getUnprocessedEvaluations().end(); it++) {
        if (it->first < optimizerState->getBestScore()) {
          optimizerState->setBestScore(it->first);
          optimizerState->setBestVariable(it->second);
        }
      }
      optimizerState->clearUnprocessedEvaluations();
    }
    std::cout << "Best Score: " << optimizerState->getBestScore() << " (" << optimizerState->getBestVariable() << ")" << std::endl;
    return optimizerState->getBestScore();
 }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
};

typedef ReferenceCountedObjectPtr<UniformSampleAndPickBestOptimizer> UniformSampleAndPickBestOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

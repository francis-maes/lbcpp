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
# include <lbcpp/Execution/WorkUnit.h>


// TODO arnaud : modified to use new Function interface but not tested yet

namespace lbcpp
{

class UniformSampleAndPickBestOptimizer : public Optimizer
{
public:
  /*UniformSampleAndPickBestOptimizer(size_t numSamples = 0)
    : numSamples(numSamples) {}*/
  
  virtual void evaluationFinished(juce::int64 identifier, double score)
    {/*optimizerState->evaluationResults.push_back(std::pair<juce::int64, double>(identifier, score));*/}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {   
    std::vector<double> values;
    ContinuousDistributionPtr apriori = (optimizerState->distribution).dynamicCast<ContinuousDistribution>();
    apriori->sampleUniformly(numSamples, values);
    
    for (size_t i = 0; i < numSamples; ++i) 
      optimizerContext->evaluate(values[i]);
    
    while (optimizerState->evaluationResults.size() < numSamples) {
      Thread::sleep(5000);  // TODO arnaud
    }
    
    // TODO arnaud : check results.size() == requests.size()
    // TODO arnaud : sot results and requests by identifier
    
    double bestScore = DBL_MAX;
    double worstScore = -DBL_MAX;
    Variable res = Variable();
    for (size_t i = 0; i < numSamples; ++i)
    {
      if (optimizerState->evaluationResults[i].second < bestScore) {
        bestScore = optimizerState->evaluationResults[i].second;
        res = optimizerState->evaluationRequests[i].first;
      }
      if (optimizerState->evaluationResults[i].second > worstScore) {
        worstScore = optimizerState->evaluationResults[i].second;
      }
    }      
    
    std::cout << "Scores: " << worstScore << " ... " << bestScore << std::endl;
    return res;

    // OLD IMPLEMENTATION
    /*
    std::vector<double> values;
    std::vector<Variable> scores(numSamples);
    apriori.dynamicCast<ContinuousDistribution>()->sampleUniformly(numSamples, values);
    
    CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(T("Optimize ") + objective->toString(), numSamples));
    for (size_t i = 0; i < numSamples; ++i)
    {
      double parameterValue = values[i];
      workUnits->setWorkUnit(i, new FunctionWorkUnit(objective, std::vector<Variable>(1, parameterValue), String::empty, &scores[i], true));
    }
    workUnits->setPushChildrenIntoStackFlag(true);
    
    context.enterScope(workUnits);
    context.run(workUnits, false);
    double bestScore = -DBL_MAX;
    double worstScore = DBL_MAX;
    double res = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      //double learningRate = pow(10.0, (double)i / 10.0 - 3.0);
      //std::cout << "Score for LR = " << learningRate << ": " << scores[i] << std::endl;
      double score = scores[i].getDouble();
      if (score > bestScore)
        bestScore = score, res = values[i];
      if (score < worstScore)
        worstScore = score;
    }
    context.leaveScope(bestScore);
    
    std::cout << "Scores: " << worstScore << " ... " << bestScore << std::endl;
    return res;
    */
  }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

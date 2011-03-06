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

// works for "double" arguments
// uses a ContinuousDistribution apriori
// do not use the initial guess
class UniformSampleAndPickBestOptimizer : public Optimizer
{
public:
  UniformSampleAndPickBestOptimizer(size_t numSamples = 0)
    : numSamples(numSamples) {}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 2;} // do not use initial guess
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index) 
    {
      case 0:
        return (TypePtr) functionClass;
      case 1:
        return (TypePtr) uniformDistributionClass;
      default:
        jassert(false); // TODO arnaud
        return anyType;
    }
  }

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const FunctionPtr& objective = inputs[0].getObjectAndCast<Function>();
    ContinuousDistributionPtr apriori = inputs[1].getObjectAndCast<ContinuousDistribution>();
    jassert(apriori);
    
    std::vector<double> values;
    std::vector<Variable> scores(numSamples);
    apriori->sampleUniformly(numSamples, values);
    
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
  }

protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_
